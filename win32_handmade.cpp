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
	// Create a new window struct.
	WNDCLASS WindowClass = {};

	// Define the window's attributes. @see https://msdn.microsoft.com/en-us/library/windows/desktop/ff729176(v=vs.85).aspx
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;

	// Callback to handle any messages sent to the window (resize etc).
	WindowClass.lpfnWndProc = mainWindowCallback; 

	// Instance of the running application.
	WindowClass.hInstance = instance;
	WindowClass.lpszClassName = "handmadeHeroWindowClass";

	// Registers the window class for subsequent use in calls to 
	// the CreateWindowEx function.
	if (RegisterClass(&WindowClass)){

		HWND WindowHandle = CreateWindowEx(NULL,
											WindowClass.lpszClassName,
											"Handmade Hero",
											WS_TILEDWINDOW|WS_VISIBLE,
											CW_USEDEFAULT,
		 									CW_USEDEFAULT,
											CW_USEDEFAULT,
											CW_USEDEFAULT,
		   									NULL,
											NULL,
											instance,
											NULL);

		if (WindowHandle){

			MSG Message;

			int getMessageRes;

			while( (getMessageRes = GetMessage(&Message, WindowHandle, 0, 0)) != 0){ 
			    if (getMessageRes == -1){
			        // handle the error and exit
			        break;
			    }else{
			        TranslateMessage(&Message);
			        DispatchMessage(&Message);
			    }
			}


		}else{
			// TODO(JM) Log error.
			OutputDebugString("Error 1. WindowHandle not created\n");
		}

	}else{
		// TODO(JM) Log error.
		OutputDebugString("Error 2. WindowClass not registered\n");
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

	case WM_SIZE:
	{
		OutputDebugString("WM_SIZE\n");
	} break;

	case WM_DESTROY:
	{
		OutputDebugString("WM_DESTROY\n");
	} break;

	case WM_CLOSE:
	{
		OutputDebugString("WM_CLOSE\n");
	} break;

	case WM_ACTIVATEAPP:
	{
		OutputDebugString("WM_ACTIVATEAPP\n");
	} break;

	default:
	{
		result = DefWindowProc(window, message, wParam, lParam);
	} break;
	}

	return result;
}