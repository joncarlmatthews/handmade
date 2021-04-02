#ifndef HEADER_FILE
#define HEADER_FILE

/**
 * Struct for the Win32 screen buffer
 */
typedef struct Win32FrameBuffer
{
    // The width in pixels of the buffer.
    uint32 width;

    // The height in pixels of the buffer.
    uint32 height;

    // 1 byte each for R, G & B and 1 byte for padding to match byte boundries (4)
    // Therefore our pixels are always 32-bits wide and are in Little Endian 
    // memory order (backwards) 0xBBGGRRPP
    uint16 bytesPerPixel;

    // The number of bytes per row. (width * bytesPerPixel)
    uint32 byteWidthPerRow;

    // Bitmap parameters
    BITMAPINFO info;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;
} Win32FrameBuffer;

/**
 * Helper struct for the Win32 screen dimensions. @see win32GetClientDimensions
 */
typedef struct win32ClientDimensions
{
    uint32 width;
    uint32 height;
} win32ClientDimensions;

/**
 * Struct for the Win32 secondary sound buffer.
 */
typedef struct Win32AudioBuffer
{
    // Flag for whether or not the buffer has been successfully initiated.
    bool bufferSuccessfulyCreated;

    // How many audio channels are we targetting?
    // 2 a left channel and a right channel
    uint8 noOfChannels;

    // How many bits per individual left or right channel? 
    // (16-bits for the left channel's sample, 16-bits for the right channel's sample.)
    uint8 bitsPerChannel;

    // How many bytes to store per "sample"? A single sample is the grouping of one left + right channel.
    // The sum of left + right channel is 32-bits, therefore each sample needs 4 bytes
    // Data written to and read from a the buffer must always start at the beginning of a 16-bit, 4-byte sample "block"
    uint8 bytesPerSample;

    // How many samples per second will we be storing?
    uint16 samplesPerSecond;

    // How many seconds worth of the audio should our buffer hold?
    uint8 secondsWorthOfAudio;

    // The block of memory to hold the samples.
    LPDIRECTSOUNDBUFFER buffer;

    // Byte count of our buffer's memory
    uint32 bufferSizeInBytes;

    // Last position within the buffer that we wrote to.
    uint32 runningByteIndex;

} Win32AudioBuffer;

internal_func void debug(char *format, ...);

internal_func void log(int level, char *format, ...);

float32 percentageOfAnotherf(float32 a, float32 b);

internal_func LRESULT CALLBACK win32MainWindowCallback(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

internal_func void win32InitFrameBuffer(Win32FrameBuffer *buffer, uint32 width, int32 height);

internal_func void win32DisplayFrameBuffer(HDC deviceHandleForWindow, Win32FrameBuffer buffer, uint32 width, uint32 height);

internal_func win32ClientDimensions win32GetClientDimensions(HWND window);

internal_func DWORD WINAPI XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE *pState);

internal_func DWORD WINAPI XInputSetStateStub(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

internal_func void loadXInputDLLFunctions(void);

internal_func void win32InitAudioBuffer(HWND window, Win32AudioBuffer *win32AudioBuffer);

internal_func void win32WriteAudioBuffer(Win32AudioBuffer *win32AudioBuffer, SineWave *sineWave, DWORD lockOffsetInBytes, DWORD lockSizeInBytes);

#endif