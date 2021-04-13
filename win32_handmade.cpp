// Windows API. Visual Studio freaks out if it's not the first include.
#include <windows.h>

#include <strsafe.h> // sprintf_s support
#include <dsound.h>  // Direct Sound for audio output.
#include <xinput.h>  // Xinput for receiving controller input. 

#define PIf                 3.14159265359f
#define LOG_LEVEL_INFO      0x100
#define LOG_LEVEL_WARN      0x200
#define LOG_LEVEL_ERROR     0x300

//=======================================
// Game layer
//=======================================

#include "handmade.cpp"

//=======================================
// End of game layer
//=======================================

// Platform layer specific function signatures
#include "win32_handmade.h"

// Display output debug strings?
const bool DEBUG_OUTPUT = FALSE;

// Whether or not the application is running
global_var bool running;

// Win32 frame buffer.
global_var Win32FrameBuffer win32FrameBuffer = {0};

// Win32 audio buffer.
global_var Win32AudioBuffer win32AudioBuffer = {0};

// XInput support
typedef DWORD WINAPI XInputGetStateDT(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState);
typedef DWORD WINAPI XInputSetStateDT(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION *pVibration);

global_var XInputGetStateDT *XInputGetState_ = XInputGetStateStub;
global_var XInputSetStateDT *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

// Direct sound support
typedef HRESULT WINAPI DirectSoundCreateDT(LPGUID lpGuid, LPDIRECTSOUND *ppDS, LPUNKNOWN  pUnkOuter);

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
    LARGE_INTEGER perfFrequencyCounterRes;
    QueryPerformanceFrequency(&perfFrequencyCounterRes);
    int64 countersPerSecond = perfFrequencyCounterRes.QuadPart;

    // Load XInput DLL functions.
    loadXInputDLLFunctions();

    // Create a new window struct and set all of it's values to 0.
    WNDCLASS windowClass = {};

    // Define the window's attributes. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
    windowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;

    // Callback to handle any messages sent to the window (resize, close etc).
    windowClass.lpfnWndProc = win32MainWindowCallback;

    // Instance of the running application.
    windowClass.hInstance = instance;
    windowClass.lpszClassName = "handmadeHeroWindowClass";

    // Registers the window class for subsequent use in calls to 
    // the CreateWindowEx function.
    if (!RegisterClass(&windowClass)) {

        // TODO(JM) Log error.
        OutputDebugString("Error 1. windowClass not registered\n");
        return FALSE;
    }

    // Physically open the window using CreateWindowEx
    HWND window = CreateWindowEx(NULL,
                                    windowClass.lpszClassName,
                                    "Handmade Hero",
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
        OutputDebugString("Error 2. window not created via CreateWindowEx\n");
        return FALSE;
    }

    // Usually you would call GetDC, do your work and then call ReleaseDC within
    // each loop, however because we specified CS_OWNDC, we can call it once
    // and use it forever.
    HDC deviceHandleForWindow = GetDC(window);

    /*
     * Game memory
     */

    GameMemory memory = {};
    memory.permanentStorageSizeInBytes = mebibytesToBytes(64);
    memory.permanentStorage = VirtualAlloc(NULL, memory.permanentStorageSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    memory.transientStorageSizeInBytes = gibibytesToBytes(1);
    memory.transientStorage = VirtualAlloc(NULL, memory.transientStorageSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (memory.permanentStorage && memory.transientStorage) {

        /*
        * Audio
        */

        // Create the Windows audio buffer
        win32InitAudioBuffer(window, &win32AudioBuffer);

        // Kick off playing the Windows audio buffer
        win32AudioBuffer.buffer->Play(0, 0, DSBPLAY_LOOPING);

        // Create the game audio buffer. We do this outside of the game loop as we need
        // to allocate memory
        AudioBuffer audioBuffer = { 0 };
        // @TODO(JM) move the audio memory to the GameMemory object
        audioBuffer.memory = VirtualAlloc(NULL, win32AudioBuffer.bufferSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        /*
        * Graphics
        */

        // Create the Windows frame buffer
        win32InitFrameBuffer(&win32FrameBuffer, 1920, 1080);

        /*
        * Controllers
        */

        // An array to hold pointers to the old and new instances of the inputs.
        GameInput inputInstances[2] = {};

        // We save a copy of what we've written to the inputs (in the old instance variable)
        // so we can compare last frame's values to this frames values.
        GameInput *inputNewInstance = &inputInstances[0];
        GameInput *inputOldInstance = &inputInstances[1];

        // How many controllers does the platform layer support?
        uint8 maxControllers = MAX_CONTROLLERS;
        if (XUSER_MAX_COUNT < maxControllers) {
            maxControllers = XUSER_MAX_COUNT;
        }

        // Running query perforamce counter for profiling the game loop
        LARGE_INTEGER runningPerformanceCounter;
        QueryPerformanceCounter(&runningPerformanceCounter);

        // Get the number of processor clock cycles
        uint64 processorClockCycles = __rdtsc();

        running = true;

        /**
         * MAIN GAME LOOP
         */
        while (running) {

            MSG message;

            // Message loop. Retrieves all messages (from the calling thread's message queue)
            // that are sent to the window. E.g. clicks and key inputs.
            while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {

                // If the message received was a quit message, then toggle our
                // running flag to false to break out of this loop on the next
                // iteration.
                if (message.message == WM_QUIT) {
                    running = false;
                }

                // Get the message ready for despatch.
                TranslateMessage(&message);

                // Dispatch the message to the application's window procedure.
                // @link win32MainWindowCallback
                DispatchMessage(&message);

            } // PeekMessage loop

            // After processing our messages, we can now (in our "while running = true"
            // loop) do what we like! WM_SIZE and WM_PAINT get called as soon as the
            // application has been launched, so we'll have built the window and 
            // declared viewport height, width etc by this point.

            /*
             * Controller input stuff
             */

             // Iterate over each controller and get its state.
            DWORD dwResult;

            for (DWORD controllerIndex = 0; controllerIndex < maxControllers; controllerIndex++) {

                XINPUT_STATE controllerInstance = { 0 };
                SecureZeroMemory(&controllerInstance, sizeof(XINPUT_STATE));

                // Simply get the state of the controller from XInput.
                dwResult = XInputGetState(controllerIndex, &controllerInstance);

                if (dwResult != ERROR_SUCCESS) {
                    // Controller is not connected/available.
                    continue;
                }

                // ...controller connected/available

                // Fetch the gamepad
                XINPUT_GAMEPAD *gamepad = &controllerInstance.Gamepad;

                GameControllerInput *newController = &inputNewInstance->controllers[controllerIndex];
                GameControllerInput *oldController = &inputOldInstance->controllers[controllerIndex];

                win32ProcessXInputControllerButton(&newController->up,
                    &oldController->up,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_UP);

                win32ProcessXInputControllerButton(&newController->up,
                    &oldController->up,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_DOWN);

                win32ProcessXInputControllerButton(&newController->up,
                    &oldController->up,
                    gamepad,
                    XINPUT_GAMEPAD_DPAD_LEFT);

                win32ProcessXInputControllerButton(&newController->up,
                    &oldController->up,
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

                // Normalise the axis values so they're between -1.0 and 1.0
                float32 leftThumbstickX = 0.0f;
                if (gamepad->sThumbLX > 0) {
                    leftThumbstickX = ((float32)gamepad->sThumbLX / 32767.0f);
                }
                else if (gamepad->sThumbLX < 0) {
                    leftThumbstickX = ((float32)gamepad->sThumbLX / 32768.0f);
                }

                float32 leftThumbstickY = 0.0f;
                if (gamepad->sThumbLY > 0) {
                    leftThumbstickY = ((float32)gamepad->sThumbLY / 32767.0f);
                }
                else if (gamepad->sThumbLY < 0) {
                    leftThumbstickY = ((float32)gamepad->sThumbLY / 32768.0f);
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
            // win32AudioBuffer.runningByteIndex and lockOffsetInBytes will be the same in theory.
            DWORD lockOffsetInBytes = 0;

            // Start playing sound. (Write a dummy wave sound)
            // Each single "sample" is a 16-bit value. 8 bits for the left channel, and 8 bits for the right channel.
            // They both go together.
            // Each individual sample gets output at a time, thus outputting to both the left and right channels
            // at the same time. The sound buffer (win32AudioBuffer.buffer) contains all of these 16-bit audio samples.

            // The IDirectSoundBuffer8::GetCurrentPosition method retrieves 
            // the position of the play and write cursors in the sound buffer.
            if (win32AudioBuffer.bufferSuccessfulyCreated) {

                HRESULT res;

                DWORD playCursorOffsetInBytes = NULL; // Offset, in bytes, of the play cursor
                DWORD writeCursorOffsetInBytes = NULL; // Offset, in bytes, of the write cursor (not used)

                res = win32AudioBuffer.buffer->GetCurrentPosition(&playCursorOffsetInBytes, &writeCursorOffsetInBytes);

                if (SUCCEEDED(res)) {

                    // IDirectSoundBuffer8::Lock Readies all or part of the buffer for a data 
                    // write and returns pointers to which data can be written

                    lockOffsetInBytes = (writeCursorOffsetInBytes % win32AudioBuffer.bufferSizeInBytes);

                    // Is the current lock offset ahead of the current play cursor? If yes, we'll get back 
                    // two chucks of data from IDirectSoundBuffer8::Lock, otherwise we'll only get back
                    // one chuck of data.
                    if (writeCursorOffsetInBytes > playCursorOffsetInBytes) {
                        lockSizeInBytes = ((win32AudioBuffer.bufferSizeInBytes - writeCursorOffsetInBytes) + (playCursorOffsetInBytes));
                    }
                    else if (writeCursorOffsetInBytes < playCursorOffsetInBytes) {
                        lockSizeInBytes = ((win32AudioBuffer.bufferSizeInBytes - (win32AudioBuffer.bufferSizeInBytes - playCursorOffsetInBytes)) - writeCursorOffsetInBytes);
                    }

                }
                else {
                    OutputDebugString("Could not get the position of the play and write cursors in the secondary sound buffer");
                }

            } // Audio buffer created.

            // Create the game's audio buffer
            gameInitAudioBuffer(&audioBuffer,
                                win32AudioBuffer.samplesPerSecond,
                                win32AudioBuffer.bytesPerSample,
                                win32AudioBuffer.secondsWorthOfAudio,
                                (win32AudioBuffer.bufferSizeInBytes / win32AudioBuffer.bytesPerSample),
                                win32AudioBuffer.bufferSizeInBytes,
                                0);

            // Create the game's frame buffer
            FrameBuffer frameBuffer = {};
            gameInitFrameBuffer(&frameBuffer,
                                win32FrameBuffer.height,
                                win32FrameBuffer.width,
                                win32FrameBuffer.bytesPerPixel,
                                win32FrameBuffer.byteWidthPerRow,
                                win32FrameBuffer.memory);

            // Main game code.
            gameUpdate(&memory, &frameBuffer, &audioBuffer, inputInstances, maxControllers);

            // Output the audio buffer in Windows.
            win32WriteAudioBuffer(&win32AudioBuffer, lockOffsetInBytes, lockSizeInBytes, &audioBuffer);

            // Display the frame buffer in Windows.
            win32ClientDimensions clientDimensions = win32GetClientDimensions(window);
            win32DisplayFrameBuffer(deviceHandleForWindow, win32FrameBuffer, clientDimensions.width, clientDimensions.height);

            // How long did this game loop (frame) take?

            // Processor clock cycles.
            uint64 processorClockCyclesAfterFrame = __rdtsc();
            int64 processorClockCyclesElapsed = (processorClockCyclesAfterFrame - processorClockCycles);
            float32 clockCycles_mega = ((float32)processorClockCyclesElapsed / 1000000.0f); // processorClockCyclesElapsed is in the millions, dividing by 1m to give us a "mega" (e.g. megahertz) value.

            // Performance-counter frequency for MS/frame & FPS
            LARGE_INTEGER gameLoopPerformanceCounter;
            QueryPerformanceCounter(&gameLoopPerformanceCounter);
            float32 countersElapsedPerFrame = (float32)(gameLoopPerformanceCounter.QuadPart - runningPerformanceCounter.QuadPart);
            float32 millisecondsElapsedPerFrame = (((float32)countersElapsedPerFrame * 1000.0f) / (float32)countersPerSecond);
            float32 secondsPerFrame = (1000.0f / (float32)millisecondsElapsedPerFrame);

            // Processor running speed in GHz
            float32 speed = ((uint64)(secondsPerFrame * clockCycles_mega) / 100.0f);

#if HANDMADE_LOCAL_BUILD
            // Console log the speed:
            char output[100] = {};
            sprintf_s(output, 100,
                        "ms/frame: %.1f FSP: %.1f. Cycles: %.1fm (%.2f GHz)\n", millisecondsElapsedPerFrame, secondsPerFrame, clockCycles_mega, speed);
            OutputDebugString(output);
#endif

            // Reset the running clock cycles & counters.
            processorClockCycles = processorClockCyclesAfterFrame;
            runningPerformanceCounter.QuadPart = gameLoopPerformanceCounter.QuadPart;

        } // game loop

    }else{
        OutputDebugString("Error allocating game memory. Unable to run game\n");
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
            if (DEBUG_OUTPUT) {
                OutputDebugString("\nWM_SIZE\n");
            }
        } break;

        case WM_DESTROY: {
            // @TODO(JM) Handle as an error. Recreate window?
            if (DEBUG_OUTPUT) {
                OutputDebugString("WM_DESTROY\n");
            }
            running = false;
        } break;

        // Called when the user requests to close the window.
        case WM_CLOSE: {
            // @TODO(JM) Display "are you sure" message to user?
            if (DEBUG_OUTPUT) {
                OutputDebugString("WM_CLOSE\n");
            }
            running = false;
        } break;

        case WM_QUIT: {
            if (DEBUG_OUTPUT) {
                OutputDebugString("\nWM_QUIT\n\n");
            }
            running = false;
        } break;

        // Called when the user makes the window active (e.g. by tabbing to it).
        case WM_ACTIVATEAPP: {
            if (DEBUG_OUTPUT) {
                OutputDebugString("\nWM_ACTIVATEAPP\n\n");
            }
        } break;

        // Request to paint a portion of an application's window.
        case WM_PAINT: {

            if (DEBUG_OUTPUT) {
                OutputDebugString("\nWM_PAINT\n\n");
            }

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

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {

            // Which key was pressed?
            WPARAM vkCode = wParam;

            // Was the user holding down ALT when pressing a key?
            if (message == WM_SYSKEYDOWN) {
                if (0x73 == vkCode) {
                    running = 0;
                }
            }
            /*
             * lParam bitmask. Written from right to left
             *
             *  First two bytes              Second two bytes
             * |-------------------------|  |---------------|
             * 31 30 29 28-25 24 23-16      15-0 
             * 0  0  0  0000  1  01001011   0000000000000001
            */
            
            // lParam is 4-bytes wide.
            uint32 *lParamBitmask = (uint32 *)&lParam;

            // Fetch the second two bytes (bits 0-15)
            uint16 *repeatCount = (uint16 *)&lParam;
            repeatCount = (repeatCount + 1);

            if (WM_KEYDOWN == message) {

                bool isDown = ((*lParamBitmask & (1 << 31)) == 0);
                bool wasDown = ((*lParamBitmask & (1 << 30)) != 0); // 1 if the key is held down

                if (vkCode == 'W') {

                    char output[100] = {};
                    sprintf_s(output, 100, "is down? %i\n", isDown);
                    OutputDebugString(output);

                    memset(output, 0, sizeof(output));
                    sprintf_s(output, 100, "was down? %i\n", wasDown);
                    OutputDebugString(output);

                    memset(output, 0, sizeof(output));
                    sprintf_s(output, 100, "repeat count %i\n", *repeatCount);
                    OutputDebugString(output);
                }
            }            

        } break;

        // The standard request from GetMessage().
        default: {

            //OutputDebugString("default\n");

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
    if (DEBUG_OUTPUT) {
        OutputDebugString("\nInitialising Win32 Buffer ");
        OutputDebugString("(Allocating a chunk of memory that's large enough to have 32-bits for each pixel based on height & width)\n\n");
    }

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
    if (DEBUG_OUTPUT) {
        OutputDebugString("\nCopying buffer memory to screen\n");
    }
    

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
    HMODULE libHandle = LoadLibrary("dsound.dll");

    if (!libHandle) {
        OutputDebugString("Could not load DirectSound DLL (dsound.dll)");
        return;
    }

    // Result variable for the various function call return checks.
    HRESULT res;

    DirectSoundCreateDT *DirectSoundCreateAddr = (DirectSoundCreateDT*)GetProcAddress(libHandle, "DirectSoundCreate");

    if (!DirectSoundCreateAddr) {
        // Function not found within library.
        OutputDebugString("DirectSoundCreate not in dsound.dll. Invalid/malformed DLL.");
        return;
    }

    DirectSoundCreateDT *DirectSoundCreate = DirectSoundCreateAddr;

    LPDIRECTSOUND directSound;

    res = DirectSoundCreate(NULL, &directSound, NULL);

    if (FAILED(res)){
        OutputDebugString("Could not create direct sound object");
        return;
    }

    res = directSound->SetCooperativeLevel(window, DSSCL_PRIORITY);

    if (FAILED(res)){
        OutputDebugString("Could not set cooperative level on direct sound object");
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
        OutputDebugString("Could not create primary buffer");
        return;
    }

    // Init our Win32 audio buffer...
    win32AudioBuffer->noOfChannels = 2;
    win32AudioBuffer->bitsPerChannel = 16;
    win32AudioBuffer->samplesPerSecond = 48000;
    win32AudioBuffer->bytesPerSample = ((win32AudioBuffer->bitsPerChannel * win32AudioBuffer->noOfChannels) / 8);
    win32AudioBuffer->secondsWorthOfAudio = 1;
    win32AudioBuffer->bufferSizeInBytes = (uint64)((win32AudioBuffer->bytesPerSample * win32AudioBuffer->samplesPerSecond) * win32AudioBuffer->secondsWorthOfAudio);
    win32AudioBuffer->runningByteIndex = 0;

    // Set the format
    WAVEFORMATEX waveFormat = {};

    waveFormat.wFormatTag          = WAVE_FORMAT_PCM;
    waveFormat.nChannels           = win32AudioBuffer->noOfChannels;
    waveFormat.nSamplesPerSec      = win32AudioBuffer->samplesPerSecond;
    waveFormat.nAvgBytesPerSec     = (win32AudioBuffer->samplesPerSecond * win32AudioBuffer->bytesPerSample);
    waveFormat.nBlockAlign         = win32AudioBuffer->bytesPerSample;
    waveFormat.wBitsPerSample      = win32AudioBuffer->bitsPerChannel;
    waveFormat.cbSize              = 0;

    res = primarySoundBuffer->SetFormat(&waveFormat);

    if (FAILED(res)){
        OutputDebugString("Could not set sound format on primary buffer");
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
        OutputDebugString("Could not create secondary buffer");
        return;
    }

    win32AudioBuffer->bufferSuccessfulyCreated = TRUE;

    OutputDebugString("Primary & secondary successfully buffer created\n");
}

internal_func void win32WriteAudioBuffer(Win32AudioBuffer *win32AudioBuffer,
                                            DWORD lockOffsetInBytes,
                                            DWORD lockSizeInBytes,
                                            GameAudioBuffer *audioBuffer)
{
    if (!win32AudioBuffer->bufferSuccessfulyCreated) {
        return;
    }

    // Ensure the offset and lock size are both on the correct byte boundaries and that there are bytes to write
    if (((lockOffsetInBytes % win32AudioBuffer->bytesPerSample) != 0) || ((lockSizeInBytes % win32AudioBuffer->bytesPerSample) != 0) || (lockSizeInBytes < win32AudioBuffer->bytesPerSample) ) {
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

        DWORD chunkOneStart = lockOffsetInBytes;
        DWORD chunkTwoStart = 0;

        // Calculate the total number of 4-byte audio sample groups (2-bytes/16 bits for the left channel, 2-bytes/16 bits for the right channel) 
        // that we have within the first block of memory IDirectSoundBuffer8::Lock has told us we can write to.
        uint64 audioSampleGroupsChunkOne = (chunkOneBytes / win32AudioBuffer->bytesPerSample);

        // Grab the first 16-bits of the first audio sample from the first block of memory 
        uint16 *audioSample = (uint16*)chunkOnePtr;

        // Create a pointer to the game audio buffer.
        uint16 *buffer = (uint16 *)audioBuffer->memory;

        // Advance the pointer to match the same as the lock offset.
        // Divide by 2 as buffer is 2 bytes, whereas lockOffsetInBytes is a single byte.
        buffer = (buffer + (lockOffsetInBytes / 2));

        // Iterate over each 2 - bytes and write the same data for both...
        for (size_t i = 0; i < audioSampleGroupsChunkOne; i++) {

            // Left channel (16-bits)
            *audioSample = *buffer;

            // Move to the right sample (16-bits)
            audioSample++;
            buffer++;

            // Right channel (16-bits)
            *audioSample = *buffer;

            // Move cursor to the start of the next sample grouping.
            audioSample++;
            buffer++;
        }

        // Calculate how many samples we need to write to in our second block of memory.
        uint64 audioSampleGroupsChunkTwo = (chunkTwoBytes / win32AudioBuffer->bytesPerSample);

        // Grab the first 16-bit of the first audio sample from the second block of memory 
        uint16 *audioTwoSample = (uint16*)chunkTwoPtr;

        // Set the audio buffer pointer back to the start of the memory block as the
        // second block of memory always starts from the beginning
        buffer = (uint16 *)audioBuffer->memory;

        for (size_t i = 0; i < audioSampleGroupsChunkTwo; i++) {

            // Left channel (16-bits)
            *audioTwoSample = *buffer;

            // Move to the right sample (16-bits)
            audioTwoSample++;
            buffer++;

            // Right channel (16-bits)
            *audioTwoSample = *buffer;

            // Move cursor to the start of the next sample grouping.
            audioTwoSample++;
            buffer++;
        }

        res = win32AudioBuffer->buffer->Unlock(chunkOnePtr, chunkOneBytes, chunkTwoPtr, chunkTwoBytes);

        if (FAILED(res)) {
            OutputDebugString("Could not unlock sound buffer");
        }

    }
    else {
        OutputDebugString("Could not lock secondary sound buffer");
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

internal_func DWORD WINAPI XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE *pState) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

internal_func DWORD WINAPI XInputSetStateStub(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

internal_func void loadXInputDLLFunctions(void)
{
    HMODULE libHandle = LoadLibrary("XInput1_4.dll");

    // No XInput 1.4? Try and load the older 9.1.0.
    if (!libHandle) {
        libHandle = LoadLibrary("XInput9_1_0.dll");
    }

    // No XInput 9.1.0? Try and load the older 1.3.
    if (!libHandle) {
        libHandle = LoadLibrary("XInput1_3.dll");
    }

    if (libHandle) {

        XInputGetStateDT *XInputGetStateAddr = (XInputGetStateDT*)GetProcAddress(libHandle, "XInputGetState");
        XInputSetStateDT *XInputSetStateAddr = (XInputSetStateDT*)GetProcAddress(libHandle, "XInputSetState");

        if (XInputGetStateAddr) {
            XInputGetState = XInputGetStateAddr;
        }
        if (XInputSetStateAddr) {
            XInputSetState = XInputSetStateAddr;
        }
    }
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

/**
 * Vibrate the controller. 0 = 0% motor usage, 65,535 = 100% motor usage.
 * The left motor is the low-frequency rumble motor. The right motor is the
 * high-frequency rumble motor.
 *
 */
internal_func void platformControllerVibrate(uint8 controllerIndex, uint16 motor1Speed, uint16 motor2Speed)
{
    XINPUT_VIBRATION pVibration = {0};

    pVibration.wLeftMotorSpeed = motor1Speed;
    pVibration.wLeftMotorSpeed = motor2Speed;

    XInputSetState(controllerIndex, &pVibration);
}