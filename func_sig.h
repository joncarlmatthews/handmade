#ifndef HEADER_FILE
#define HEADER_FILE

LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

internal_func void win32InitBuffer(win32OffScreenBuffer *buffer, uint32_t width, uint32_t height);

internal_func void win32CopyBufferToWindow(HDC deviceHandleForWindow, win32OffScreenBuffer buffer, uint32_t width, uint32_t height);

internal_func void writeToBufferBitmap(win32OffScreenBuffer buffer, int redOffset, int greenOffset);

internal_func win32ClientDimensions win32GetClientDimensions(HWND window);

#endif