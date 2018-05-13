#include <windows.h>

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
 * @param in tHow the user has specified the window to be shown
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

			MSG message;

			char getMessageRes;

			// Message loop. Retrieves all messages (from the calling thread's message queue)
			// that are sent to the window. E.g. clicks and key inputs.
			// Keeps looping until message received is WM_QUIT.
			while( (getMessageRes = GetMessage(&message, windowHandle, 0, 0)) != 0){
			    if (getMessageRes == -1){
			        // handle the error and exit
			        break;
			    }else{
			        TranslateMessage(&message);
			        DispatchMessage(&message);
			    }
			}


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

		case WM_SIZE: {
			OutputDebugString("WM_SIZE\n");
		} break;

		case WM_DESTROY: {
			OutputDebugString("WM_DESTROY\n");
		} break;

		case WM_CLOSE: {
			OutputDebugString("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP: {
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT: {
			OutputDebugString("WM_PAINT\n");
			PAINTSTRUCT paint;
			HDC device = BeginPaint(window, &paint);
			EndPaint(window, &paint);
		} break;

		default: {
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}