#ifndef HEADER_FILE
#define HEADER_FILE

#define LOG_LEVEL_INFO      0x100
#define LOG_LEVEL_WARN      0x200
#define LOG_LEVEL_ERROR     0x300

internal_func void debug(char *format, ...);

internal_func void log(uint8_t level, char* format, ...);

internal_func LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

internal_func void win32InitBuffer(win32OffScreenBuffer *buffer, uint32_t width, uint32_t height);

internal_func void win32CopyBufferToWindow(HDC deviceHandleForWindow, win32OffScreenBuffer buffer, uint32_t width, uint32_t height);

internal_func void win32WriteBitsToBufferMemory(win32OffScreenBuffer buffer, int redOffset, int greenOffset);

internal_func win32ClientDimensions win32GetClientDimensions(HWND window);

internal_func DWORD WINAPI XInputGetStateStub(_In_  DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);

internal_func DWORD WINAPI XInputSetStateStub(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);

internal_func void loadXInputDLLFunctions(void);

internal_func void win32InitDirectSound(HWND window);

#endif