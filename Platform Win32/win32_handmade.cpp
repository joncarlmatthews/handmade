// Windows API. Visual Studio freaks out if it's not the first include.
#include <windows.h>

#include <strsafe.h> // sprintf_s support
#include <dsound.h>  // Direct Sound for audio output.
#include <xinput.h>  // Xinput for receiving controller input.
#include <math.h>  // floor

//=======================================
// Game layer
//=======================================

#include "..\Game\handmade.h"

//=======================================
// End of game layer
//=======================================

// Platform layer specific function signatures
#include "win32_handmade.h"

// Whether or not the application is running
global_var bool8 running;
global_var bool8 paused;

// Create the Windows frame buffer
// @TOOD(JM) move this out of the global scope
global_var Win32FrameBuffer win32FrameBuffer = { 0 };

// XInput support
typedef DWORD WINAPI XInputGetStateDT(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState);
typedef DWORD WINAPI XInputSetStateDT(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION *pVibration);

global_var XInputGetStateDT *XInputGetState_ = XInputGetStateStub;
global_var XInputSetStateDT *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

// Direct sound support
typedef HRESULT WINAPI DirectSoundCreateDT(LPGUID lpGuid, LPDIRECTSOUND *ppDS, LPUNKNOWN  pUnkOuter);

// Query performance counter "frequency" value. Global so we can access it
// in all places in the plarform layer.
global_var int64 globalQPCFrequency;

//===========================================
// Game-required platform layer  functions
//===========================================

PLATFORM_ALLOCATE_MEMORY(platformAllocateMemory)
{
    return VirtualAlloc(NULL, bytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

PLATFORM_FREE_MEMORY(platformFreeMemory)
{
    VirtualFree(address, 0, MEM_RELEASE);
}

/**
 * Vibrate the controller. 0 = 0% motor usage, 65,535 = 100% motor usage.
 * The left motor is the low-frequency rumble motor. The right motor is the
 * high-frequency rumble motor.
 *
 */
PLATFORM_CONTROLLER_VIBRATE(platformControllerVibrate)
{
    XINPUT_VIBRATION pVibration = { 0 };

    pVibration.wLeftMotorSpeed = motor1Speed;
    pVibration.wLeftMotorSpeed = motor2Speed;

    XInputSetState(controllerIndex, &pVibration);
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_platformReadEntireFile)
{
    DEBUG_file file = { 0 };
    bool32 res;

    // Open the file for reading.
    HANDLE handle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == handle) {
        OutputDebugStringA("Cannot read file");
        return file;
    }

    // Get the size of the file in bytes.
    LARGE_INTEGER sizeStruct;
    res = GetFileSizeEx(handle, &sizeStruct);

    if (!res) {
        OutputDebugStringA("Cannot get file size");
        CloseHandle(handle);
        return file;
    }

    uint64 sizeInBytes = sizeStruct.QuadPart;

    // As GetFileSizeEx can read files larger than 4-bytes, but ReadFile can only
    // take a maximum of 4-bytes, lets make sure we're not reading files larger
    // than 4GB.
    uint32 sizeInBytes32 = win32TruncateToUint32Safe(sizeInBytes);

    // Allocate enough memory for the file.
    file.memory = VirtualAlloc(NULL, sizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (NULL == file.memory) {
        OutputDebugStringA("Cannot allocate memory for file");
        CloseHandle(handle);
        return file;
    }

    // Read the file into the memory.
    DWORD bytesRead;
    res = ReadFile(handle, file.memory, sizeInBytes32, &bytesRead, NULL);

    if ((!res) || (bytesRead != sizeInBytes32)) {
        OutputDebugStringA("Cannot read file into memory");
        //DEBUG_platformFreeFileMemory(&file);
        //CloseHandle(handle);
        //return file;
    }

    file.sizeinBytes = bytesRead;

    CloseHandle(handle);

    return file;
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_platformFreeFileMemory)
{
    VirtualFree(file->memory, 0, MEM_RELEASE);
    file->memory = 0;
    file->sizeinBytes = 0;
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_platformWriteEntireFile)
{
    bool32 res;

    // Open the file for writing.
    HANDLE handle = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == handle) {
        OutputDebugStringA("Cannot read file");
        return false;
    }

    // Read the file into the memory.
    DWORD bytesWritten;
    res = WriteFile(handle, memory, memorySizeInBytes, &bytesWritten, 0);

    if ((!res) || (bytesWritten != memorySizeInBytes)) {
        OutputDebugStringA("Could not write file to location");
        CloseHandle(handle);
        return false;
    }

    CloseHandle(handle);

    return true;
}

/*
 * The entry point for this graphical Windows-based application.
 * 
 * @param HINSTANCE A handle to the current instance of the application.
 * @param HINSTANCE A handle to the previous instance of the application.
 * @param LPSTR Command line arguments sent to the application.
 * @param int How the user has specified the window to be shown
 */
int APIENTRY wWinMain(_In_ HINSTANCE instance,
                        _In_opt_ HINSTANCE prevInstance,
                        _In_ LPWSTR commandLine,
                        _In_ int showCode)
{
    // Get the current performance-counter frequency, in counts per second.
    // @see https://docs.microsoft.com/en-us/windows/win32/api/profileapi/nf-profileapi-queryperformancefrequency
    // @see https://www.codeproject.com/Questions/480201/whatplusQueryPerformanceFrequencyplusfor-3f
    LARGE_INTEGER perfFrequencyCounterRes;
    QueryPerformanceFrequency(&perfFrequencyCounterRes);
    globalQPCFrequency = perfFrequencyCounterRes.QuadPart;

    // Load XInput DLL functions.
    loadXInputDLLFunctions();

    // Create a new window struct and set all of it's values to 0.
    WNDCLASS windowClass = {0};

    // Define the window's attributes. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;

    // Callback to handle any messages sent to the window (resize, close etc).
    windowClass.lpfnWndProc = win32MainWindowCallback;

    // Instance of the running application.
    windowClass.hInstance = instance;
    windowClass.lpszClassName = TEXT("handmadeHeroWindowClass");

    // Registers the window class for subsequent use in calls to 
    // the CreateWindowEx function.
    if (!RegisterClass(&windowClass)) {

        // TODO(JM) Log error.
        OutputDebugStringA("Error 1. windowClass not registered\n");
        return FALSE;
    }

    // Physically open the window using CreateWindowEx
    HWND window = CreateWindowEx(NULL,
                                    windowClass.lpszClassName,
                                    TEXT("Handmade Hero"),
                                    WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    NULL,
                                    NULL,
                                    instance,
                                    NULL);

    if (!window) {

        // TODO(JM) Log error.
        OutputDebugStringA("Error 2. window not created via CreateWindowEx\n");
        return FALSE;
    }

    // Usually you would call GetDC, do your work and then call ReleaseDC within
    // each loop, however because we specified CS_OWNDC, we can call it once
    // and use it forever.
    HDC deviceHandleForWindow = GetDC(window);

    GameCode gameCode = { 0 };

    loadGameDLLFunctions(&gameCode);

    /*
     * Game memory
     */
#if HANDMADE_LOCAL_BUILD
    LPVOID memoryStartAddress = (LPVOID)gameCode.gameTebibyteToBytes(4);
#else
    LPVOID memoryStartAddress = NULL;
#endif

    GameMemory memory = {0};
    memory.platformAllocateMemory = &platformAllocateMemory;
    memory.platformFreeMemory = &platformFreeMemory;
    memory.platformControllerVibrate = &platformControllerVibrate;
    memory.DEBUG_platformReadEntireFile = &DEBUG_platformReadEntireFile;
    memory.DEBUG_platformWriteEntireFile = &DEBUG_platformWriteEntireFile;
    memory.DEBUG_platformFreeFileMemory = &DEBUG_platformFreeFileMemory;

    memory.permanentStorageSizeInBytes = gameCode.gameMebibytesToBytes(64);
    memory.transientStorageSizeInBytes = gameCode.gameGibibytesToBytes(1);

    uint64 memoryTotalSize = (memory.permanentStorageSizeInBytes + memory.transientStorageSizeInBytes);

    memory.permanentStorage = VirtualAlloc(memoryStartAddress, memoryTotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    memory.transientStorage = (((uint8 *)memory.permanentStorage) + memory.permanentStorageSizeInBytes);

    if (memory.permanentStorage && memory.transientStorage) {

        /*
         * Framerate fixing.
         */
        Win32FixedFrameRate win32FixedFrameRate = {0};
        win32FixedFrameRate.monitorRefreshRate = 60;
        win32FixedFrameRate.gameTargetFPS = 30;
        win32FixedFrameRate.gameTargetMSPerFrame = (1000.0f / (float32)win32FixedFrameRate.gameTargetFPS);

        // Get the refresh rate of the monitor.
        DEVMODEA devMode = {0};
        bool32 getDisplaySettings = EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &devMode);

        if (getDisplaySettings) {
            win32FixedFrameRate.monitorRefreshRate = (uint8)devMode.dmDisplayFrequency;
        }

        // Set the system's minimum timer resolution to 1 millisecond
        // so that calls to the Windows Sleep() function are more
        // granular. E.g. the wake from the Sleep() will be checked
        // every 1ms, rather than the system default.
        win32FixedFrameRate.timeOutIntervalMS = 1;
        win32FixedFrameRate.timeOutIntervalSet = timeBeginPeriod(win32FixedFrameRate.timeOutIntervalMS);

        /*
         * Audio
         */

        // Create the Windows audio buffer
        Win32AudioBuffer win32AudioBuffer = {0};
        win32InitAudioBuffer(window, &win32AudioBuffer);

        // Kick off playing the Windows audio buffer
        win32AudioBufferTogglePlay(&win32AudioBuffer);

        // Create the game audio buffer.
        GameAudioBuffer gameAudioBuffer = {0};
        gameAudioBuffer.writeEntireBuffer = FALSE;
        gameAudioBuffer.minFramesWorthOfAudio = 4;

        /*
         * Graphics
         */

        // Create the Windows frame buffer
        win32InitFrameBuffer(&win32FrameBuffer, 1920, 1080);

        /*
         * Controllers
         */

        // How many controllers does the platform layer support?
        ControllerCounts controllerCounts = {0};
        controllerCounts.gameMaxControllers = MAX_CONTROLLERS;
        controllerCounts.platformMaxControllers = XUSER_MAX_COUNT;

        // An array to hold pointers to the old and new instances of the inputs.
        GameInput inputInstances[2] = {0};

        // We save a copy of what we've written to the inputs (in the old instance variable)
        // so we can compare last frame's values to this frames values.
        GameInput *inputNewInstance = &inputInstances[0];
        GameInput *inputOldInstance = &inputInstances[1];

        // Get the number of processor clock cycles
        uint64 runningProcessorClockCyclesCounter = __rdtsc();

        // Create a variable to hold the current time (we'll use this for profiling the elapsed game loop time)
        LARGE_INTEGER runningGameTime = win32GetTime();

        AncillaryPlatformLayerData ancillaryPlatformLayerData = {0};

        running = true;

        /**
         * MAIN GAME LOOP
         */
        while (running) {

            MSG message = {0};

            // Define keyboard controller support.
            // @TODO(JM) check that keyboard is connected.
            GameControllerInput *keyboard = &inputNewInstance->controllers[0];

            // Clear keyboard to zero.
            *keyboard = {0};
            keyboard->isConnected = true;

            controllerCounts.connectedControllers = 1;

            // Handle the Win32 message loop
            win32ProcessMessages(window, message, keyboard);

            if (paused) {

                win32AudioBufferToggleStop(&win32AudioBuffer);
                continue;

            } else {
                win32AudioBufferTogglePlay(&win32AudioBuffer);
            }

            // After processing our messages, we can now (in our "while running = true"
            // loop) do what we like! WM_SIZE and WM_PAINT get called as soon as the
            // application has been launched, so we'll have built the window and 
            // declared viewport height, width etc by this point.

            /*
             * Controller input stuff
             */

            // Iterate over each controller and get its state.
            DWORD dwResult;

            uint8 gamePadsAdded = 0;

            for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {

                XINPUT_STATE controllerInstance = { 0 };
                SecureZeroMemory(&controllerInstance, sizeof(XINPUT_STATE));

                // Simply get the state of the controller from XInput.
                dwResult = XInputGetState(controllerIndex, &controllerInstance);

                if (dwResult != ERROR_SUCCESS) {
                    // Controller is not connected/available.
                    continue;
                }

                // ...controller connected/available
                gamePadsAdded++;

                // Make sure we dont add more than our supported controller count.
                if (gamePadsAdded >= (controllerCounts.gameMaxControllers - 1)) {
                    continue;
                }

                controllerCounts.connectedControllers = (controllerCounts.connectedControllers + 1);

                // Fetch the gamepad
                XINPUT_GAMEPAD *gamepad = &controllerInstance.Gamepad;

                uint8 ourControllerIndex = ((uint8)controllerIndex + 1);

                GameControllerInput *newController = &inputNewInstance->controllers[ourControllerIndex];
                GameControllerInput *oldController = &inputOldInstance->controllers[ourControllerIndex];

                newController->isConnected = true;

                win32ProcessXInputControllerButton(&newController->dPadUp,
                    &oldController->dPadUp,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_UP);

                win32ProcessXInputControllerButton(&newController->dPadDown,
                    &oldController->dPadDown,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_DOWN);

                win32ProcessXInputControllerButton(&newController->dPadLeft,
                    &oldController->dPadLeft,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_LEFT);

                win32ProcessXInputControllerButton(&newController->dPadRight,
                    &oldController->dPadRight,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_RIGHT);

                win32ProcessXInputControllerButton(&newController->up,
                    &oldController->up,
                    gamepad,
                    XINPUT_GAMEPAD_Y);

                win32ProcessXInputControllerButton(&newController->down,
                    &oldController->down,
                    gamepad,
                    XINPUT_GAMEPAD_A);

                win32ProcessXInputControllerButton(&newController->right,
                    &oldController->right,
                    gamepad,
                    XINPUT_GAMEPAD_B);

                win32ProcessXInputControllerButton(&newController->left,
                    &oldController->left,
                    gamepad,
                    XINPUT_GAMEPAD_X);

                // Left controller thumbstick support
                newController->isAnalog = true;

                // Normalise the axis values so the values are between -1.0 and 1.0
                // @see maximum signed short values
                float32 leftThumbstickX = 0.0f;
                if (gamepad->sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                    leftThumbstickX = ((float32)gamepad->sThumbLX / 32512.0f);
                }
                else if (gamepad->sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                    leftThumbstickX = ((float32)gamepad->sThumbLX / 32768.0f);
                }

                float32 leftThumbstickY = 0.0f;
                if (gamepad->sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                    leftThumbstickY = ((float32)(gamepad->sThumbLY - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / (32512.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE));
                }
                else if (gamepad->sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) {
                    leftThumbstickY = ((float32)(gamepad->sThumbLY + XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE) / (32768.0f - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE));
                }

                newController->leftThumbstick.position.x = leftThumbstickX;
                newController->leftThumbstick.position.y = leftThumbstickY;

                // Swap the controller intances
                GameControllerInput *temp = newController;
                newController = oldController;
                oldController = temp;

            } // controller loop

            // Size, in bytes, of the portion of the buffer to write.
            DWORD lockSizeInBytes = 0;

            // Offset, in bytes, from the start of the buffer to the point where the lock begins.
            // We Mod the result by the total number of bytes so that the value wraps.
            // Result will look like this: 0, 4, 8, 12, 16, 24 etc...
            DWORD lockOffsetInBytes = 0;

            // Start playing sound. (Write a dummy wave sound)
            // Each single "sample" is a 16-bit value. 8 bits for the left channel, and 8 bits for the right channel.
            // They both go together.
            // Each individual sample gets output at a time, thus outputting to both the left and right channels
            // at the same time. The sound buffer (win32AudioBuffer.buffer) contains all of these 16-bit audio samples.

            // The IDirectSoundBuffer8::GetCurrentPosition method retrieves 
            // the position of the play and write cursors in the sound buffer.
            if (win32AudioBuffer.bufferSuccessfulyCreated) {

                DWORD playCursorOffsetInBytes   = NULL; // Offset, in bytes, of the play cursor
                DWORD writeCursorOffsetInBytes  = NULL; // Offset, in bytes, of the write cursor
                DWORD writePlayDifference       = 0; // Difference in bytes between the play and the write cursors.

                struct AuidioLatency {
                    uint32 samplesLatent;
                    float32 samplesLatentAsPercentageOfBuffer;
                    float32 latencyInMS;
                } audioLatency = {0};

                HRESULT res = win32AudioBuffer.buffer->GetCurrentPosition(&playCursorOffsetInBytes, &writeCursorOffsetInBytes);

                if ((DS_OK == res) && (win32AudioBuffer.bufferSizeInBytes > 0)) {

                    // IDirectSoundBuffer8::Lock Readies all or part of the buffer for a data 
                    // write and returns pointers to which data can be written

                    lockOffsetInBytes = (writeCursorOffsetInBytes % win32AudioBuffer.bufferSizeInBytes);

                    // Is the current lock offset ahead of the current play cursor? If yes, we'll get back 
                    // two chucks of data from IDirectSoundBuffer8::Lock, otherwise we'll only get back
                    // one chuck of data.
                    if (writeCursorOffsetInBytes > playCursorOffsetInBytes) {
                        lockSizeInBytes = (win32AudioBuffer.bufferSizeInBytes - (writeCursorOffsetInBytes - playCursorOffsetInBytes));
                        writePlayDifference = (writeCursorOffsetInBytes - playCursorOffsetInBytes);
                    } else if (writeCursorOffsetInBytes < playCursorOffsetInBytes) {
                        lockSizeInBytes = ((win32AudioBuffer.bufferSizeInBytes - (win32AudioBuffer.bufferSizeInBytes - playCursorOffsetInBytes)) - writeCursorOffsetInBytes);
                        writePlayDifference = ((win32AudioBuffer.bufferSizeInBytes - playCursorOffsetInBytes) + writeCursorOffsetInBytes);
                    }

                    audioLatency.samplesLatent = (writePlayDifference / win32AudioBuffer.bytesPerSample);
                    audioLatency.samplesLatentAsPercentageOfBuffer = ((((float32)audioLatency.samplesLatent * 100.0f) / ((float32)win32AudioBuffer.samplesPerSecond * (float32)win32AudioBuffer.secondsWorthOfAudio)) / 100.0f);
                    audioLatency.latencyInMS = ((1000.f * win32AudioBuffer.secondsWorthOfAudio) * audioLatency.samplesLatentAsPercentageOfBuffer);

                   
                    // If we're opting to *not* write the entire audio buffer, calculate how much to write here...
                    if ((!gameAudioBuffer.writeEntireBuffer) && (gameAudioBuffer.minFramesWorthOfAudio >= 1)) {

                        // How many samples do we need to write? (number of samples in MS)
                        // Write at least the audio latency (in ms)
                        float32 msToWrite = audioLatency.latencyInMS;

                        // If the game's target frame rate (in ms) is larger than the audio latency (in ms)
                        // then set that as our minimum latency.
                        if (win32FixedFrameRate.gameTargetMSPerFrame > audioLatency.latencyInMS) {
                            msToWrite = win32FixedFrameRate.gameTargetMSPerFrame;
                        }
                        // Now add up to the margin of safety
                        float32 marginTotalInMS = (win32FixedFrameRate.gameTargetMSPerFrame * (float32)gameAudioBuffer.minFramesWorthOfAudio);
                        if (marginTotalInMS > msToWrite) {
                            msToWrite = (msToWrite + (marginTotalInMS - msToWrite));
                        }

                        float32 samplesToWrite = ((float32)win32AudioBuffer.samplesPerSecond * (((msToWrite * 100.0f) / 1000.0f) / 100));
                        uint32 noOfBytesToWrite = (uint32)(samplesToWrite * win32AudioBuffer.bytesPerSample);

                        if (noOfBytesToWrite > win32AudioBuffer.bufferSizeInBytes) {
                            // We've somehow ended up with a calculation that's bigger than
                            // the audio buffer available. Don't overwrite the lockSizeInBytes
                            assert(!"noOfBytesToWrite calculation is > win32AudioBuffer.bufferSizeInBytes");
                        } else {
                            // Overwrite the lockSizeInBytes to match our smaller lock size.
                            lockSizeInBytes = noOfBytesToWrite;
                        }
                    }
                    

#if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_AUDIO)

                    // @TODO(JM) Make audio latency match a single frame
                    if (win32AudioBuffer.bufferSizeInBytes > 0) {
                        char buff[200] = { 0 };
                        sprintf_s(buff,
                            sizeof(buff),
                            "Audio latency: %.2fms (%.2f frames)\n",
                            audioLatency.latencyInMS,
                            (audioLatency.latencyInMS / win32FixedFrameRate.gameTargetMSPerFrame));
                        OutputDebugStringA(buff);
                    }

#endif // HANDMADE_DEBUG_AUDIO

                    ancillaryPlatformLayerData.audioBuffer.playCursorPosition   = playCursorOffsetInBytes;
                    ancillaryPlatformLayerData.audioBuffer.writeCursorPosition  = writeCursorOffsetInBytes;
                    ancillaryPlatformLayerData.audioBuffer.lockSizeInBytes      = lockSizeInBytes;
                    ancillaryPlatformLayerData.audioBuffer.lockOffsetInBytes    = lockOffsetInBytes;
                } else {
                    OutputDebugStringA("Could not get the position of the play and write cursors in the secondary sound buffer");
                }

            } // Audio buffer created.

            // Create the game's audio buffer
            gameCode.gameInitAudioBuffer(&memory,
                                            &gameAudioBuffer,
                                            lockSizeInBytes,
                                            win32AudioBuffer.bytesPerSample,
                                            win32AudioBuffer.bufferSizeInBytes);

            // Create the game's frame buffer
            GameFrameBuffer gameFrameBuffer = {0};
            gameCode.gameInitFrameBuffer(&gameFrameBuffer,
                                            win32FrameBuffer.height,
                                            win32FrameBuffer.width,
                                            win32FrameBuffer.bytesPerPixel,
                                            win32FrameBuffer.byteWidthPerRow,
                                            win32FrameBuffer.memory);

            // Main game code.
            gameCode.gameUpdate(&memory, &gameFrameBuffer, &gameAudioBuffer, inputInstances, &controllerCounts, ancillaryPlatformLayerData);

            // Output the audio buffer in Windows.
            win32WriteAudioBuffer(&win32AudioBuffer, lockOffsetInBytes, lockSizeInBytes, &gameAudioBuffer);

            // How long did this game loop (frame) take?

            // Performance-counter frequency for MS/frame & FPS
            LARGE_INTEGER gameLoopTime = win32GetTime();

            // Calculate milliseconds (and seconds) taken for this frame
            float32 millisecondsElapsedForFrame     = win32GetElapsedTimeMS(runningGameTime, gameLoopTime, globalQPCFrequency);
            float32 secondsElapsedForFrame          = win32GetElapsedTimeS(runningGameTime, gameLoopTime, globalQPCFrequency);

            // Cap framerate to target FPS if we're running ahead.
            if ((millisecondsElapsedForFrame < win32FixedFrameRate.gameTargetMSPerFrame) && ((floor(win32FixedFrameRate.gameTargetMSPerFrame) - floor(millisecondsElapsedForFrame)) >= 1)) {
                if (TIMERR_NOERROR == win32FixedFrameRate.timeOutIntervalSet) {
                    DWORD sleepMS = ((DWORD)floor(win32FixedFrameRate.gameTargetMSPerFrame) - (DWORD)floor(millisecondsElapsedForFrame));
                    if (sleepMS > 0) {
                        Sleep(sleepMS);
                    }
                    millisecondsElapsedForFrame = win32GetElapsedTimeMS(runningGameTime, win32GetTime(), globalQPCFrequency);
                }else{
                    while (millisecondsElapsedForFrame < win32FixedFrameRate.gameTargetMSPerFrame) {
                        millisecondsElapsedForFrame = win32GetElapsedTimeMS(runningGameTime, win32GetTime(), globalQPCFrequency);
                    }
                }
            } else {
                // @TODO(JM) Missed target framerate. Log.
            }

            // Display the frame buffer in Windows. AKA "flip the frame" or "page flip".
            win32ClientDimensions clientDimensions = win32GetClientDimensions(window);
            win32DisplayFrameBuffer(deviceHandleForWindow, win32FrameBuffer, clientDimensions.width, clientDimensions.height);

            // Reset the running clock counter.
            runningGameTime.QuadPart = gameLoopTime.QuadPart;

#if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)

            // Calculate how many processor clock cycles elapsed for this frame.
            // @NOTE(JM) __rdtsc is only for dev and not for relying on for shipped code that will run on end user's machine.
            uint64 processorClockCyclesAfterFrame = __rdtsc();
            int64 processorClockCyclesElapsedForFrame = (processorClockCyclesAfterFrame - runningProcessorClockCyclesCounter);
            float32 clockCycles_mega = ((float32)processorClockCyclesElapsedForFrame / 1000000.0f); // processorClockCyclesElapsedForFrame is in the millions, dividing by 1m to give us a "mega" (e.g. megahertz) value.

            // Calculate the FPS given the speed of this current frame.
            float32 fps = (1000.0f / (float32)millisecondsElapsedForFrame);

            // Calculate the processor running speed in GHz
            float32 processorSpeed = ((uint64)(fps * clockCycles_mega) / 100.0f);

            // Reset the running clock cycles.
            runningProcessorClockCyclesCounter = processorClockCyclesAfterFrame;

            // Console log the speed:
            char output[100] = {0};
            sprintf_s(output, sizeof(output),
                        "ms/frame: %.1f s/frame %.5f, FSP: %.1f. Cycles: %.1fm (%.2f GHz).\n",
                        millisecondsElapsedForFrame, secondsElapsedForFrame, fps, clockCycles_mega, processorSpeed);
            OutputDebugStringA(output);
#endif

        } // game loop

        if (TIMERR_NOERROR == win32FixedFrameRate.timeOutIntervalSet) {
            timeEndPeriod(win32FixedFrameRate.timeOutIntervalMS);
        }


    }else{
        OutputDebugStringA("Error allocating game memory. Unable to run game\n");
    }

    // Close the application.
    return(0);
}

/*
 * Callback method for WNDCLASS struct. Processes messages sent to the window.
 * E.g. resize, close etc.
 *
 * @see https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms633573(v=vs.85)
 *
 * If you observe the console log, you'll notice that:
 * 
 * When the app first loads it sends messages to the queue in this order:
 *
 * 1) WM_ACTIVATEAPP
 * 2) WM_SIZE
 * 3) WM_PAINT
 *
 * When the app is focused on, or clicked away from it sends: WM_ACTIVATEAPP
 *
 * When the app's window is resized it sends WM_SIZE immediately followed by
 * WM_PAINT
 *
 * When the app's window position is moved (not resized) it sends WM_PAINT
 *
 * When the app is closed it sends WM_CLOSE
 */
internal_func LRESULT CALLBACK win32MainWindowCallback(HWND window,
                                                        UINT message,
                                                        WPARAM wParam,
                                                        LPARAM lParam)
{
    LRESULT result = 0;

    switch (message) {

        // This message is *only* sent when the application is first loaded OR
        // when the window is resized.
        case WM_SIZE: {
        } break;

        case WM_DESTROY: {
            // @TODO(JM) Handle as an error. Recreate window?
            running = false;
        } break;

        // Called when the user requests to close the window.
        case WM_CLOSE: {
            // @TODO(JM) Display "are you sure" message to user?
            running = false;
        } break;

        case WM_QUIT: {
            running = false;
        } break;

        // Called when the user makes the window active (e.g. by tabbing to it).
        case WM_ACTIVATEAPP: {
        } break;

        // Request to paint a portion of an application's window.
        case WM_PAINT: {

            // Prepare the window for painting.

            // The PAINTSTRUCT var contains the area that needs to be repainted, 
            // however we dont need this as we simply repaint the entire window
            // each time, not just the area that Windows tells us needs to be
            // repainted. BeginPaint has to be called before StretchDIBits.
            PAINTSTRUCT paint; 
            HDC deviceHandleForWindow = BeginPaint(window, &paint);

            win32ClientDimensions clientDimensions = win32GetClientDimensions(window);

            win32DisplayFrameBuffer(deviceHandleForWindow, win32FrameBuffer, clientDimensions.width, clientDimensions.height);

            // End the paint request and releases the device context.
            EndPaint(window, &paint);
        } break;

        // Keyboard messages. Now handled within our Win32 game loop.
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {
            // This shouldnt happen. assert during development to make sure.
            assert(!"Windows has called this directtly and not throug PeekMessage");
        } break;

        // The standard request from GetMessage().
        default: {

            //OutputDebugStringA("default\n");

            // The default window procedure to provide default processing for 
            // any window messages not explicitly handled. It's required by the
            // Win32 API that every message is handled. And the docs specify
            // that DefWindowProc is called for all non handled messages.
            // @see https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-defwindowproca
            result = DefWindowProc(window, message, wParam, lParam);

        } break;
    }

    return result;
}

/*
 * This function will create a new DIB, or resize it if its already been created
 * during a previous call to this function.
 *
 * A DIB (Device Independent Bitmap) is what Windows calls things that we can 
 * write into, which it can then display to the screen using it's internal 
 * Graphics Device Interface (GDI).
 *
 * @param win32FrameBuffer  *buffer     A pointer to the Win32 off screen buffer
 * @param int                   width       The width of the window's viewport
 * @param int                   height      The height of the window's viewport
 * 
 */
internal_func void win32InitFrameBuffer(Win32FrameBuffer *buffer, uint32 width, int32 height)
{

    // buffer->foo is a dereferencing shorthand for (*buffer).foo

    // Does the bitmapMemory already exist from a previous WM_SIZE call?
    if (buffer->memory != NULL) {

        // Yes, then free the memorty allocated.
        // We do this because we have to redraw it as this method
        // (win32InitFrameBuffer) is called on a window resize.
        VirtualFree(buffer->memory, NULL, MEM_RELEASE);
    }

    buffer->bytesPerPixel   = 4;
    buffer->width           = width;
    buffer->height          = height;

    buffer->info.bmiHeader.biSize           = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth          = width;
    buffer->info.bmiHeader.biHeight         = -height; // If negative, it's drawn top down. If positive, it's drawn bottom up.
    buffer->info.bmiHeader.biPlanes         = 1;
    buffer->info.bmiHeader.biBitCount       = (buffer->bytesPerPixel * 8); // 32-bits per pixel
    buffer->info.bmiHeader.biCompression    = BI_RGB;

    // How many bytes do we need for our bitmap?
    // viewport width * viewport height = viewport area
    // then viewport area * how many bytes we need per pixel.
    uint32 bitmapMemorySizeInBytes = ((buffer->width * buffer->height) * buffer->bytesPerPixel);

    // Now allocate the memory using VirtualAlloc to the size of the previously
    // calculated bitmapMemorySizeInBytes
    buffer->memory = VirtualAlloc(NULL, bitmapMemorySizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    // Calculate the width in bytes per row.
    buffer->byteWidthPerRow = (buffer->width * buffer->bytesPerPixel);
}

/*
 * Function for handling WM_PAINT message.
 *
 * Copies a Win32FrameBuffer's memory to the actual window - which will then
 * display its contents to the screen.
 *
 *
 * @param deviceHandleForWindow     The window handle
 * @param Win32FrameBuffer      The buffer
 * @param width                        The window's viewport width
 * @param height                    The window's viewport height
 */
internal_func void win32DisplayFrameBuffer(HDC deviceHandleForWindow, 
                                            Win32FrameBuffer buffer,
                                            uint32 width,
                                            uint32 height)
{
    // @TODO(JM) Do some maths to stop the buffer's height and width being
    // skewed when the window's width and height doesn't match the aspect
    // ratio that we want. (e.g. when someone manually resizes the window)
    // ...

    // StretchDIBits function copies the data of a rectangle of pixels to 
    // the specified destination. The first parameter is the handle for
    // the destination's window that we want to write the data to.
    // Pixels are drawn to screen from the top left to the top right, then drops a row,
    // draws from left to right and so on. Finally finishing on the bottom right pixel
    StretchDIBits(deviceHandleForWindow,
                    0,
                    0,
                    width,
                    height,
                    0,
                    0,
                    buffer.width,
                    buffer.height,
                    buffer.memory,
                    &buffer.info,
                    DIB_RGB_COLORS,
                    SRCCOPY);
}

internal_func void win32InitAudioBuffer(HWND window, Win32AudioBuffer *win32AudioBuffer)
{
    win32AudioBuffer->bufferSuccessfulyCreated = FALSE;

    // Load the library
    HMODULE libHandle = LoadLibrary(TEXT("dsound.dll"));

    if (!libHandle) {
        OutputDebugStringA("Could not load DirectSound DLL (dsound.dll)");
        return;
    }

    // Result variable for the various function call return checks.
    HRESULT res;

    DirectSoundCreateDT *DirectSoundCreateAddr = (DirectSoundCreateDT*)GetProcAddress(libHandle, "DirectSoundCreate");

    if (!DirectSoundCreateAddr) {
        // Function not found within library.
        OutputDebugStringA("DirectSoundCreate not in dsound.dll. Invalid/malformed DLL.");
        return;
    }

    DirectSoundCreateDT *DirectSoundCreate = DirectSoundCreateAddr;

    LPDIRECTSOUND directSound;

    res = DirectSoundCreate(NULL, &directSound, NULL);

    if (FAILED(res)){
        OutputDebugStringA("Could not create direct sound object");
        return;
    }

    res = directSound->SetCooperativeLevel(window, DSSCL_PRIORITY);

    if (FAILED(res)){
        OutputDebugStringA("Could not set cooperative level on direct sound object");
        return;
    }

    // Create a "primary buffer". We do this purely to set the format
    // of the sound card (via DIRECTSOUNDBUFFER::SetFormat). We do this
    // so that when we create the secondary buffer that we actually write 
    // to, the sound card is already in the correct format. This is the only
    // purpose of the primary buffer.
    DSBUFFERDESC primarySoundBufferDesc;
    ZeroMemory(&primarySoundBufferDesc, sizeof(DSBUFFERDESC));

    primarySoundBufferDesc.dwSize            = sizeof(primarySoundBufferDesc);
    primarySoundBufferDesc.dwFlags           = DSBCAPS_PRIMARYBUFFER;
    primarySoundBufferDesc.dwBufferBytes     = 0;
    primarySoundBufferDesc.lpwfxFormat       = NULL;
    primarySoundBufferDesc.guid3DAlgorithm   = GUID_NULL;

    LPDIRECTSOUNDBUFFER primarySoundBuffer;

    res = directSound->CreateSoundBuffer(&primarySoundBufferDesc, &primarySoundBuffer, NULL);

    if (FAILED(res)){
        OutputDebugStringA("Could not create primary buffer");
        return;
    }

    // Init our Win32 audio buffer...
    win32AudioBuffer->noOfChannels = 2;
    win32AudioBuffer->bitsPerChannel = 16;
    win32AudioBuffer->samplesPerSecond = 48000;
    win32AudioBuffer->bytesPerSample = ((win32AudioBuffer->bitsPerChannel * win32AudioBuffer->noOfChannels) / 8);
    win32AudioBuffer->secondsWorthOfAudio = 1;
    win32AudioBuffer->bufferSizeInBytes = (uint64)((win32AudioBuffer->bytesPerSample * win32AudioBuffer->samplesPerSecond) * win32AudioBuffer->secondsWorthOfAudio);

    // Set the format
    WAVEFORMATEX waveFormat = {0};

    waveFormat.wFormatTag          = WAVE_FORMAT_PCM;
    waveFormat.nChannels           = win32AudioBuffer->noOfChannels;
    waveFormat.nSamplesPerSec      = win32AudioBuffer->samplesPerSecond;
    waveFormat.nAvgBytesPerSec     = (win32AudioBuffer->samplesPerSecond * win32AudioBuffer->bytesPerSample);
    waveFormat.nBlockAlign         = win32AudioBuffer->bytesPerSample;
    waveFormat.wBitsPerSample      = win32AudioBuffer->bitsPerChannel;
    waveFormat.cbSize              = 0;

    res = primarySoundBuffer->SetFormat(&waveFormat);

    if (FAILED(res)){
        OutputDebugStringA("Could not set sound format on primary buffer");
        return;
    }

    // Create the secondary buffer. This is the buffer we'll use to actually
    // write bytes to.
    DSBUFFERDESC secondarySoundBufferDesc;
    ZeroMemory(&secondarySoundBufferDesc, sizeof(DSBUFFERDESC));

    secondarySoundBufferDesc.dwSize              = sizeof(secondarySoundBufferDesc);
    secondarySoundBufferDesc.dwFlags             = 0;
    secondarySoundBufferDesc.dwBufferBytes       = win32AudioBuffer->bufferSizeInBytes;
    secondarySoundBufferDesc.lpwfxFormat         = &waveFormat;
    secondarySoundBufferDesc.guid3DAlgorithm     = GUID_NULL;

    res = directSound->CreateSoundBuffer(&secondarySoundBufferDesc, &win32AudioBuffer->buffer, NULL);

    if (FAILED(res)){
        OutputDebugStringA("Could not create secondary buffer");
        return;
    }

    win32AudioBuffer->bufferSuccessfulyCreated = TRUE;

    OutputDebugStringA("Primary & secondary successfully buffer created\n");
}

internal_func void win32AudioBufferTogglePlay(Win32AudioBuffer *win32AudioBuffer)
{
    DWORD pdwStatus;
    if (SUCCEEDED(win32AudioBuffer->buffer->GetStatus(&pdwStatus))) {
        if ((pdwStatus & DSBSTATUS_PLAYING) != DSBSTATUS_PLAYING) { // If not playing
            win32AudioBuffer->buffer->Play(0, 0, DSBPLAY_LOOPING);
        }
    }
}

internal_func void win32AudioBufferToggleStop(Win32AudioBuffer *win32AudioBuffer)
{
    DWORD pdwStatus;
    if (SUCCEEDED(win32AudioBuffer->buffer->GetStatus(&pdwStatus))) {
        if ((pdwStatus & DSBSTATUS_PLAYING) == DSBSTATUS_PLAYING) { // If playing
            win32AudioBuffer->buffer->Stop();
        }
    }
}

internal_func void win32WriteAudioBuffer(Win32AudioBuffer *win32AudioBuffer,
                                            DWORD lockOffsetInBytes,
                                            DWORD lockSizeInBytes,
                                            GameAudioBuffer *gameAudioBuffer)
{
    if (!win32AudioBuffer->bufferSuccessfulyCreated) {
        return;
    }

    // Ensure we have at least once sample to write
    if (lockSizeInBytes < win32AudioBuffer->bytesPerSample) {
        return;
    }

    HRESULT res;

    void *chunkOnePtr; // Receives a pointer to the first locked part of the buffer.
    DWORD chunkOneBytes; // Receives the number of bytes in the block at chunkOnePtr

    void *chunkTwoPtr; // Receives a pointer to the second locked part of the buffer.
    DWORD chunkTwoBytes; // Receives the number of bytes in the block at chunkTwoPtr

    res = win32AudioBuffer->buffer->Lock(lockOffsetInBytes,
                                            lockSizeInBytes,
                                            &chunkOnePtr,
                                            &chunkOneBytes,
                                            &chunkTwoPtr,
                                            &chunkTwoBytes,
                                            0);


    if (SUCCEEDED(res)) {

        // Calculate the total number of 4-byte audio sample groups (2-bytes/16 bits for the left channel, 2-bytes/16 bits for the right channel) 
        // that we have within the first block of memory IDirectSoundBuffer8::Lock has told us we can write to.
        uint64 audioSampleGroupsChunkOne = (chunkOneBytes / win32AudioBuffer->bytesPerSample);

        // Grab the first 4-bytes of the first audio sample grouping from the first block of memory 
        uint32 *audioSample = (uint32 *)chunkOnePtr;

        // Create a pointer to the game audio buffer with the same 4-byte single audio sample grouping range.
        uint32 *buffer = (uint32 *)gameAudioBuffer->memory;

        // Advance the game audio buffer pointer to match the same as the lock offset.
        //buffer = (buffer + (lockOffsetInBytes / win32AudioBuffer->bytesPerSample));

        // Iterate over each individual audio sample grouping (2-bytes for the left channel, 2-bytes for the right channel)
        // and write the same data for both...
        for (size_t i = 0; i < audioSampleGroupsChunkOne; i++) {

            // Left + right channel (4-bytes)
            *audioSample = *buffer;

            // Move cursor to the start of the next audio sample grouping.
            audioSample++;
            buffer++;
        }

        // Calculate how many samples we need to write to in our second block of memory.
        uint64 audioSampleGroupsChunkTwo = (chunkTwoBytes / win32AudioBuffer->bytesPerSample);

        // Grab the first 4-bytes of the first audio sample grouping from the second block of memory 
        uint32 *audioTwoSample = (uint32*)chunkTwoPtr;

        // Set the audio buffer pointer back to the start of the memory block as the
        // second block of memory always starts from the beginning
        buffer = (uint32 *)gameAudioBuffer->memory;

        for (size_t i = 0; i < audioSampleGroupsChunkTwo; i++) {

            // Left + right channel (4-bytes)
            *audioTwoSample = *buffer;

            // Move cursor to the start of the next audio sample grouping.
            audioTwoSample++;
            buffer++;
        }

        res = win32AudioBuffer->buffer->Unlock(chunkOnePtr, chunkOneBytes, chunkTwoPtr, chunkTwoBytes);

        if (FAILED(res)) {
            OutputDebugStringA("Could not unlock sound buffer");
        }

    } else {
        OutputDebugStringA("Could not lock secondary sound buffer");
    }

    return;
}

internal_func win32ClientDimensions win32GetClientDimensions(HWND window)
{
    RECT clientRect;
    GetClientRect(window, &clientRect);

    win32ClientDimensions dim = {0};

    dim.width = clientRect.right;
    dim.height = clientRect.bottom;

    return dim;
}

internal_func void win32ProcessMessages(HWND window, MSG message, GameControllerInput *keyboard)
{
    // Win32 Message loop. Retrieves all messages (from the calling thread's message queue)
    // that are sent to the window. E.g. clicks and key inputs.
    while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {

        switch (message.message) {

            // If the message received was a quit message, then toggle our
            // running flag to false to break out of this loop on the next
            // iteration.
            case WM_QUIT: {
                running = false;
            } break;

            // All keyboard input messages
            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP: {

                // Which key was pressed?
                WPARAM vkCode = message.wParam;

                /*
                 * lParam bitmask. Written from right to left
                 *
                 *  First two bytes              Second two bytes
                 * |-------------------------|  |---------------|
                 * 31 30 29 28-25 24 23-16      15-0
                 * 0  0  0  0000  1  01001011   0000000000000001
                */

                // lParam is 4-bytes wide.
                uint32 *lParamBitmask = (uint32 *)&message.lParam;

                // Fetch the second two bytes (bits 0-15)
                // @BUG(JM) this count is wrong
                uint16 *repeatCount = (uint16 *)&message.lParam;
                repeatCount = (repeatCount + 1);

                bool32 isDown = ((*lParamBitmask & (1 << 31)) == 0); // 1 if the key is down
                bool32 wasDown = ((*lParamBitmask & (1 << 30)) != 0); // 1 if the key was down

#ifdef HANDMADE_DEBUG
                if (vkCode == 'W') {
                    char buff[100] = {0};
                    sprintf_s(buff, sizeof(buff), "is down? %i\n", isDown);
                    OutputDebugStringA(buff);

                    memset(buff, 0, sizeof(buff));
                    sprintf_s(buff, sizeof(buff), "was down? %i\n", wasDown);
                    OutputDebugStringA(buff);

                    memset(buff, 0, sizeof(buff));
                    sprintf_s(buff, sizeof(buff), "repeat count %i\n", *repeatCount);
                    OutputDebugStringA(buff);
                }
#endif // HANDMADE_DEBUG

                switch (vkCode) {
                    case 'P': {
                        if (isDown) {
                            if (paused) {
                                paused = false;
                            }else {
                                paused = true;
                            }
                        }
                    } break;

                    case 'W': {
                        GameControllerBtnState state = {0};
                        state.halfTransitionCount++;
                        state.endedDown = isDown;
                        keyboard->dPadUp = state;
                    } break;

                    case 'A': {
                        GameControllerBtnState state = {0};
                        state.halfTransitionCount++;
                        state.endedDown = isDown;
                        keyboard->dPadLeft = state;
                    } break;

                    case 'S': {
                        GameControllerBtnState state = {0};
                        state.halfTransitionCount++;
                        state.endedDown = isDown;
                        keyboard->dPadDown = state;
                    } break;

                    case 'D': {
                        GameControllerBtnState state = {0};
                        state.halfTransitionCount++;
                        state.endedDown = isDown;
                        keyboard->dPadRight = state;
                    } break;

                    case 'Q': {
                        GameControllerBtnState state = {0};
                        state.halfTransitionCount++;
                        state.endedDown = isDown;
                        keyboard->shoulderL1 = state;
                    } break;

                    case 'E': {
                        GameControllerBtnState state = {0};
                        state.halfTransitionCount++;
                        state.endedDown = isDown;
                        keyboard->shoulderR1 = state;
                    } break;
                }

            } break;

            // The standard request from GetMessage().
            default: {

                // Dispatch the message to the application's window procedure win32MainWindowCallback()
                TranslateMessage(&message); // Get the message ready for despatch.
                DispatchMessage(&message); // Actually do the despatch

            } break;

        } // message switch

    } // PeekMessage loop
}

internal_func LARGE_INTEGER win32GetTime()
{
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return counter;
}

internal_func float32 win32GetElapsedTimeMS(const LARGE_INTEGER startCounter, const LARGE_INTEGER endCounter, int64 countersPerSecond)
{
    return ( ((float32)(endCounter.QuadPart - startCounter.QuadPart) * 1000.0f) / (float32)countersPerSecond);
}

internal_func float32 win32GetElapsedTimeS(const LARGE_INTEGER startCounter, const LARGE_INTEGER endCounter, int64 countersPerSecond)
{
    return ((float32)(endCounter.QuadPart - startCounter.QuadPart) / (float32)countersPerSecond);
}

internal_func void win32ProcessXInputControllerButton(GameControllerBtnState *newState,
                                                        GameControllerBtnState *oldState,
                                                        XINPUT_GAMEPAD *gamepad,
                                                        uint16 gamepadButtonBit)
{
    if ((*newState).endedDown != (*oldState).endedDown) {
        (*newState).halfTransitionCount = ((*newState).halfTransitionCount + 1);
    }
    (*newState).endedDown = ((*gamepad).wButtons & gamepadButtonBit);
}

internal_func uint32 win32TruncateToUint32Safe(uint64 value)
{
    assert((value <= 0xffffffff));
    return (uint32)value;
}

//=======================================
// Library loading
//=======================================
internal_func DWORD WINAPI XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE *pState)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

internal_func DWORD WINAPI XInputSetStateStub(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
{
    return ERROR_DEVICE_NOT_CONNECTED;
}

internal_func void loadXInputDLLFunctions(void)
{
    HMODULE libHandle = LoadLibrary(TEXT("XInput1_4.dll"));

    // No XInput 1.4? Try and load the older 9.1.0.
    if (!libHandle) {
        libHandle = LoadLibrary(TEXT("XInput9_1_0.dll"));
    }

    // No XInput 9.1.0? Try and load the even older 1.3.
    if (!libHandle) {
        libHandle = LoadLibrary(TEXT("XInput1_3.dll"));
    }

    if (libHandle) {

        XInputGetStateDT *XInputGetStateAddr = (XInputGetStateDT *)GetProcAddress(libHandle, "XInputGetState");
        XInputSetStateDT *XInputSetStateAddr = (XInputSetStateDT *)GetProcAddress(libHandle, "XInputSetState");

        if (XInputGetStateAddr) {
            XInputGetState = XInputGetStateAddr;
        }
        if (XInputSetStateAddr) {
            XInputSetState = XInputSetStateAddr;
        }
    }
}

internal_func void loadGameDLLFunctions(GameCode *gameCode)
{
    HMODULE libHandle = LoadLibrary(TEXT("..\\build\\Game\\x64\\Debug\\Game.dll"));

    if (libHandle) {

        GameUpdate *gameUpdateAddr                      = (GameUpdate *)GetProcAddress(libHandle, "gameUpdate");
        GameInitFrameBuffer *gameInitFrameBufferAddr    = (GameInitFrameBuffer *)GetProcAddress(libHandle, "gameInitFrameBuffer");
        GameInitAudioBuffer *gameInitAudioBufferAddr    = (GameInitAudioBuffer *)GetProcAddress(libHandle, "gameInitAudioBuffer");
        GameKibibytesToBytes *gameKibibytesToBytesAddr  = (GameKibibytesToBytes *)GetProcAddress(libHandle, "gameKibibytesToBytes");
        GameMebibytesToBytes *gameMebibytesToBytesAddr  = (GameMebibytesToBytes *)GetProcAddress(libHandle, "gameMebibytesToBytes");
        GameGibibytesToBytes *gameGibibytesToBytesAddr  = (GameGibibytesToBytes *)GetProcAddress(libHandle, "gameGibibytesToBytes");
        GameTebibyteToBytes *gameTebibyteToBytesAddr    = (GameTebibyteToBytes *)GetProcAddress(libHandle, "gameTebibyteToBytes");

        if (gameUpdateAddr) {
            gameCode->gameUpdate = gameUpdateAddr;
        } else {
            assert(!"unable to fund gameUpdate function");
        }

        if (gameInitFrameBufferAddr) {
            gameCode->gameInitFrameBuffer = gameInitFrameBufferAddr;
        } else {
            assert(!"unable to fund gameInitFrameBuffer function");
        }

        if (gameInitAudioBufferAddr) {
            gameCode->gameInitAudioBuffer = gameInitAudioBufferAddr;
        } else {
            assert(!"unable to fund gameInitAudioBuffer function");
        }

        if (gameKibibytesToBytesAddr) {
            gameCode->gameKibibytesToBytes = gameKibibytesToBytesAddr;
        } else {
            assert(!"unable to fund gameKibibytesToBytes function");
        }

        if (gameMebibytesToBytesAddr) {
            gameCode->gameMebibytesToBytes = gameMebibytesToBytesAddr;
        } else {
            assert(!"unable to fund gameMebibytesToBytes function");
        }

        if (gameGibibytesToBytesAddr) {
            gameCode->gameGibibytesToBytes = gameGibibytesToBytesAddr;
        } else {
            assert(!"unable to fund gameGibibytesToBytes function");
        }

        if (gameTebibyteToBytesAddr) {
            gameCode->gameTebibyteToBytes = gameTebibyteToBytesAddr;
        } else {
            assert(!"unable to fund gameTebibyteToBytes function");
        }
    }
}