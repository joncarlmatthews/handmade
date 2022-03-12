// Windows API. Visual Studio freaks out if it's not the first include.
#include <windows.h>

#include <strsafe.h> // sprintf_s support
#include <dsound.h>  // Direct Sound for audio output.
#include <xinput.h>  // Xinput for receiving controller input.

#include "..\Game\types.h" // Basic types
#include "..\Game\util.h" // Function signatures and basic types that are shared across the game and platform layer
#include "..\Game\game.h" // Game layer specific function signatures
#include "win32_handmade.h" // Platform layer specific function signatures

// Include the definitions of the utility/helper Functions that are
// shared across the game and platform layer
#include "..\Game\util.cpp"

// Whether or not the application is running/paused
global_var bool8 running = TRUE;
global_var bool8 paused = FALSE;

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
    LPVOID startAddress = (LPVOID)memoryStartAddress;
    return VirtualAlloc(startAddress, memorySizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
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
#if HANDMADE_LOCAL_BUILD

    DEBUG_PLATFORM_LOG(DEBUG_platformLog)
    {
        OutputDebugStringA(buff);
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
        file.memory = platformAllocateMemory(thread, 0, sizeInBytes);

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

#endif

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
    win32LoadXInputDLLFunctions();

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

    // Physically open the window using CreateWindowEx. (WS_EX_TOPMOST is
    // handy to have the game window not disappear behind Visual Studio dialogs when debugging
    DWORD windowStyle = NULL;
    //windowStyle = WS_EX_TOPMOST;
    HWND window = CreateWindowEx(windowStyle,
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

    // Get the thread's content (@TODO JM)
    PlatformThreadContext thread = { 0 };

    // Create a Win32 state object to hold persistent data for the platform layer.
    Win32State win32State = { 0 };

#if HANDMADE_LOCAL_BUILD
    win32State.inputRecording = 0;
    win32State.inputPlayback = 0;
#endif

    // Calculate the absolute path to this executable.
    wchar_t absPath[MAX_PATH] = { 0 };
    char    absPathA[MAX_PATH] = { 0 };
    win32GetAbsolutePath(absPath);
    utilWideCharToChar(absPath, MAX_PATH, absPathA, MAX_PATH);

    wcsncpy_s(win32State.absPath, MAX_PATH, absPath, MAX_PATH);
    strncpy_s(win32State.absPathA, sizeof(win32State.absPathA), absPathA, MAX_PATH);

    GameCode gameCode = { 0 };
    win32LoadGameDLLFunctions(win32State.absPath, &gameCode);

    /*
     * Game memory
     */
#if HANDMADE_LOCAL_BUILD
    uint64 memoryStartAddress = utilTebibyteToBytes(4);
#else
    uint64 memoryStartAddress = 0;
#endif

    GameMemory memory = {0};
    memory.platformAllocateMemory = &platformAllocateMemory;
    memory.platformFreeMemory = &platformFreeMemory;
    memory.platformControllerVibrate = &platformControllerVibrate;

#if HANDMADE_LOCAL_BUILD
    memory.DEBUG_platformLog = &DEBUG_platformLog;
    memory.DEBUG_platformReadEntireFile = &DEBUG_platformReadEntireFile;
    memory.DEBUG_platformWriteEntireFile = &DEBUG_platformWriteEntireFile;
    memory.DEBUG_platformFreeFileMemory = &DEBUG_platformFreeFileMemory;
#endif

    memory.permanentStorageSizeInBytes = utilMebibytesToBytes(64);
    memory.transientStorageSizeInBytes = utilGibibytesToBytes(1);

    uint64 memoryTotalSize = (memory.permanentStorageSizeInBytes + memory.transientStorageSizeInBytes);

    memory.permanentStorage = platformAllocateMemory(&thread, memoryStartAddress, memoryTotalSize);
    memory.transientStorage = (((uint8 *)memory.permanentStorage) + memory.permanentStorageSizeInBytes);

    if (memory.permanentStorage && memory.transientStorage) {

        win32State.gameMemorySize = memoryTotalSize;
        win32State.gameMemory = memory.permanentStorage;

#if HANDMADE_LOCAL_BUILD
        memory.recordingStorageGameState   = platformAllocateMemory(&thread, (memoryStartAddress + memoryTotalSize), memoryTotalSize);
        memory.recordingStorageInput       = platformAllocateMemory(&thread, (memoryStartAddress + (memoryTotalSize * 2)), memoryTotalSize);

        win32State.gameMemoryRecordedState = memory.recordingStorageGameState;
        win32State.gameMemoryRecordedInput = memory.recordingStorageInput;
#endif

        /*
         * Framerate fixing.
         */
        Win32FixedFrameRate win32FixedFrameRate = {0};
        win32FixedFrameRate.capMode = FRAME_RATE_CAP_MODE_SLEEP;
        win32FixedFrameRate.monitorRefreshRate = 60;
        win32FixedFrameRate.gameTargetFPS = 30;
        win32FixedFrameRate.gameTargetMSPerFrame = (1000.0f / (float32)win32FixedFrameRate.gameTargetFPS);

        // Get the refresh rate of the monitor from the Windows API.
        int32 gdcRes = GetDeviceCaps(deviceHandleForWindow, VREFRESH);

        if (gdcRes > 1) {
            win32FixedFrameRate.monitorRefreshRate = (uint8)gdcRes;
        }

        // Set the system's minimum timer resolution to 1 millisecond
        // so that calls to the Windows Sleep() function are more
        // granular. E.g. the wake from the Sleep() will be checked
        // every 1ms, rather than the system default.
        if (win32FixedFrameRate.capMode == FRAME_RATE_CAP_MODE_SLEEP){
            win32FixedFrameRate.timeOutIntervalMS = 1;
            win32FixedFrameRate.timeOutIntervalSet = timeBeginPeriod(win32FixedFrameRate.timeOutIntervalMS);
        }

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
        win32InitFrameBuffer(&thread, &win32FrameBuffer, 960, 540);

        

        /*
         * Controllers
         */

        // How many controllers does the platform layer support?
        ControllerCounts controllerCounts = {0};
        controllerCounts.gameMaxControllers = MAX_CONTROLLERS;
        controllerCounts.platformMaxControllers = XUSER_MAX_COUNT;

        // An array to hold pointers to the old and new instances of the inputs.
        GameInput GameInputInstances[2] = {0};

        // We save a copy of what we've written to the inputs (in the old instance variable)
        // so we can compare last frame's values to this frame's values.
        GameInput *gameInput        = &GameInputInstances[0];
        GameInput *gameInputOld     = &GameInputInstances[1];

        // Mouse support
        GameMouseInput mouse = { 0 };

        // Assign the mouse object to the game input
        gameInput->mouse = mouse;

        // Keyboard support
        GameControllerInput keyboard = { 0 };
        keyboard.isConnected = 1; // @TODO(JM) check that it's actually is connected.

        // Assign the keyboard object to the game input
        gameInput->controllers[0] = keyboard; // Assign the first game input controller as the keyboard
        controllerCounts.connectedControllers = 1; // @TODO(JM) Support for multiple controllers

        // Get the number of processor clock cycles
        uint64 runningProcessorClockCyclesCounter = __rdtsc();

        // Get the current time for profiling FPS
        LARGE_INTEGER netFrameTime = win32GetTime();

        /**
         * MAIN GAME LOOP
         */
        while (running) {

            // Get the position of the mouse
            win32GetMousePosition(window, &gameInput->mouse);

            // Handle the Win32 message loop and handle mouse and keyboard input
            win32ProcessMessages(window, gameInput, *gameInputOld, &win32State);

            if (paused) {
                win32AudioBufferToggleStop(&win32AudioBuffer);
                continue;
            } else {
                win32AudioBufferTogglePlay(&win32AudioBuffer);
            }

            // After processing our messages, we can now (in our "while running = true"
            // loop) do what we like..!

            /*
             * Controller input stuff
             */

            // Iterate over each controller and get its state.
            DWORD dwResult;

            uint8 gamePadsAdded = 0;

            for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {

                XINPUT_STATE xinputControllerInstance = { 0 };
                SecureZeroMemory(&xinputControllerInstance, sizeof(XINPUT_STATE));

                // Simply get the state of the controller from XInput.
                dwResult = XInputGetState(controllerIndex, &xinputControllerInstance);

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
                XINPUT_GAMEPAD *gamepad = &xinputControllerInstance.Gamepad;

                uint8 ourControllerIndex = ((uint8)controllerIndex + 1);

                GameControllerInput *gameController = &gameInput->controllers[ourControllerIndex];

                gameController->isConnected = true;

                win32ProcessXInputControllerButton(&gameController->dPadUp,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_DPAD_UP);

                win32ProcessXInputControllerButton(&gameController->dPadDown,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_DPAD_DOWN);

                win32ProcessXInputControllerButton(&gameController->dPadLeft,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_DPAD_LEFT);

                win32ProcessXInputControllerButton(&gameController->dPadRight,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_DPAD_RIGHT);

                win32ProcessXInputControllerButton(&gameController->up,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_Y);

                win32ProcessXInputControllerButton(&gameController->down,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_A);

                win32ProcessXInputControllerButton(&gameController->right,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_B);

                win32ProcessXInputControllerButton(&gameController->left,
                                                    gamepad,
                                                    XINPUT_GAMEPAD_X);

                // Left controller thumbstick support...
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

                gameController->leftThumbstick.position.x = leftThumbstickX;
                gameController->leftThumbstick.position.y = leftThumbstickY;

                // If the user is using the thumbstick, then set the controller
                // to analog mode, if they're using the D-pad set to non-alanlog mode
                if (!gameController->isAnalog) {
                    if ((leftThumbstickX != 0.0f) || (leftThumbstickY != 0.0f)) {
                        gameController->isAnalog = true;
                    }
                }

                if (gameController->isAnalog) {
                    if ((gameController->dPadUp.endedDown)
                        || (gameController->dPadDown.endedDown)
                        || (gameController->dPadLeft.endedDown)
                        || (gameController->dPadRight.endedDown)
                        ) {
                        gameController->isAnalog = false;
                    }
                }

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
                    

#if defined(HANDMADE_DEBUG_AUDIO)

                    gameAudioBuffer.playCursorPosition = playCursorOffsetInBytes;
                    gameAudioBuffer.writeCursorPosition = writeCursorOffsetInBytes;
                    gameAudioBuffer.lockSizeInBytes = lockSizeInBytes;

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

#endif
                } else {
                    OutputDebugStringA("Could not get the position of the play and write cursors in the secondary sound buffer");
                }

            } // Audio buffer created.

#if HANDMADE_LOCAL_BUILD

            // Recording/playback
            if (win32State.inputRecording) {
                win32RecordInput(&win32State, gameInput);
            }

            if (win32State.inputPlayback) {
               win32PlaybackInput(&win32State, gameInput);
            }
#endif

            // Create the game's audio buffer
            gameCode.gameInitAudioBuffer(&thread,
                                            &memory,
                                            &gameAudioBuffer,
                                            lockSizeInBytes,
                                            win32AudioBuffer.bytesPerSample,
                                            win32AudioBuffer.bufferSizeInBytes);

            // Create the game's frame buffer
            GameFrameBuffer gameFrameBuffer = {0};
            gameCode.gameInitFrameBuffer(&thread,
                                            &gameFrameBuffer,
                                            win32FrameBuffer.height,
                                            win32FrameBuffer.width,
                                            win32FrameBuffer.bytesPerPixel,
                                            win32FrameBuffer.byteWidthPerRow,
                                            win32FrameBuffer.memory);

            // Main game code.
            gameCode.gameUpdate(&thread, &memory, &gameFrameBuffer, &gameAudioBuffer, GameInputInstances, &controllerCounts);

            win32LoadGameDLLFunctions(win32State.absPath, &gameCode);

            // Output the audio buffer in Windows.
            win32WriteAudioBuffer(&win32AudioBuffer, lockOffsetInBytes, lockSizeInBytes, &gameAudioBuffer);

            // Take a copy of this frame's controller inputs
            gameInputOld->mouse = gameInput->mouse;
            gameInputOld->controllers[0] = gameInput->controllers[0];

            // Display the frame buffer in Windows. AKA "flip the frame" or "page flip".
            win32ClientDimensions clientDimensions = win32GetClientDimensions(window);
            win32DisplayFrameBuffer(deviceHandleForWindow, win32FrameBuffer, clientDimensions.width, clientDimensions.height);

            // How long did this game loop (frame) take? (E.g. 2ms)
            LARGE_INTEGER gameLoopTime = win32GetTime();

            float32 millisecondsElapsedForFrame = win32GetElapsedTimeMS(netFrameTime, gameLoopTime, globalQPCFrequency);

#if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)

            {
                char output[100] = { 0 };
                sprintf_s(output, sizeof(output),
                            "Time for frame to complete: %f milliseconds\n",
                            millisecondsElapsedForFrame);
                OutputDebugStringA(output);
            }

            {
                char output[100] = { 0 };
                sprintf_s(output, sizeof(output),
                            "Target time for frame to complete: %f milliseconds\n",
                            win32FixedFrameRate.gameTargetMSPerFrame);
                OutputDebugStringA(output);
            }
#endif
                
            // Cap frame rate to target FPS if we're running ahead.
            if (millisecondsElapsedForFrame < win32FixedFrameRate.gameTargetMSPerFrame){

                float32 needToSleepForMS = (win32FixedFrameRate.gameTargetMSPerFrame - millisecondsElapsedForFrame);

                #if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)
                {
                    char output[100] = { 0 };
                    sprintf_s(output, sizeof(output),
                                "Need to sleep for: %f milliseconds (%f)\n",
                                needToSleepForMS, (millisecondsElapsedForFrame + needToSleepForMS));
                    OutputDebugStringA(output);
                }
                #endif

                // Win32 Sleep?
                if (win32FixedFrameRate.capMode == FRAME_RATE_CAP_MODE_SLEEP) {

                    INT msToSleepI = (INT)needToSleepForMS;

                    #if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)
                    {
                        char output[100] = { 0 };
                        sprintf_s(output, sizeof(output),
                                    "Sleeping for... %i\n",
                                    msToSleepI);
                        OutputDebugStringA(output);
                    }
                    #endif

                    Sleep(msToSleepI);

                    // Update the MS taken for this frame
                    millisecondsElapsedForFrame = win32GetElapsedTimeMS(gameLoopTime, win32GetTime(), globalQPCFrequency);

                    // Spin lock for any fractions of time left
                    if (millisecondsElapsedForFrame < win32FixedFrameRate.gameTargetMSPerFrame) {
                        while (millisecondsElapsedForFrame < win32FixedFrameRate.gameTargetMSPerFrame) {
                            millisecondsElapsedForFrame = win32GetElapsedTimeMS(gameLoopTime, win32GetTime(), globalQPCFrequency);
                        }
                    }

                } else {

                    // Spin lock for full duration of time
                    #if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)
                        OutputDebugStringA("Entering spin lock...\n");
                    #endif

                    while (millisecondsElapsedForFrame < win32FixedFrameRate.gameTargetMSPerFrame) {
                        millisecondsElapsedForFrame = win32GetElapsedTimeMS(gameLoopTime, win32GetTime(), globalQPCFrequency);
                    }
                }

            }else if((INT)millisecondsElapsedForFrame > (INT)win32FixedFrameRate.gameTargetMSPerFrame){

                // @TODO(JM) Missed target framerate. Log.

                #if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)
                {
                    char output[500] = { 0 };
                    sprintf_s(output, sizeof(output),
                                "======================================MISSED================================ (%f > %f)\n\n",
                                millisecondsElapsedForFrame,
                                win32FixedFrameRate.gameTargetMSPerFrame);
                    OutputDebugStringA(output);
                }
                #endif
            }

            // Set net frame time (E.g. 33.33ms or 16.66ms)
            netFrameTime = win32GetTime();
            float32 netFrameTimeMS = win32GetElapsedTimeMS(gameLoopTime, netFrameTime, globalQPCFrequency);

            // Set the ms per frame to the game input object so we can regulate movement speed
            gameInput->msPerFrame = netFrameTimeMS;

            #if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_FPS)
                {
                    char output[100] = { 0 };
                    sprintf_s(output, sizeof(output),
                                "Net time for frame to complete: %f milliseconds\n\n",
                                netFrameTimeMS);

                    OutputDebugStringA(output);
                }
            #endif

            #if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_CLOCKCYCLES)
            {
                // Calculate how many processor clock cycles elapsed for this frame.
                // @NOTE(JM) __rdtsc is only for dev and not for relying on for shipped code that will run on end user's machine.
                uint64 processorClockCyclesAfterFrame = __rdtsc();
                int64 processorClockCyclesElapsedForFrame = (processorClockCyclesAfterFrame - runningProcessorClockCyclesCounter);
                float32 clockCycles_mega = ((float32)processorClockCyclesElapsedForFrame / 1000000.0f); // processorClockCyclesElapsedForFrame is in the millions, dividing by 1m to give us a "mega" (e.g. megahertz) value.

                // Calculate the FPS given the speed of this current frame.
                float32 fps = (1000.0f / (float32)netFrameTimeMS);

                // Calculate the processor running speed in GHz
                float32 processorSpeed = ((uint64)(fps * clockCycles_mega) / 100.0f);

                // Reset the running clock cycles.
                runningProcessorClockCyclesCounter = processorClockCyclesAfterFrame;

                // Console log the speed:
                char output[100] = { 0 };
                sprintf_s(output, sizeof(output),
                            "Cycles: %.1fm (%.2f GHz).\n",
                            clockCycles_mega, processorSpeed);
                OutputDebugStringA(output);
            }
            #endif


        } // game loop

        if (win32FixedFrameRate.capMode == FRAME_RATE_CAP_MODE_SLEEP) {
            if (TIMERR_NOERROR == win32FixedFrameRate.timeOutIntervalSet) {
                timeEndPeriod(win32FixedFrameRate.timeOutIntervalMS);
            }
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
            running = FALSE;
        } break;

        // Called when the user requests to close the window.
        case WM_CLOSE: {
            // @TODO(JM) Display "are you sure" message to user?
            running = FALSE;
        } break;

        case WM_QUIT: {
            running = FALSE;
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

            // Paint the whole screen black (stops the artifacts around the frame buffer)
            int32 screenWidth = GetDeviceCaps(deviceHandleForWindow, HORZRES);
            int32 screenHeight = GetDeviceCaps(deviceHandleForWindow, VERTRES);
            if (screenWidth && screenHeight){
                PatBlt(deviceHandleForWindow, 0, 0, screenWidth, screenHeight, BLACKNESS);
            }

            win32DisplayFrameBuffer(deviceHandleForWindow, win32FrameBuffer, clientDimensions.width, clientDimensions.height);

            // End the paint request and releases the device context.
            EndPaint(window, &paint);
        } break;

        // Keyboard messages. Now handled within our Win32 game loop.
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {
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
internal_func void win32InitFrameBuffer(PlatformThreadContext *thread, Win32FrameBuffer *buffer, uint32 width, int32 height)
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
    buffer->memory = platformAllocateMemory(thread, 0, bitmapMemorySizeInBytes);

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
    // For prototyping purposes, we are always going to blit 1-to-1 pixels to make
    // sure we don't introduce artifacts. We can achieve this by not allowing the image
    // to stretch (by setting the destination width and height to be fixed to what the
    // source width and height are). This will help us when it comes to learning how
    // to write our renderer.
    bool stretch = false;

    if (!stretch) {
        width = buffer.width;
        height = buffer.height;
    }

    uint8 offsetX = 15;
    uint8 offsetY = 15;
    
    // StretchDIBits function copies the data of a rectangle of pixels to 
    // the specified destination. The first parameter is the handle for
    // the destination's window that we want to write the data to.
    // Pixels are drawn to screen from the top left to the top right, then drops a row,
    // draws from left to right and so on. Finally finishing on the bottom right pixel.
    StretchDIBits(deviceHandleForWindow,
                    offsetX,
                    offsetY,
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
    DSBUFFERDESC primarySoundBufferDesc = {0};
    SecureZeroMemory(&primarySoundBufferDesc, sizeof(DSBUFFERDESC));

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
    DSBUFFERDESC secondarySoundBufferDesc = { 0 };
    SecureZeroMemory(&secondarySoundBufferDesc, sizeof(DSBUFFERDESC));

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

#if defined(HANDMADE_DEBUG)
    OutputDebugStringA("Primary & secondary successfully buffer created\n");
#endif
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

internal_func void win32ProcessMessages(HWND window,
                                        GameInput *gameInput,
                                        GameInput oldGameInput,
                                        Win32State *win32State)
{
    MSG message = {0};

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

            // Mouse left click
            case WM_LBUTTONDOWN:
            {
                GameControllerBtnState state = {0};
                state.endedDown = TRUE;
                gameInput->mouse.leftClick = state;
            } break;

            case WM_LBUTTONUP:
            {
                GameControllerBtnState state = {0};
                state.endedDown = FALSE;
                gameInput->mouse.leftClick = state;
            } break;

            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP: {
            } break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYUP:{

                /*
                 * lParam bitmask. Written from right to left
                 *
                 * @see https://docs.microsoft.com/en-us/windows/win32/inputdev/wm-keydown
                 *
                 *  First two bytes              Second two bytes
                 * |-------------------------|  |----------------|
                 * 31 30 29 28-25 24 23-16      15-0
                 * 0  0  0  0000  1  01001011   00000000 00000001
                */

                // Which key was pressed?
                WPARAM vkCode = message.wParam;

                // Was the button down or up?
                BOOL keyDown = FALSE;

                if (0 == (message.lParam & ((uint64)1 << 31))){
                    keyDown = TRUE;
                }

                GameControllerBtnState state = { 0 };

                if (keyDown) {
                    state.endedDown = TRUE;
                } else {
                    state.endedDown = FALSE;
                }

                switch (vkCode) {
                    case 'W': {
                        state.wasDown = oldGameInput.controllers[0].dPadUp.endedDown;
                        gameInput->controllers[0].dPadUp = state;
                    } break;
                    case 'A': {
                        state.wasDown = oldGameInput.controllers[0].dPadLeft.endedDown;
                        gameInput->controllers[0].dPadLeft = state;
                    } break;
                    case 'S': {
                        state.wasDown = oldGameInput.controllers[0].dPadDown.endedDown;
                        gameInput->controllers[0].dPadDown = state;
                    } break;
                    case 'D': {
                        state.wasDown = oldGameInput.controllers[0].dPadRight.endedDown;
                        gameInput->controllers[0].dPadRight = state;
                    } break;
                    case 'Q': {
                        state.wasDown = oldGameInput.controllers[0].shoulderL1.endedDown;
                        gameInput->controllers[0].shoulderL1 = state;
                    } break;
                    case 'E': {
                        state.wasDown = oldGameInput.controllers[0].shoulderR1.endedDown;
                        gameInput->controllers[0].shoulderR1 = state;
                    } break;

#if HANDMADE_LOCAL_BUILD
                    // Playback recording/looping
                    case 'L': {
                        if (keyDown) {
                            if (0 == win32State->inputPlayback){ // Lock the developer into the loop. Have to rebuild to exit.
                                if (!win32State->inputRecording) {
                                    win32BeginInputRecording(win32State);
                                }
                                else {
                                    win32EndInputRecording(win32State);
                                    win32BeginRecordingPlayback(win32State);
                                }
                            }
                        }
                    } break;

                    case 'P':{
                        if (keyDown) {
                            if (paused) {
                                paused = false;
                            } else {
                                paused = true;
                            }
                        }
                    } break;
#endif
                }
            }

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

internal_func void win32ProcessXInputControllerButton(GameControllerBtnState *currentState,
                                                        XINPUT_GAMEPAD *gamepad,
                                                        uint16 gamepadButtonBit)
{
    (*currentState).endedDown = ((*gamepad).wButtons & gamepadButtonBit);
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

internal_func void win32LoadXInputDLLFunctions(void)
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

internal_func void win32LoadGameDLLFunctions(wchar_t *absPath, GameCode *gameCode)
{
    wchar_t gameDLLFileName[20] = L"Game.dll";
    wchar_t gameDLLFilePath[276] = { 0 };

    wchar_t gameCopyDLLFileName[20] = L"Game_copy.dll";
    wchar_t gameCopyDLLFilePath[276] = {0};

    utilConcatStringsW(absPath, MAX_PATH, gameDLLFileName, 20, gameDLLFilePath, 276);
    utilConcatStringsW(absPath, MAX_PATH, gameCopyDLLFileName, 20, gameCopyDLLFilePath, 276);

    BOOL loadGameCode = false;

    // Does the copy yet exist?
    DWORD dwAttrib = GetFileAttributes(gameCopyDLLFilePath);

    if (!(dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY))) {

#if defined(HANDMADE_DEBUG)
        wchar_t buff[500] = { 0 };
        swprintf_s(buff, 500, L"Game_copy.dll doesnt exist, going to copy...\n");
        OutputDebugString(buff);
#endif

        BOOL res = CopyFile(gameDLLFilePath, gameCopyDLLFilePath, false);

#if defined(HANDMADE_DEBUG)
        if (!res) {
            wchar_t buff[500] = { 0 };
            swprintf_s(buff, 500, L"Game_temp.dll doesnt exist, but could not copy: 0x%X\n", GetLastError()); // 0x7E == The specified module could not be found.
            OutputDebugString(buff);
            assert(!"Game code can not be loaded");
        }
#endif

        loadGameCode = true;

    } else {

        // File already exists...

        // Check to see if we actually need to do the copy.
        FILETIME lastWriteTimeGame = {};
        lastWriteTimeGame = win32GetFileLastWriteDate(gameDLLFilePath);
        FILETIME lastWriteTimeGameCopy = {};
        lastWriteTimeGameCopy = win32GetFileLastWriteDate(gameCopyDLLFilePath);

        if (CompareFileTime(&lastWriteTimeGame, &lastWriteTimeGameCopy) != 0) {

            // Free a lock if there is one...
            if (gameCode->dllHandle != 0x0) {
                BOOL res = FreeLibrary((HMODULE)gameCode->dllHandle);

#if defined(HANDMADE_DEBUG)
                if (!res) {
                    wchar_t buff[500] = { 0 };
                    swprintf_s(buff, 500, L"Could not free DLL handle: 0x%X\n", GetLastError()); // 0x7E == The specified module could not be found.
                    OutputDebugString(buff);
                }
#endif

            }

            BOOL res = CopyFile(gameDLLFilePath, gameCopyDLLFilePath, false);

#if defined(HANDMADE_DEBUG)
            if (!res) {
                wchar_t buff[500] = { 0 };
                swprintf_s(buff, 500, L"DLL copy failed: 0x%X\n", GetLastError()); // 0x20 = The process cannot access the file because it is being used by another process.
                OutputDebugString(buff);
            }
#endif

            loadGameCode = true;
        } else {

            if (gameCode->dllHandle == 0x0) {
                loadGameCode = true;
            }
        }
    }

    if (loadGameCode) {
        HMODULE libHandle = LoadLibrary(gameCopyDLLFilePath);

        bool8 valid = 1;

        if (libHandle) {

            gameCode->dllHandle = libHandle;

            GameUpdate *gameUpdateAddr = (GameUpdate *)GetProcAddress(libHandle, "gameUpdate");
            GameInitFrameBuffer *gameInitFrameBufferAddr = (GameInitFrameBuffer *)GetProcAddress(libHandle, "gameInitFrameBuffer");
            GameInitAudioBuffer *gameInitAudioBufferAddr = (GameInitAudioBuffer *)GetProcAddress(libHandle, "gameInitAudioBuffer");

            if (gameUpdateAddr) {
                gameCode->gameUpdate = gameUpdateAddr;
            } else {
                assert(!"unable to find gameUpdate");
                valid = 0;
            }

            if (gameInitFrameBufferAddr) {
                gameCode->gameInitFrameBuffer = gameInitFrameBufferAddr;
            } else {
                assert(!"unable to find gameInitFrameBuffer");
                valid = 0;
            }

            if (gameInitAudioBufferAddr) {
                gameCode->gameInitAudioBuffer = gameInitAudioBufferAddr;
            } else {
                assert(!"unable to find gameInitAudioBuffer");
                valid = 0;
            }

        } else {
            assert(!"unable to load game code");
            valid = 0;
        }

        if (!valid) {
            gameCode->gameUpdate = &gameUpdateStub;
            gameCode->gameInitFrameBuffer = &gameInitFrameBufferStub;
            gameCode->gameInitAudioBuffer = &gameInitAudioBufferStub;
        }
    }
}

internal_func void win32GetAbsolutePath(wchar_t *path)
{
    // Get the module path for the running exe
    wchar_t modulePath[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, modulePath, MAX_PATH);

    // Set a pointer to the last backslash
    void *ptr = 0x0;
    for (size_t i = 0; i < MAX_PATH; i++) {
        if (modulePath[i] == '\\') {
            ptr = &modulePath[i];
        }
        if (modulePath[i] == '\0') {
            if (ptr == 0x0) {
                ptr = &modulePath[i];
            }
            break;
        }
    }

    // Write to a new char array, copying the contents of the
    // module path, until we hit the last slash
    for (size_t i = 0; i < MAX_PATH; i++) {
        path[i] = modulePath[i];
        if (&modulePath[i] == ptr) {
            break;
        }
        if (modulePath[i] == '\0') {
            break;
        }
    }

    /*
    swprintf_s(moduleDirectory, MAX_PATH, L"%ls", moduleDirectory);
    OutputDebugString(moduleDirectory);
    */
}

internal_func FILETIME win32GetFileLastWriteDate(const wchar_t *filename)
{
    WIN32_FILE_ATTRIBUTE_DATA fileData = { 0 };

    BOOL res = GetFileAttributesExW(filename, GetFileExInfoStandard, &fileData);

    if (!res) {
        assert(!"Cannot get file data");
    }

    return fileData.ftLastWriteTime;
}

internal_func void win32GetMousePosition(HWND window, GameMouseInput *mouseInput)
{
    if (paused){
        return;
    }

    // Get the mouse inputs from the Windows API
    POINT point = { 0 };

    mouseInput->isConnected = GetCursorPos(&point);

    if (mouseInput->isConnected) {

        if (ScreenToClient(window, &point)) {
            mouseInput->position.x = point.x;
            mouseInput->position.y = point.y;
        }
    }
}

#if HANDMADE_LOCAL_BUILD

internal_func void win32BeginInputRecording(Win32State *win32State)
{
    CopyMemory(win32State->gameMemoryRecordedState, win32State->gameMemory, win32State->gameMemorySize);
    win32State->inputRecording = 1;
}

internal_func void win32EndInputRecording(Win32State *win32State)
{
    win32State->inputRecording = 0;
}

internal_func void win32RecordInput(Win32State *win32State, GameInput *gameInput)
{
    uint64 offset = 0;
    if (win32State->recordingWriteFrameIndex >= 1) {
        offset = ((sizeof(*gameInput)) * win32State->recordingWriteFrameIndex);
    }
    CopyMemory(((CHAR*)win32State->gameMemoryRecordedInput + offset), gameInput, sizeof(*gameInput));
    win32State->recordingWriteFrameIndex += 1;
}

internal_func void win32BeginRecordingPlayback(Win32State *win32State)
{
    // Read out the copy of the game's memory from the recorded memory block.
    CopyMemory(win32State->gameMemory, win32State->gameMemoryRecordedState, win32State->gameMemorySize);
    win32State->inputPlayback = 1;
}

internal_func void win32EndRecordingPlayback(Win32State *win32State)
{
    win32State->recordingReadFrameIndex = 0;
    win32State->inputPlayback = 0;
}

internal_func void win32PlaybackInput(Win32State *win32State, GameInput *gameInput)
{
    uint64 offset = 0;
    if (win32State->recordingReadFrameIndex >= 1) {
        offset = ((sizeof(*gameInput)) * win32State->recordingReadFrameIndex);
    }
    CopyMemory(gameInput, ((CHAR*)win32State->gameMemoryRecordedInput + offset), sizeof(*gameInput));
    win32State->recordingReadFrameIndex += 1;

    if (win32State->recordingReadFrameIndex == win32State->recordingWriteFrameIndex) {
        // We have read all the bytes from the recorded input, loop back to the start...
        win32EndRecordingPlayback(win32State);
        win32BeginRecordingPlayback(win32State);
    }
}
#endif