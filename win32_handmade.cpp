#include <windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, 
						HINSTANCE hPrevInstance, 
						LPSTR lpCmdLine, 
						int nCmdShow)
{
	MessageBox(NULL, "This is Handmade Hero", "Handmade Hero", MB_OK);
	return(0);
}