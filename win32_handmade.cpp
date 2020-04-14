// Windows API.
#include <windows.h>

// Typedefs that specify exact-width integer types for increased code portability.
/*
 * char:        (1)     int8_t  / uint8_t   (-128 127)          (0 255)
 * short:       (2)     int16_t / uint16_t  (-32,768 32,767)    (0 65,536)
 * int & long:  (4)     int32_t / uint32_t  (-2.1bn to 2.1bn)   (0 to 4.2bn)
 * long long:   (8)     int64_t / uint64_t  (-9qn 9qn)          (0-18qn)
 */
#include <stdint.h>
#include <strsafe.h> // For STRSAFE_MAX_CCH
#include <stdarg.h> // For variable number of arguments in function sigs

// Xinput for receiving controller input. 
#include <xinput.h>

#define global_var          static; // Global variables
#define local_persist_var   static; // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static; // Functions that are only available within the file they're declared in

// I know this wont change, but it's to help me read the code, instead of seeing
// things multiplied by what might seem like an arbitrary 8 all over the place.
const int BITS_PER_BYTE = 8;

// Display output debug strings?
const bool DEBUG_OUTPUT = FALSE;

/**
 * Struct for the Win32 screen buffer
 *
 */
struct win32OffScreenBuffer 
{
    // The width in pixels of the buffer.
    uint32_t width;

    // The height in pixels of the buffer.
    uint32_t height;

    // 1 byte each for R, G & B and 1 byte for padding to match byte boundries (4)
    // Therefore our pixels are always 32-bits wide and are in Little Endian memory order (backwards) 0xBBGGRRPP
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
 *
 */
struct win32ClientDimensions 
{
    uint32_t width;
    uint32_t height;
};

// Whether or not the application is running
global_var bool running;

// Create the Win32 off screen buffer.
global_var win32OffScreenBuffer backBuffer;

// Function signatures
#include "func_sig.h"

// XInput support
typedef DWORD WINAPI XInputGetStateDT(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);
typedef DWORD WINAPI XInputSetStateDT(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);

global_var XInputGetStateDT *XInputGetState_ = XInputGetStateStub;
global_var XInputSetStateDT *XInputSetState_ = XInputSetStateStub;

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

void loadXInputDLLFunctions(void)
{
    HMODULE libHandle = LoadLibrary("XInput1_4.dll");

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

/*
 * The entry point for this graphical Windows-based application.
 * 
 * @param HINSTANCE A handle to the current instance of the application.
 * @param HINSTANCE A handle to the previous instance of the application.
 * @param LPSTR Command line arguments sent to the application.
 * @param int How the user has specified the window to be shown
 */
int CALLBACK WinMain(HINSTANCE instance, 
                        HINSTANCE prevInstance, 
                        LPSTR commandLine, 
                        int showCode)
{
    // Load XInput DLL functions.
    loadXInputDLLFunctions();

    // Create our back buffer.
    win32InitBuffer(&backBuffer, 1024, 768);

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

    running = true;

    int redOffset = 0;
    int greenOffset = 0;

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

            if (btnUpDepressed) {

                // Animate the screen
                redOffset = (redOffset++);
                greenOffset = (greenOffset++);

                // Vibrate the controller
                XINPUT_VIBRATION pVibration;

                pVibration.wLeftMotorSpeed = 65535;
                pVibration.wRightMotorSpeed = 65535;

                XInputSetState(controllerIndex, &pVibration);
            }
        }

        win32WriteBitsToBufferMemory(backBuffer, redOffset, greenOffset);

        win32ClientDimensions clientDimensions = win32GetClientDimensions(window);

        win32CopyBufferToWindow(deviceHandleForWindow, backBuffer, clientDimensions.width, clientDimensions.height);

    } // running

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
LRESULT CALLBACK win32MainWindowCallback(HWND window,
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

            win32CopyBufferToWindow(deviceHandleForWindow, backBuffer, clientDimensions.width, clientDimensions.height);

            // End the paint request and releases the device context.
            EndPaint(window, &paint);
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN: 
        case WM_SYSKEYUP: {

            uint32_t vkCode = wParam;

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
 * @param win32OffScreenBuffer  *buffer     A pointer to the Win32 off screen buffer
 * @param int                   width       The width of the window's viewport
 * @param int                   height      The height of the window's viewport
 * 
 */
internal_func void win32InitBuffer(win32OffScreenBuffer *buffer, uint32_t width, uint32_t height)
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
        // (win32InitBuffer) is called on a window resize.
        VirtualFree(buffer->memory, NULL, MEM_RELEASE);
    }

    buffer->bytesPerPixel   = 4;
    buffer->width           = width;
    buffer->height          = height;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = width;
    buffer->info.bmiHeader.biHeight = -height; // If negative, it's drawn top down. If positive, it's drawn bottom up.
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = (buffer->bytesPerPixel * BITS_PER_BYTE);
    buffer->info.bmiHeader.biCompression = BI_RGB;

    // How many bytes do we need for our bitmap?
    // viewport width * viewport height = viewport area
    // then viewport area * how many bytes we need per pixel.
    int bitmapMemorySizeInBytes = ((buffer->width * buffer->height) * buffer->bytesPerPixel);

    // Now allocate the memory using VirtualAlloc to the size of the previously
    // calculated bitmapMemorySizeInBytes
    buffer->memory = VirtualAlloc(NULL, bitmapMemorySizeInBytes, MEM_COMMIT, PAGE_READWRITE);

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
internal_func void win32WriteBitsToBufferMemory(win32OffScreenBuffer buffer, int redOffset, int greenOffset)
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
    for (int x = 0; x < buffer.height; x++) {

        // We know that each pixel is 4 bytes wide (bytesPerPixel) so we make
        // our pointer the same width to grab the relevant block of memory for
        // each pixel. (32 bits = 4 bytes)
        uint32_t *pixel = (uint32_t *)row;

        // Create a loop that iterates for the same number of columns we have for the viewport.
        // (We know the number of pixel columns from the viewport width)
        // We name the iterator y to denote the y axis (up the stairs)
        for (int y = 0; y < buffer.width; y++) {

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

            // @JM(Note) Method one. Setting a pointer at the start of the pixel
            // and then moving it into the coorect place in memory before the 
            // write. (Three writes to memory)
            /*
            uint8_t *r = (uint8_t *)pixel;
            r = (r + 2);
            *r = 32;

            uint8_t *g = (uint8_t *)pixel;
            g = (g + 1);
            *g = 70;

            uint8_t *b = (uint8_t *)pixel;
            *b = 166;

            uint8_t *padding = (uint8_t *)pixel;
            padding = (padding + 3);
            *padding = 0;
            */

            // @JM(Note) Method two. Bit shifting the individual 1 byte variables
            // into the pixel location, whilst bitwise oring them into the pixel
            // to maintain pixel's previous values. (Three writes to memory)
            /*
            uint8_t red = 51;
            uint8_t green = 193;
            uint8_t blue = 67;
            *pixel = (red << 16 | *pixel);
            *pixel = (green << 8 | *pixel);
            *pixel = (blue | *pixel);
            */

            // @JM(Note) Method three. Bit shifting and bitwise oring the 
            // individual 1 byte variables into the themselves, then setting
            // that to the pixel variable's value. (One write to memory)
            /*
            uint8_t red     = 0xcf;
            uint8_t green   = 0x4a;
            uint8_t blue    = 0xb8;
            *pixel = ((red << 16) | (green << 8) | blue);
            */

            uint8_t red = (uint8_t)(x + redOffset);  // Grab the first 8 bits of the variable x and add offset 1
            uint8_t green = (uint8_t)(y + greenOffset);  // Grab the first 8 bits of the variable i and add offset 2
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
 * Copies a win32OffScreenBuffer's memory to the actual window - which will then
 * display its contents to the screen.
 *
 *
 * @param deviceHandleForWindow     The window handle
 * @param win32OffScreenBuffer      The buffer
 * @param width                        The window's viewport width
 * @param height                    The window's viewport height
 */
internal_func void win32CopyBufferToWindow(HDC deviceHandleForWindow, 
                                            win32OffScreenBuffer buffer,
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

DWORD WINAPI XInputGetStateStub(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE* pState) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

DWORD WINAPI XInputSetStateStub(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration) {
    return ERROR_DEVICE_NOT_CONNECTED;
}

/*
 * Utility function for outputting a debug string that takes parameters
 * in the same form as the C's native printf.
 *
 * Required: windows.h, stdint.h, strsafe.h, stdarg.h
 *
 * @author Jon Matthews
 *
 * @param char * format
 * @param mixed values
 */
void debug(char *format, ...)
{
    va_list argList;

    const uint16_t bufSze = (STRSAFE_MAX_CCH * sizeof(TCHAR));

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