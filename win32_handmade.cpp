#include <windows.h>
#include <stdint.h>

#define global_var          static; // Global variables
#define local_persist_var   static; // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static; // Functions that are only available within the file they're declared in

// I know this wont change, but it's to help me read the code, instead of seeing
//things multiplied by 8 all over the place.
global_var int bitsPerByte = 8;

// Whether or not the application is running
global_var bool running;

// Bitmap info for creating when creating the DIB.
global_var BITMAPINFO bitmapInfo;

// Bitmap memory for when creating the DIB.
global_var void *bitmapMemory;

// Function signatures
LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
internal_func void win32ResizeDeviceIndependentBitmapSeciton(long width, long height);
internal_func void win32UpdateViewport(HDC deviceHandleForWindow, long x, long y, long width, long height);

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
	HWND windowHandle = CreateWindowEx(NULL,
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

	if (!windowHandle) {

		// TODO(JM) Log error.
		OutputDebugString("Error 2. windowHandle not created via CreateWindowEx\n");
		return FALSE;
	}

	running = true;

	while (running) {

		MSG message;

		BOOL getMessageRes;

		// Message loop. Retrieves all messages (from the calling thread's message queue)
		// that are sent to the window. E.g. clicks and key inputs.
		getMessageRes = GetMessage(&message, windowHandle, 0, 0);
		if (getMessageRes > 0) {

			// Get the message ready for despatch.
			TranslateMessage(&message);

			// Dispatch the message to the application's window procedure.
			// @link win32MainWindowCallback
			DispatchMessage(&message);
						
		}else {

			// handle the error and exit
			OutputDebugString("Error\n");
			break;
						
		}

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
			OutputDebugString("WM_SIZE!\n");

            // Window resized. Get the new window's viewport dimensions.
			// We can do this by calling GetClientRect. RECT.right and RECT.bottom
            // are effectively width and height.
			RECT clientRect;
			GetClientRect(window, &clientRect);

            // Call our function for actually handling the window resize.
			win32ResizeDeviceIndependentBitmapSeciton(clientRect.right, clientRect.bottom);

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

		// Called when the user makes the window active (e.g. by tabbing to it).
		case WM_ACTIVATEAPP: {
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		// Request to paint a portion of an application's window.
		case WM_PAINT: {

			OutputDebugString("WM_PAINT!\n");

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
internal_func void win32ResizeDeviceIndependentBitmapSeciton(long viewportWidth, long viewportHeight)
{
	// Does the bitmapMemory already exist from a previous WM_SIZE call?
	if (bitmapMemory != NULL) {

		// Yes, then free the memorty allocated.
		// We do this because we have to redraw it as this method
		// (win32ResizeDeviceIndependentBitmapSeciton) is called on a window resize.
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}

	// How many bytes do we need per pixel? Our pixels will be rendered using
	// RBG colours. Therefore we need 1 byte for R, 1 byte for G and 1 byte for B (3)
	// However you should always align your bytes in alignment with the byte 
	// boundaries. E.g. 4, 8, 16, 32 etc. Therefore will will add 1 extra byte 
	// for padding. Apparently there is a penalty of some sort for not doing so. 
	// (Day 004)
	int bytesPerPixel = 4;

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

    // Now actually draw to the window.

    // First cast the bitmapMemory from a void * so that C treats it as a pointer to just
    // bytes in memory. To do this cast it as an 8-bit unsigned integer (aka unsigned char)
    uint8_t *row = (uint8_t *)bitmapMemory;

    //int pitch = (viewportWidth * bytesPerPixel);

    OutputDebugString("Viewport height: ");

    // Loop over each row. (Pixel row, from left to right)
    for (int i = 0; i < viewportHeight; i++) {

        //OutputDebugString("Row: %i \n", i);
        //wsprintf("Row: %i \n", *i);
        OutputDebugString("Row: ");
        OutputDebugString((char *)i);
        OutputDebugString("\n");

        // For each row, loop over each pixel in the row
        for (int i = 0; i < viewportWidth; i++){

        }
    }
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

