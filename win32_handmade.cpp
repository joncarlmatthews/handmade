#include <windows.h>

#define internal_func		static;
#define local_persist_var	static;
#define global_var			static;

global_var bool running;

/*
 *  Callback method for WNDCLASS struct. Processes messages sent to the window.
 */
LRESULT CALLBACK mainWindowCallback(HWND window,
									UINT message,
									WPARAM wParam,
									LPARAM lParam);

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
	windowClass.lpfnWndProc = mainWindowCallback; 

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

				char getMessageRes;

				// Message loop. Retrieves all messages (from the calling thread's message queue)
				// that are sent to the window. E.g. clicks and key inputs.
				// Keeps looping until message received is WM_QUIT.
				while ((getMessageRes = GetMessage(&message, windowHandle, 0, 0)) != 0) {
					if (getMessageRes == -1) {
						// handle the error and exit
						OutputDebugString("Error\n");
						break;
					}else {

						// Get the message ready for despatch.
						TranslateMessage(&message);

						// Dispatch the message to the application's window procedure.
						// @link mainWindowCallback
						DispatchMessage(&message);
					}

				} // GetMessages

			} // running

		}else{
			// TODO(JM) Log error.
			OutputDebugString("Error 1. windowHandle not created via CreateWindowEx\n");
		}

	}else{
		// TODO(JM) Log error.
		OutputDebugString("Error 2. windowClass not registered\n");
	}

	return(0);
}

LRESULT CALLBACK mainWindowCallback(HWND window,
									UINT message,
									WPARAM wParam,
									LPARAM lParam)
{
	LRESULT result = 0;
	switch (message) {

		// Sent after the window's size has changed.
		case WM_SIZE: {
			OutputDebugString("WM_SIZE\n");
		} break;

		case WM_DESTROY: {
			// @TODO(JM) Handle as an error. Recreate window?
			OutputDebugString("WM_DESTROY\n");
			running = false;
		} break;

		case WM_CLOSE: {
			// @TODO(JM) Display "are you sure" message to user?
			OutputDebugString("WM_CLOSE\n");
			running = false;
		} break;

		case WM_ACTIVATEAPP: {
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		// Request to paint a portion of an application's window.
		case WM_PAINT: {
			OutputDebugString("WM_PAINT\n");

			PAINTSTRUCT paint;

			// Prepare the window for painting and get the device context.
			HDC deviceHandle = BeginPaint(window, &paint);

			// Grab the x and y coords and the width and height from the paint struct
			// written to by BeginPaint
			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right;
			int height = paint.rcPaint.bottom;

			// Call PatBlt to paint a black rectangle on within the window
			PatBlt(deviceHandle, x, y, width, height, BLACKNESS);

			// End the paint request and releases the device context.
			EndPaint(window, &paint);

		} break;

		default: {
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}