#include <windows.h>

/*
typedef struct tagWNDCLASS {
  UINT      style;
  WNDPROC   lpfnWndProc;
  int       cbClsExtra;
  int       cbWndExtra;
  HINSTANCE hInstance;
  HICON     hIcon;
  HCURSOR   hCursor;
  HBRUSH    hbrBackground;
  LPCTSTR   lpszMenuName;
  LPCTSTR   lpszClassName;
} WNDCLASS, *PWNDCLASS;

WindowProc(
  _In_ HWND   hwnd,
  _In_ UINT   uMsg,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
);
*/
/*
LRESULT CALLBACK mainWindowCallback(Window, Message, wParam, lParam)
{
	LRESULT result = 0;
	switch(Message){

		case WM_SIZE:
		{
			OutputDebugString("WM_SIZE");
		} break;

		case WM_DESTROY:
		{
			OutputDebugString("WM_SIZE");
		} break;

		case WM_CLOSE:
		{
			OutputDebugString("WM_SIZE");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_SIZE");
		} break;

		default:
		{
			OutputDebugString("default");
			result = 
		} break;
	}

	return result;
}
*/

int CALLBACK WinMain(HINSTANCE Instance, 
						HINSTANCE PrevInstance, 
						LPSTR commandLine, 
						int showCode)
{
	WNDCLASS WindowClass = {};
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	//WindowClass.lpfnWndProc = ;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "handmadeHeroWindowClass";

	return(0);
}