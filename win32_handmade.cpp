#include <windows.h>

#define global_var			static; // Global variables
#define local_persist_var	static; // Static variables within a local scope (e.g. case statement, function)
#define internal_func		static; // Functions that are only available within the file they're declared in

// Whether or not the application is running
global_var bool running;

// Bitmap info for creating when creating the DIB.
global_var BITMAPINFO bitmapInfo;

// Bitmap memory for when creating the DIB.
global_var void *bitmapMemory;

/*
 * Callback method for WNDCLASS struct. Processes messages sent to the window.
 */
LRESULT CALLBACK win32MainWindowCallback(HWND window,
											UINT message,
											WPARAM wParam,
											LPARAM lParam);

/*
 * A Device Independent Bitmap (DIB) is what Windows calls things
 * that we can write into which it can then display to the screen
 * using it's internal Graphics Device Interface (GDI).
 *
 * This method will create a new DIB, or resize it if its already been created.
 * 
 * Called each time the message WM_SIZE is sent to the message callback handler.
 *
 * @param width		The client viewport width
 * @param height	The client viewport height
 */
internal_func void win32ResizeDeviceIndependentBitmapSeciton(long width, long height)
{
	// Does the bitmapMemory already exist from a previous WM_SIZE call?
	if (bitmapMemory != NULL) {

		// Yes, then free the memorty allocated.
		// We do this because we have to redraw it as this method
		// is called on a window resize.
		VirtualFree(bitmapMemory, NULL, MEM_RELEASE);
	}

	bitmapInfo.bmiHeader.biSize = sizeof(bitmapInfo.bmiHeader);
	bitmapInfo.bmiHeader.biWidth = width;
	bitmapInfo.bmiHeader.biHeight = height;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biBitCount = 32;
	bitmapInfo.bmiHeader.biCompression = BI_RGB;

	// How many bytes do we need per pixel? Our pixels
	// will be rendered using RBG colours. Therefore we need
	// 1 byte for R, 1 byte for G and 1 byte for B. 
	// and 1 extra byte for padding so we align with
	// bitmapInfo.bmiHeader.biBitCount
	int bytesPerPixel = 4;

	// How many bytes do we need for our bitmap?
	// viewport width * viewport height = viewport area
	// then viewport area * how many bytes we need per pixel.
	int bitmapMemorySize = ((width * height) * bytesPerPixel);

	bitmapMemory = VirtualAlloc(NULL, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

/*
* Updates the client's viewport using the DIB created in win32ResizeDeviceIndependentBitmapSeciton().
*
* @param window		The window handle
* @param x			The client viewport top left position
* @param y			The client viewport bottom right position
* @param width		The client viewport width
* @param height		The client viewport height
*/
internal_func void win32UpdateViewport(HDC deviceHandleForWindow, long x, long y, long width, long height)
{
	// StretchDIBits function copies the data of a rectangle of pixels to the specified destination.
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

	// Callback to handle any messages sent to the window (resize etc).
	windowClass.lpfnWndProc = win32MainWindowCallback;

	// Instance of the running application.
	windowClass.hInstance = instance;
	windowClass.lpszClassName = "handmadeHeroWindowClass";

	// Registers the window class for subsequent use in calls to 
	// the CreateWindowEx function.
	if (RegisterClass(&windowClass)){

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

		if (windowHandle){

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

		}else{
			// TODO(JM) Log error.
			OutputDebugString("Error 1. windowHandle not created via CreateWindowEx\n");
		}

	}else{
		// TODO(JM) Log error.
		OutputDebugString("Error 2. windowClass not registered\n");
	}

	// Close the application.
	return(0);
}

LRESULT CALLBACK win32MainWindowCallback(HWND window,
											UINT message,
											WPARAM wParam,
											LPARAM lParam)
{
	LRESULT result = 0;

	switch (message) {

		// Sent after the window's size has changed.
		case WM_SIZE: {
			OutputDebugString("WM_SIZE\n");

			RECT clientRect;

			GetClientRect(window, &clientRect);

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
			OutputDebugString("WM_PAINT\n");

			PAINTSTRUCT paint;

			// Prepare the window for painting and get the device context.
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
			// The default window procedure to provide default processing for any window messages not explicitly handled.
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}

