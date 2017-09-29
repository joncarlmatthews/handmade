#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK mainWindowCallback(HWND Window, 
									UINT Message, 
									WPARAM wParam, 
									LPARAM lParam)
{
	LRESULT result = 0;
	switch(Message){

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
			OutputDebugString("MESSAGE: ");
			//char msgbuf;
			//sprintf(msgbuf, "%i", Message);
			//OutputDebugString(msgbuf);
			OutputDebugString("\n");

		} break;
	}

	return result;
}

int CALLBACK WinMain(HINSTANCE Instance, 
						HINSTANCE PrevInstance, 
						LPSTR commandLine, 
						int showCode)
{
	OutputDebugString("Error 0\n");

	WNDCLASS WindowClass = {};
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = mainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "handmadeHeroWindowClass";

	if (RegisterClass(&WindowClass)){

		HWND WindowHandle = CreateWindowEx(
			NULL,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_TILEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
		 	CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
		   	NULL,
			NULL,
			Instance,
			NULL
		);

		if (WindowHandle){

			MSG Message;

			int getMessageRes;

			while( (getMessageRes = GetMessage(&Message, WindowHandle, 0, 0)) != 0)
			{ 
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
			OutputDebugString("Error 1\n");
		}

	}else{
		// TODO(JM) Log error.
		OutputDebugString("Error 1\n");
	}

	return(0);
}