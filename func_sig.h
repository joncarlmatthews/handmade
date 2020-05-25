#ifndef HEADER_FILE
#define HEADER_FILE

internal_func void debug(char *format, ...);

internal_func void log(int level, char* format, ...);

float32 percentageOfAnotherf(float32 a, float32 b);

internal_func LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

internal_func void win32InitFrameBuffer(win32FrameBuffer *buffer, uint32_t width, uint32_t height);

internal_func void win32WriteFrameBuffer(win32FrameBuffer buffer, int redOffset, int greenOffset);

internal_func void win32DisplayFrameBuffer(HDC deviceHandleForWindow, win32FrameBuffer buffer, uint32_t width, uint32_t height);

internal_func win32ClientDimensions win32GetClientDimensions(HWND window);

internal_func DWORD WINAPI XInputGetStateStub(_In_  DWORD dwUserIndex, _Out_ XINPUT_STATE* pState);

internal_func DWORD WINAPI XInputSetStateStub(_In_ DWORD dwUserIndex, _In_ XINPUT_VIBRATION* pVibration);

internal_func void loadXInputDLLFunctions(void);

internal_func void win32InitDirectSound(HWND window);

internal_func bool win32WriteAudioBuffer(DWORD lockOffsetInBytes, DWORD lockSizeInBytes, uint32_t cyclesPerSecond);

#endif