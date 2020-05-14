
// Windows API.
#include <windows.h>

// Typedefs that specify exact-width integer types for increased code portability.
/*
 * char:        (1)     int8_t  / uint8_t   (-128 127)          (0 255)
 * short:       (2)     int16_t / uint16_t  (-32,768 32,767)    (0 65,536)
 * int (long):  (4)     int32_t / uint32_t  (-2.1bn to 2.1bn)   (0 to 4.2bn)
 * long long:   (8)     int64_t / uint64_t  (-9qn 9qn)          (0-18qn)
 */
#include <stdint.h>
#include <math.h> // For Sin
#include <strsafe.h> // For StringCbVPrintfA & STRSAFE_MAX_CCH
#include <stdarg.h> // For variable number of arguments in function sigs
#include <dsound.h> // Direct Sound for audio output.
#include <xinput.h> // Xinput for receiving controller input. 
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#define global_var          static // Global variables
#define local_persist_var   static // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static // Functions that are only available within the file they're declared in
#define PIf                 3.14159265359f

#define LOG_LEVEL_INFO      0x100
#define LOG_LEVEL_WARN      0x200
#define LOG_LEVEL_ERROR     0x300

typedef uint32_t    bool32; // For 0 or "> 0 I don't care" booleans
typedef float       float32;
typedef double      float64;

// I know this wont change, but it's to help me read the code, instead of seeing
// things multiplied by what might seem like an arbitrary 8 all over the place.
const int BITS_PER_BYTE = 8;

// Display output debug strings?
const bool DEBUG_OUTPUT = FALSE;

/**
 * Struct for the Win32 screen buffer
 */
struct win32FrameBuffer 
{
    // The width in pixels of the buffer.
    uint32_t width;

    // The height in pixels of the buffer.
    uint32_t height;

    // 1 byte each for R, G & B and 1 byte for padding to match byte boundries (4)
    // Therefore our pixels are always 32-bits wide and are in Little Endian 
    // memory order (backwards) 0xBBGGRRPP
    uint16_t bytesPerPixel;

    // The number of bytes per row. (width * bytesPerPixel)
    uint32_t byteWidthPerRow;

    // Bitmap parameters
    BITMAPINFO info;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;
};

/**
 * Helper struct for the Win32 screen dimensions. @see win32GetClientDimensions
 */
struct win32ClientDimensions 
{
    uint32_t width;
    uint32_t height;
};

/**
 * Struct for the Win32 secondary sound buffer.
 */
struct win32AudioBuffer
{
    // Flag for whether or not the buffer has been successfully initiated.
    bool bufferSuccessfulyCreated;

    // How many audio channels are we targetting?
    uint8_t noOfChannels;

    // How many bits per individual left or right sample? 
    // (16-bits for the left channel 16-bits for the right channel.)
    uint8_t bitsPerSample;

    // How many bytes to store per sample? (sum of left + right channel)
    uint8_t bytesPerSample;

    // How many samples per second will we be storing?
    uint16_t samplesPerSecond;

    // How many seconds worth of the audio should our buffer hold?
    uint8_t secondsWorthOfAudio;

    // The block of memory to hold the samples.
    LPDIRECTSOUNDBUFFER buffer;

    // Byte count of our buffer's memory
    uint64_t bufferSizeInBytes;

    // Last position within the buffer that we wrote to.
    DWORD runningByteIndex;
};

// Whether or not the application is running
global_var bool running;

// Win32 frame buffer.
global_var win32FrameBuffer frameBuffer;

// Win32 audio buffer
global_var win32AudioBuffer audioBuffer;

// Function signatures
#include "func_sig.h"

// XInput support
typedef DWORD WINAPI XInputGetStateDT(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);
typedef DWORD WINAPI XInputSetStateDT(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);

global_var XInputGetStateDT *XInputGetState_ = XInputGetStateStub;
global_var XInputSetStateDT *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

// Direct sound support
typedef HRESULT WINAPI DirectSoundCreateDT(LPGUID lpGuid, LPDIRECTSOUND* ppDS, LPUNKNOWN  pUnkOuter);

/*
 * The entry point for this graphical Windows-based application.
 * 
 * @param HINSTANCE A handle to the current instance of the application.
 * @param HINSTANCE A handle to the previous instance of the application.
 * @param LPSTR Command line arguments sent to the application.
 * @param int How the user has specified the window to be shown
 */
internal_func int CALLBACK WinMain(HINSTANCE instance,
                                    HINSTANCE prevInstance, 
                                    LPSTR commandLine, 
                                    int showCode)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // Load XInput DLL functions.
    loadXInputDLLFunctions();

    // Create our back buffer.
    win32InitFrameBuffer(&frameBuffer, 1024, 768);

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

    // Graphics stuff
    uint32_t redOffset = 0;
    uint32_t greenOffset = 0;

    // Audio stuff...
    win32InitDirectSound(window);
    win32WriteAudioBuffer(0, audioBuffer.bufferSizeInBytes, 200);
    audioBuffer.buffer->Play(0, 0, DSBPLAY_LOOPING);

    running = TRUE;

    while (running) {

        MSG message;

        // Message loop. Retrieves all messages (from the calling thread's message queue)
        // that are sent to the window. E.g. clicks and key inputs.
        while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {

            if (message.message == WM_QUIT) {
                running = false;
            }

            // Get the message ready for despatch.
            TranslateMessage(&message);

            // Dispatch the message to the application's window procedure.
            // @link win32MainWindowCallback
            DispatchMessage(&message);

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
        for (DWORD controllerIndex = 0; controllerIndex < XUSER_MAX_COUNT; controllerIndex++) {

            XINPUT_STATE controllerState;
            SecureZeroMemory(&controllerState, sizeof(XINPUT_STATE));

            // Simply get the state of the controller from XInput.
            dwResult = XInputGetState(controllerIndex, &controllerState);

            if (dwResult != ERROR_SUCCESS) {
                // Controller is not connected/available.
                continue;
            }

            // ...controller is connected

            // Fetch the pad
            XINPUT_GAMEPAD *pad = &controllerState.Gamepad;

            // Set booleans for the individual button states
            bool btnUpDepressed             = (pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
            bool btnDownDepressed           = (pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            bool btnLeftDepressed           = (pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            bool btnRightDepressed          = (pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
            bool btnStartDepressed          = (pad->wButtons & XINPUT_GAMEPAD_START);
            bool btnBackDepressed           = (pad->wButtons & XINPUT_GAMEPAD_BACK);
            bool btnShoulderLeftDepressed   = (pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
            bool btnShoulderRightDepressed  = (pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
            bool btnADepressed              = (pad->wButtons & XINPUT_GAMEPAD_A);
            bool btnBDepressed              = (pad->wButtons & XINPUT_GAMEPAD_B);
            bool btnCDepressed              = (pad->wButtons & XINPUT_GAMEPAD_X);
            bool btnDDepressed              = (pad->wButtons & XINPUT_GAMEPAD_Y);

            int16_t leftThumbstickX = pad->sThumbLX;
            int16_t leftThumbstickY = pad->sThumbLY;

            // Animate the screen
            redOffset = (redOffset + (leftThumbstickY >> 12));            
            greenOffset = (greenOffset - (leftThumbstickX >> 12));

            // Vibrate the controller
            XINPUT_VIBRATION pVibration;

            if ( (leftThumbstickX != 0) || (leftThumbstickY != 0)) {
                pVibration.wLeftMotorSpeed = 10000;
                pVibration.wRightMotorSpeed = 10000;
               
            }else {
                pVibration.wLeftMotorSpeed = 0;
                pVibration.wRightMotorSpeed = 0;
            }

            XInputSetState(controllerIndex, &pVibration);

        } // controller loop

        win32WriteFrameBuffer(frameBuffer, redOffset, greenOffset);

        win32ClientDimensions clientDimensions = win32GetClientDimensions(window);
        win32DisplayFrameBuffer(deviceHandleForWindow, frameBuffer, clientDimensions.width, clientDimensions.height);

        /*
         * Audio stuff
         */

        // Start playing sound. (Write a dummy wave sound)
        // Each single "sample" is a 16-bit value. 8 bits for the left channel, and 8 bits for the right channel.
        // They both go together.
        // Each individual sample gets output at a time, thus outputting to both the left and right channels
        // at the same time. The sound buffer (audioBuffer.buffer) contains all of these 16-bit audio samples.

        // The IDirectSoundBuffer8::GetCurrentPosition method retrieves 
        // the position of the play and write cursors in the sound buffer.
        if (audioBuffer.bufferSuccessfulyCreated) {
         
            HRESULT res;

            DWORD playCursorOffsetInBytes = NULL; // Offset, in bytes, of the play cursor
            DWORD writeCursorOffsetInBytes = NULL; // Offset, in bytes, of the write cursor (not used)

            res = audioBuffer.buffer->GetCurrentPosition(&playCursorOffsetInBytes, &writeCursorOffsetInBytes);

            if (SUCCEEDED(res)) {

                // IDirectSoundBuffer8::Lock Readies all or part of the buffer for a data 
                // write and returns pointers to which data can be written

                // Offset, in bytes, from the start of the buffer to the point where the lock begins.
                // We Mod the result by the total number of bytes so that the value wraps.
                // Result will look like this: 0, 4, 8, 12, 16, 24
                DWORD lockOffsetInBytes = (audioBuffer.runningByteIndex % audioBuffer.bufferSizeInBytes);

                // Size, in bytes, of the portion of the buffer to lock.
                DWORD lockSizeInBytes;

                // Is the current lock offset ahead of the current play cursor? If yes, we'll get back 
                // two chucks of data from IDirectSoundBuffer8::Lock, otherwise we'll only get back
                // one chuck of data.
                if (lockOffsetInBytes > playCursorOffsetInBytes) {

                    // Gap to the end of the buffer, plus the start of the buffer up to the current play cursor
                    lockSizeInBytes = (audioBuffer.bufferSizeInBytes - lockOffsetInBytes) + (0 + playCursorOffsetInBytes);

                }
                else {
                    // Gap from the current lock offset up to the current play cursor
                    lockSizeInBytes = (playCursorOffsetInBytes - lockOffsetInBytes);
                }

                win32WriteAudioBuffer(lockOffsetInBytes, lockSizeInBytes, 200);

            }
            else {
                log(LOG_LEVEL_ERROR, "Could not get the position of the play and write cursors in the secondary sound buffer");
                log(LOG_LEVEL_ERROR, "");

            }

        }

    } // running

    CoUninitialize();

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

            win32DisplayFrameBuffer(deviceHandleForWindow, frameBuffer, clientDimensions.width, clientDimensions.height);

            // End the paint request and releases the device context.
            EndPaint(window, &paint);
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {

            // Which key was pressed?
            uint32_t vkCode = wParam;

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
            uint32_t *lParamBitmask = (uint32_t *)&lParam;

            // Fetch the second two bytes (bits 0-15)
            uint16_t *repeatCount = (uint16_t *)&lParam;
            repeatCount = (repeatCount + 1);

            if (WM_KEYDOWN == message) {

                bool isDown = ((*lParamBitmask & (1 << 31)) == 0);
                bool wasDown = ((*lParamBitmask & (1 << 30)) != 0); // 1 if the key is held down

                if (vkCode == 'W') {
                    debug("is down? %i\n", isDown);
                    debug("was down? %i\n", wasDown);
                    debug("repeat count: %i\n", *repeatCount);
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
internal_func void win32InitFrameBuffer(win32FrameBuffer *buffer, uint32_t width, uint32_t height)
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
    buffer->info.bmiHeader.biBitCount       = (buffer->bytesPerPixel * BITS_PER_BYTE); // 32-bits per pixel
    buffer->info.bmiHeader.biCompression    = BI_RGB;

    // How many bytes do we need for our bitmap?
    // viewport width * viewport height = viewport area
    // then viewport area * how many bytes we need per pixel.
    uint32_t bitmapMemorySizeInBytes = ((buffer->width * buffer->height) * buffer->bytesPerPixel);

    // Now allocate the memory using VirtualAlloc to the size of the previously
    // calculated bitmapMemorySizeInBytes
    buffer->memory = VirtualAlloc(NULL, bitmapMemorySizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    // Calculate the width in bytes per row.
    buffer->byteWidthPerRow = (buffer->width * buffer->bytesPerPixel);
}

/*
 * Writes bits to the buffer's memory. Sets the colour values for the individual 
 * pixels (which are each 32 bits of the memory block).
 *
 * Once all the memory has been written to the buffer is ready to be drawn on
 * screen
 */
internal_func void win32WriteFrameBuffer(win32FrameBuffer buffer, int redOffset, int greenOffset)
{
    if (DEBUG_OUTPUT) {
        OutputDebugString("\nWriting to Buffer's memory\n");
    }

    // Create a pointer to bitmapMemory
    // In order for us to have maximum control over the pointer arithmatic, we cast it to
    // an 1 byte datatype. This enables us to step through the memory block 1 byte
    // at a time.
    uint8_t *row = (uint8_t *)buffer.memory;

    // Create a loop that iterates for the same number of rows we have for the viewport. 
    // (We know the number of pixel rows from the viewport height)
    // We name the iterator x to denote the x axis (along the corridor)
    for (uint32_t x = 0; x < buffer.height; x++) {

        // We know that each pixel is 4 bytes wide (bytesPerPixel) so we make
        // our pointer the same width to grab the relevant block of memory for
        // each pixel. (32 bits = 4 bytes)
        uint32_t *pixel = (uint32_t *)row;

        // Create a loop that iterates for the same number of columns we have for the viewport.
        // (We know the number of pixel columns from the viewport width)
        // We name the iterator y to denote the y axis (up the stairs)
        for (uint32_t y = 0; y < buffer.width; y++) {

            /*
             * Write to this pixel...
             *
             * Each pixel looks like this (in hex): 00 00 00 00
             * Each of the 00 represents 1 of our 4-byte pixels.
             *
             * As the order of bytes is little endian, the RGB bytes are backwards
             * when writting to them:
             *
             * B    G   R   Padding
             * 00   00  00  00
            */

            uint8_t red = (uint8_t)(x + redOffset);  // Chop off anything after the first 8 bits of the variable x + offset
            uint8_t green = (uint8_t)(y + greenOffset);  // Chop off anything after the first 8 bits of the variable y + offset
            uint8_t blue = 0;
            *pixel = ((red << 16) | (green << 8) | blue);

            // Move the pointer forward to the start of the next 4 byte block
            pixel = (pixel + 1);
        }

        // Move the row pointer forward by the byte width of the row so that for
        // the next iteration of the row we're then starting at the first byte
        // of that particular row
        row = (row + buffer.byteWidthPerRow);
    }
}

/*
 * Function for handling WM_PAINT message.
 *
 * Copies a win32FrameBuffer's memory to the actual window - which will then
 * display its contents to the screen.
 *
 *
 * @param deviceHandleForWindow     The window handle
 * @param win32FrameBuffer      The buffer
 * @param width                        The window's viewport width
 * @param height                    The window's viewport height
 */
internal_func void win32DisplayFrameBuffer(HDC deviceHandleForWindow, 
                                            win32FrameBuffer buffer,
                                            uint32_t width,
                                            uint32_t height)
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

internal_func win32ClientDimensions win32GetClientDimensions(HWND window)
{
    RECT clientRect;
    GetClientRect(window, &clientRect);

    win32ClientDimensions dim;

    dim.width = clientRect.right;
    dim.height  = clientRect.bottom;

    return dim;
}

internal_func DWORD WINAPI XInputGetStateStub(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

internal_func DWORD WINAPI XInputSetStateStub(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration) {
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

        XInputGetStateDT* XInputGetStateAddr = (XInputGetStateDT*)GetProcAddress(libHandle, "XInputGetState");
        XInputSetStateDT* XInputSetStateAddr = (XInputSetStateDT*)GetProcAddress(libHandle, "XInputSetState");

        if (XInputGetStateAddr) {
            XInputGetState = XInputGetStateAddr;
        }
        if (XInputSetStateAddr) {
            XInputSetState = XInputSetStateAddr;
        }
    }
}

internal_func void win32InitDirectSound(HWND window)
{
    audioBuffer.bufferSuccessfulyCreated = FALSE;

    // Load the library
    HMODULE libHandle = LoadLibrary("dsound.dll");

    if (!libHandle) {
        log(LOG_LEVEL_WARN, "Could not load DirectSound DLL (dsound.dll)");
        return;
    }

    // Create the struct    
    audioBuffer.noOfChannels = 2;
    audioBuffer.bitsPerSample = 16;
    audioBuffer.samplesPerSecond = 48000;
    audioBuffer.bytesPerSample = ((audioBuffer.bitsPerSample * audioBuffer.noOfChannels) / BITS_PER_BYTE);
    audioBuffer.secondsWorthOfAudio = 1;
    audioBuffer.bufferSizeInBytes = ((audioBuffer.bytesPerSample * audioBuffer.samplesPerSecond) * audioBuffer.secondsWorthOfAudio);
    audioBuffer.runningByteIndex = 0;

    // Block alignment, in bytes. Must process a multiple of blockAlignment 
    // bytes of data at a time. Data written to and read from a device 
    // must always start at the beginning of a block
    uint8_t blockAlignment = audioBuffer.bytesPerSample;

    // Result variable for the various function call return checks.
    HRESULT res;

    DirectSoundCreateDT* DirectSoundCreateAddr = (DirectSoundCreateDT*)GetProcAddress(libHandle, "DirectSoundCreate");

    if (!DirectSoundCreateAddr) {
        // Function not found within library.
        log(LOG_LEVEL_WARN, "DirectSoundCreate not in dsound.dll. Malformed DLL?");
        return;
    }

    DirectSoundCreateDT* DirectSoundCreate = DirectSoundCreateAddr;

    LPDIRECTSOUND directSound;

    res = DirectSoundCreate(NULL, &directSound, NULL);

    if (FAILED(res)){
        log(LOG_LEVEL_ERROR, "Could not create direct sound object");
        return;
    }

    res = directSound->SetCooperativeLevel(window, DSSCL_PRIORITY);

    if (FAILED(res)){
        log(LOG_LEVEL_ERROR, "Could not set cooperative level on direct sound object");
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
        log(LOG_LEVEL_ERROR, "Could not create primary buffer");
        return;
    }

    // Set the format
    WAVEFORMATEX waveFormat = {};

    waveFormat.wFormatTag          = WAVE_FORMAT_PCM;
    waveFormat.nChannels           = audioBuffer.noOfChannels;
    waveFormat.nSamplesPerSec      = audioBuffer.samplesPerSecond;
    waveFormat.nAvgBytesPerSec     = (audioBuffer.samplesPerSecond * blockAlignment);
    waveFormat.nBlockAlign         = blockAlignment;
    waveFormat.wBitsPerSample      = audioBuffer.bitsPerSample;
    waveFormat.cbSize              = 0;

    res = primarySoundBuffer->SetFormat(&waveFormat);

    if (FAILED(res)){
        log(LOG_LEVEL_ERROR, "Could not set sound format on primary buffer");
        return;
    }

    // Create the secondary buffer. This is the buffer we'll use to actually
    // write bytes to.
    DSBUFFERDESC secondarySoundBufferDesc;
    ZeroMemory(&secondarySoundBufferDesc, sizeof(DSBUFFERDESC));

    secondarySoundBufferDesc.dwSize              = sizeof(secondarySoundBufferDesc);
    secondarySoundBufferDesc.dwFlags             = 0;
    secondarySoundBufferDesc.dwBufferBytes       = audioBuffer.bufferSizeInBytes;
    secondarySoundBufferDesc.lpwfxFormat         = &waveFormat;
    secondarySoundBufferDesc.guid3DAlgorithm     = GUID_NULL;

    res = directSound->CreateSoundBuffer(&secondarySoundBufferDesc, &audioBuffer.buffer, NULL);

    if (FAILED(res)){
        log(LOG_LEVEL_ERROR, "Could not create secondary buffer");
        return;
    }

    audioBuffer.bufferSuccessfulyCreated = TRUE;
    debug("Primary & secondary successfully buffer created\n");
}

bool win32WriteAudioBuffer(DWORD lockOffsetInBytes, DWORD lockSizeInBytes, uint32_t cyclesPerSecond)
{
    if (!audioBuffer.bufferSuccessfulyCreated) {
        return false;
    }

    // Ensure the offset and lock size are both on the correct byte boundaries
    if (((lockOffsetInBytes % audioBuffer.bytesPerSample) != 0) || ((lockSizeInBytes % audioBuffer.bytesPerSample) != 0)) {
        return false;
    }

    HRESULT res;

    bool doAudioWrite;

    if (lockSizeInBytes <= 0) {
        doAudioWrite = false;
        //log(LOG_LEVEL_INFO, "Skipping audio write, block size is 0");
        //log(LOG_LEVEL_INFO, "Lock offset in bytes: %i Lock size in bytes: %i, play cursor position: %i", lockOffsetInBytes, lockSizeInBytes, playCursorOffsetInBytes);
        //log(LOG_LEVEL_INFO, "");

    }
    else {
        doAudioWrite = true;
    }

    if (doAudioWrite) {

        void *chunkOnePtr; // Receives a pointer to the first locked part of the buffer.
        DWORD chunkOneBytes; // Receives the number of bytes in the block at chunkOnePtr

        void *chunkTwoPtr; // Receives a pointer to the second locked part of the buffer.
        DWORD chunkTwoBytes; // Receives the number of bytes in the block at chunkTwoPtr

        res = audioBuffer.buffer->Lock(lockOffsetInBytes,
            lockSizeInBytes,
            &chunkOnePtr,
            &chunkOneBytes,
            &chunkTwoPtr,
            &chunkTwoBytes,
            0);

        if (SUCCEEDED(res)) {

            if (cyclesPerSecond < 40) {
                cyclesPerSecond = 40;
            }else if(cyclesPerSecond > 1000) {
                cyclesPerSecond = 1000;
            }

            // Size of the wave? Larger wave = louder
            int16_t sizeOfWave = 4000;

            // Calculate the total number of 4-byte audio sample groups (16 for the left, 16 for the right) 
            // that we have within the first block of memory IDirectSoundBuffer8::Lock has 
            // told us we can write to.
            uint64_t audioSampleGroupsChunkOne = (chunkOneBytes / audioBuffer.bytesPerSample);

            // Calculate the total number of 4-byte audio sample groups that we will have per complete cycle.
            uint16_t audioSampleGroupsPerCycle = (audioBuffer.bufferSizeInBytes / cyclesPerSecond);

            // Calculate the total number of 4-byte audio sample groups per cycle quarter.
            uint16_t audioSampleGroupsPerCycleQuarter = (audioSampleGroupsPerCycle / 4);

            // At the start of which 4 byte group index we are starting our write from?
            // @TODO(JM) assert that this is a 4 byte boundry
            uint64_t byteGroupIndex = lockOffsetInBytes;

            // Grab the first 16-bit audio sample from the first block of memory 
            uint16_t *audioSample = (uint16_t*)chunkOnePtr;

            // Audio sample value
            int16_t audioSampleValue = 0;

            float percentageOfAngle = 0.0f;
            float angle = 0.0f;
            float radians = 0.0f;
            float sine = 0.0f;
            
            // Iterate over each 2 - bytes and write the same data for both...
            for (size_t i = 0; i < audioSampleGroupsChunkOne; i++) {

                percentageOfAngle = percentageOfAnotherf((float)byteGroupIndex, audioSampleGroupsPerCycle);
                angle = (360.0f * (percentageOfAngle / 100.0f));
                radians = (angle * (PIf / 180.0f));
                sine = sinf(radians);

                audioSampleValue = (int16_t)(sine * sizeOfWave);

                // Left (16-bits)
                *audioSample = audioSampleValue;

                // Move to the right sample (16-bits)
                audioSample++;

                // Write the right sample
                *audioSample = audioSampleValue;

                // Move cursor to the start of the next sample grouping.
                audioSample++;

                byteGroupIndex = (byteGroupIndex + audioBuffer.bytesPerSample);
            }

            uint64_t audioSampleGroupsChunkTwo = (chunkTwoBytes / audioBuffer.bytesPerSample);

            // Grab the first 16-bit audio sample from the first block of memory 
            uint16_t *audioTwoSample = (uint16_t*)chunkTwoPtr;

            for (size_t i = 0; i < audioSampleGroupsChunkTwo; i++) {

                percentageOfAngle = percentageOfAnotherf((float)byteGroupIndex, audioSampleGroupsPerCycle);
                angle = (360.0f * (percentageOfAngle / 100.0f));
                radians = (angle * (PIf / 180.0f));
                sine = sinf(radians);

                audioSampleValue = (int16_t)(sine * sizeOfWave);

                // Left (16-bits)
                *audioTwoSample = audioSampleValue;

                // Move to the right sample (16-bits)
                audioTwoSample++;

                // Write the right sample
                *audioTwoSample = audioSampleValue;

                // Move cursor to the start of the next sample grouping.
                audioTwoSample++;

                byteGroupIndex = (byteGroupIndex + audioBuffer.bytesPerSample);
            }

            audioBuffer.runningByteIndex = byteGroupIndex;

            res = audioBuffer.buffer->Unlock(chunkOnePtr, chunkOneBytes, chunkTwoPtr, chunkTwoBytes);

            if (FAILED(res)) {
                //log(LOG_LEVEL_ERROR, "Could not unlock sound buffer");
                //log(LOG_LEVEL_ERROR, "");
            }

        }
        else {
            //log(LOG_LEVEL_ERROR, "Could not lock secondary sound buffer: ");
            //log(LOG_LEVEL_ERROR, "");
        }

    } // doWrite

    return true;
}

/*
 * Utility function for outputting a debug string that takes parameters
 * in the same form as the C's native printf. 1000 characters max
 *
 * Required: windows.h, stdint.h, strsafe.h, stdarg.h
 *
 * @author Jon Matthews
 *
 * @param char * format
 * @param mixed values
 */
internal_func void debug(char *format, ...)
{
    va_list argList;

    // @see STRSAFE_MAX_CCH
    const uint16_t bufSze = (100 * sizeof(TCHAR));

    char msgBuf[bufSze];

    va_start(argList, format);
    HRESULT hr = StringCbVPrintfA(msgBuf, bufSze, format, argList);
    va_end(argList);

    if (hr == S_OK) {
        OutputDebugString(msgBuf);
    }else{
        OutputDebugString("Error creating debug string\n");
    }
}

/*
 * Utility function for logging a debug string that takes parameters
 * in the same form as the C's native printf. 1000 characters max
 *
 * Required: windows.h, stdint.h, strsafe.h, stdarg.h
 *
 * @author Jon Matthews
 *
 * @param char *level
 * @param char *format
 * @param mixed values
 */
internal_func void log(int level, char *format, ...)
{
    va_list argList;

    // @see STRSAFE_MAX_CCH
    const uint16_t bufSze = (100 * sizeof(TCHAR));

    char msgBuf[bufSze];

    va_start(argList, format);
    HRESULT hr = StringCbVPrintfA(msgBuf, bufSze, format, argList);
    va_end(argList);

    // @TOD(JM) Log msgBuf to file...
    if (hr == S_OK) {

        switch (level) {
            default:
            case LOG_LEVEL_INFO:
                OutputDebugString("Info: ");
                break;
            case LOG_LEVEL_WARN:
                OutputDebugString("Warning: ");
                break;
            case LOG_LEVEL_ERROR:
                OutputDebugString("Error: ");
                break;
        }

        OutputDebugString(msgBuf);
        OutputDebugString("\n");
    }
    else {
        OutputDebugString("LOG: Error creating debug string\n");
    }
}

float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b == 0) {
        return 0;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}
