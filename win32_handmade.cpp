// Windows API.
#include <windows.h>

// Typedefs that specify exact-width integer types for increased code portability.
#include <stdint.h>

/*
 * char:        (1)     int8_t  / uint8_t
 * short:       (2)     int16_t / uint16_t
 * int:         (4)     int32_t / uint32_t
 * long long:   (8)     int64_t / uint64_t
 */

#define global_var          static; // Global variables
#define local_persist_var   static; // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static; // Functions that are only available within the file they're declared in

// I know this wont change, but it's to help me read the code, instead of seeing
// things multiplied by 8 all over the place.
global_var int bitsPerByte = 8;

// Whether or not the application is running
global_var bool running;

// The width in pixels of the applications viewport.
global_var long viewportWidth;

// The height in pixels of the applications viewport.
global_var long viewportHeight;

// How many bytes do we need per pixel? Our pixels will be writeToBitmaped using
// RBG colours. Therefore we need 1 byte for R, 1 byte for G and 1 byte for B (3)
// However you should always align your bytes in alignment with the byte 
// boundaries. E.g. 4, 8, 16, 32 etc. Therefore will will add 1 extra byte 
// for padding. Apparently there is a penalty of some sort for not doing so. 
// (Day 004)
global_var int bytesPerPixel = 4;

// Bitmap info for creating when creating the DIB.
global_var BITMAPINFO bitmapInfo;

// Bitmap memory for when creating the DIB.
global_var void *bitmapMemory;

// Function signatures
LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
internal_func void win32ResizeDeviceIndependentBitmapSeciton();
internal_func void win32UpdateViewport(HDC deviceHandleForWindow, long x, long y, long width, long height);
internal_func void writeToBitmap(int viewportWidth, int viewportHeight, int bytesPerPixel, int redOffset, int greenOffset);

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
	// Create a new window struct and set all of it's values to 0.
	WNDCLASS windowClass = {};

	// Define the window's attributes. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
	windowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;

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

        // After processing our messages, we can now (in our while running = true
        // loop) do what we like! WM_SIZE and WM_PAINT get called as soon as the
        // application has been launched, so we'll have built the window and 
        // declared viewport height, width etc by this point.
        writeToBitmap(viewportWidth, viewportHeight, bytesPerPixel, redOffset, greenOffset);

        HDC deviceHandleForWindow = GetDC(window);

        RECT clientRect;
        GetClientRect(window, &clientRect);

        long width = clientRect.right;
        long height = clientRect.bottom;

        win32UpdateViewport(deviceHandleForWindow, 0, 0, width, height);
        ReleaseDC(window, deviceHandleForWindow);

        redOffset = (redOffset++);
        greenOffset = (greenOffset++);

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

		// Sent after the window's size has changed (window resize).
		case WM_SIZE: {
			OutputDebugString("WM_SIZE\n");

            // Window resized. Get the new window's viewport dimensions.
			// We can do this by calling GetClientRect. RECT.right and RECT.bottom
            // are effectively width and height.
			RECT clientRect;
			GetClientRect(window, &clientRect);

            viewportWidth   = clientRect.right;
            viewportHeight  = clientRect.bottom;

            // Call our function for actually handling the window resize.
			win32ResizeDeviceIndependentBitmapSeciton();

		} break;

		case WM_DESTROY: {
			// @TODO(JM) Handle as an error. Recreate window?
			OutputDebugString("WM_DESTROY\n");
			running = false;
		} break;

		// Called when the user requests to close the window.
		case WM_CLOSE: {
			// @TODO(JM) Display "are you sure" message to user?
			OutputDebugString("WM_CLOSE\n");
			running = false;
		} break;

        case WM_QUIT: {
            OutputDebugString("WM_QUIT\n");
            running = false;
        } break;

		// Called when the user makes the window active (e.g. by tabbing to it).
		case WM_ACTIVATEAPP: {
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		// Request to paint a portion of an application's window.
		case WM_PAINT: {

			OutputDebugString("WM_PAINT\n");

			// Prepare the window for painting and get the device context.
            PAINTSTRUCT paint;
			HDC deviceHandleForWindow = BeginPaint(window, &paint);

			// Grab the x and y coords and the width and height from the paint struct
			// written to by BeginPaint
			long x = paint.rcPaint.left;
			long y = paint.rcPaint.top;
			long width = paint.rcPaint.right;
			long height = paint.rcPaint.bottom;

			win32UpdateViewport(deviceHandleForWindow, x, y, width, height);

			// End the paint request and releases the device context.
			EndPaint(window, &paint);

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
 * Function for handling WM_SIZE message.
 *
 * This method will create a new DIB, or resize it if its already been created.
 * during a previous call to WM_SIZE.
 *
 * A Device Independent Bitmap (DIB) is what Windows calls things
 * that we can write into, which it can then display to the screen
 * using it's internal Graphics Device Interface (GDI).
 *
 *
 * @param width		The client viewport width
 * @param height	The client viewport height
 */
internal_func void win32ResizeDeviceIndependentBitmapSeciton()
{
	// Does the bitmapMemory already exist from a previous WM_SIZE call?
	if (bitmapMemory != NULL) {

		// Yes, then free the memorty allocated.
		// We do this because we have to redraw it as this method
		// (win32ResizeDeviceIndependentBitmapSeciton) is called on a window resize.
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = viewportWidth;
	bitmapInfo.bmiHeader.biHeight = -viewportHeight; // If negative, it's drawn top down. If positive it's drawn bottom up.
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = (bytesPerPixel * bitsPerByte);
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	// How many bytes do we need for our bitmap?
	// viewport width * viewport height = viewport area
	// then viewport area * how many bytes we need per pixel.
	int bitmapMemorySize = ((viewportWidth * viewportHeight) * bytesPerPixel);

    // Now allocate the memory using VirtualAlloc to the size of the previously
    // calculated bitmapMemorySize
	bitmapMemory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE); 
}

/*
 * Function for handling WM_PAINT message.
 *
 * Paints to the client's viewport using the DIB created in win32ResizeDeviceIndependentBitmapSeciton().
 * 
 * @NOTE(JM) does this need to be it's own function? Currently just one line.
 *
 * @param window		The window handle
 * @param x			The client viewport top left position
 * @param y			The client viewport bottom right position
 * @param width		The client viewport width
 * @param height		The client viewport height
 */
internal_func void win32UpdateViewport(HDC deviceHandleForWindow, 
                                        long x, 
                                        long y, 
                                        long width, 
                                        long height)
{
	// StretchDIBits function copies the data of a rectangle of pixels to 
	// the specified destination.
	StretchDIBits(deviceHandleForWindow,
                    x,
                    y,
                    width,
                    height,
                    x,
                    y,
                    width,
                    height,
                    bitmapMemory,
                    &bitmapInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);
}

internal_func void writeToBitmap(int viewportWidth, 
                                    int viewportHeight, 
                                    int bytesPerPixel,
                                    int redOffset,
                                    int greenOffset) {

    // Calculate the width in bytes per row.
    int byteWidthPerRow = (viewportWidth * bytesPerPixel);

    // Create a pointer to bitmapMemory
    // In order for us to have maximum control over the pointer arithmatic, we cast it to
    // an 1 byte datatype. This enables us to step through the memory block 1 byte
    // at a time.
    uint8_t *row = (uint8_t *)bitmapMemory;

    // Create a loop that iterates for the same number of rows we have for the viewport. 
    // (We know the number of pixel rows from the viewport height)
    // We name the iterator x to denote the x axis (along the corridor)
    for (int x = 0; x < viewportHeight; x++) {

        // We know that each pixel is 4 bytes wide (bytesPerPixel) so we make
        // our pointer the same width to grab the relevant block of memory for
        // each pixel. (32 bits = 4 bytes)
        uint32_t *pixel = (uint32_t *)row;

        // Create a loop that iterates for the same number of columns we have for the viewport.
        // (We know the number of pixel columns from the viewport width)
        // We name the iterator y to denote the y axis (up the stairs)
        for (int y = 0; y < viewportWidth; y++) {

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
        row = (row + byteWidthPerRow);
    }
}

