#ifndef HEADER_WIN32
#define HEADER_WIN32

#ifdef _DEBUG
#define _DEBUG_FPS
//#define _DEBUG_CLOCKCYCLES
//#define _DEBUG_LIVE_LOOP_EDITING
#endif

#define TARGET_FPS 60
#define CAP_FPS false
#define _ASSERT_FPS true

// Custom message ID for sending to the window when we consider
// the app ready.
#define WM_HANDMADE_HERO_READY (WM_APP + 1)

// @NOTE(JM) Putting these here for reference.
// The following useful types are defined in the Win32 API
// https://learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types
/*
typedef unsigned short      WORD;   (uint16)
typedef unsigned long       DWORD;  (uint32)
typedef unsigned long long  QWORD;  (uint64)
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef char                CHAR;
typedef unsigned char       UCHAR;
typedef short               SHORT;
typedef unsigned short      USHORT;
typedef int                 INT;
typedef unsigned int        UINT;
typedef long                LONG;   (int32)
typedef unsigned long       ULONG;
*/

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
    // memory order 0xPPRRGGBB
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

} Win32AudioBuffer;

typedef struct Win32FixedFrameRate {

    // Monitor refresh rate in Hertz.
    uint8 monitorRefreshRate;

    // Target FPS.
    uint8 gameTargetFPS;

    // Target FPS in milliseconds.
    float32 gameTargetMSPerFrame;

    // System's minimum timer resolution.
    UINT timeOutIntervalMS;

    // Boolean did the system set the minimum timer resolution successfully.
    MMRESULT timeOutIntervalSet;

} Win32FixedFrameRate;

typedef struct Win32GameInputRecording
{
    uint8 count;
    GameInput *gameInput;
} Win32GameInputRecording;

typedef struct Win32State
{
    wchar_t absPath[MAX_PATH];

    uint64 gameMemorySize;
    void *gameMemory;

    HWND *window;

    xyuint monitorDims;
    xyuint monitorAspectRatio;

#if HANDMADE_LOCAL_BUILD
    void *gameMemoryRecordedState;
    void *gameMemoryRecordedInput;
    uint64 recordingWriteFrameIndex;
    uint64 recordingReadFrameIndex;
    bool8 inputRecording;
    bool8 inputPlayback;
#endif

} Win32State;

internal
LRESULT CALLBACK win32MainWindowCallback(HWND window,
                                            UINT message,
                                            WPARAM wParam,
                                            LPARAM lParam);

/*
 * Creates and returns the current time elapsed in MS since program execution
 * started via QueryPerformanceCounter
 */
internal LARGE_INTEGER win32GetTime();

/*
 * Calculates the time elapsed (milliseconds) between time a (startCounter) and time b (endCounter)
 * relative to a counters per second value
 */
internal float32 win32GetElapsedTimeMS(const LARGE_INTEGER startCounter, const LARGE_INTEGER endCounter, int64 countersPerSecond);

/*
 * Calculates the time elapsed (in seconds) between time a (startCounter) and time b (endCounter)
 * relative to a counters per second value
 */
internal float32 win32GetElapsedTimeS(const LARGE_INTEGER startCounter, const LARGE_INTEGER endCounter, int64 countersPerSecond);

internal void win32InitFrameBuffer(PlatformThreadContext *thread, Win32FrameBuffer *buffer, uint32 width, int32 height);

/*
 * @param deviceHandleForWindow     The window handle
 * @param Win32FrameBuffer          The game's filled frame buffer
 * @param clientWindowWidth         The window's width
 * @param clientWindowHeight        The window's height
 */
internal
void win32DisplayFrameBuffer(HDC deviceHandleForWindow,
                                Win32FrameBuffer buffer,
                                uint32 clientWindowWidth,
                                uint32 clientWindowHeight);

internal win32ClientDimensions win32GetClientDimensions(HWND window);

internal DWORD WINAPI XInputGetStateStub(DWORD dwUserIndex, XINPUT_STATE *pState);

internal DWORD WINAPI XInputSetStateStub(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration);

internal void win32LoadXInputDLLFunctions(void);

/**
 * @brief Loads game code from DLL
 * 
 * @param absPath Absolute path to the folder that contains game's main executeable
 * @param gameCode 
 * @return void
*/
internal void win32LoadGameDLLFunctions(wchar_t *absPath, GameCode *gameCode);
internal void win32GetAbsolutePath(wchar_t *path);

internal void win32InitAudioBuffer(HWND window, Win32AudioBuffer *win32AudioBuffer);

internal void win32AudioBufferTogglePlay(Win32AudioBuffer *win32AudioBuffer);
internal void win32AudioBufferToggleStop(Win32AudioBuffer *win32AudioBuffer);

internal void win32WriteAudioBuffer(Win32AudioBuffer *win32AudioBuffer,
                                            DWORD lockOffsetInBytes,
                                            DWORD lockSizeInBytes,
                                            GameAudioBuffer *audioBuffer);

internal void win32ProcessXInputControllerButton(GameControllerBtnState *currentState,
                                                        XINPUT_GAMEPAD *gamepad,
                                                        uint16 gamepadButtonBit);


internal void win32ProcessMessages(HWND window, GameInput *gameInput, GameInput oldGameInput, Win32State *win32State);

/*
 * Truncates 8-bytes (uint64) to 4-bytes (uint32). If in debug mode,
 * the code will assert if the value passed in is larger than 4 bytes
 */
internal uint32 win32TruncateToUint32Safe(uint64 value);

internal FILETIME win32GetFileLastWriteDate(const wchar_t *filename);

internal void win32GetMousePosition(HWND window, GameMouseInput* mouseInput);

internal void win32PlatformLog(const wchar_t *str, ...);

//===========================================
// Game-required platform layer signatures
//===========================================
PLATFORM_ALLOCATE_MEMORY(platformAllocateMemory);
PLATFORM_FREE_MEMORY(platformFreeMemory);
PLATFORM_TOGGLE_FULLSCREEN(platformToggleFullscreen);
PLATFORM_CONTROLLER_VIBRATE(platformControllerVibrate);

#if HANDMADE_LOCAL_BUILD

DEBUG_PLATFORM_LOG(DEBUG_platformLog);
DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUG_platformReadEntireFile);
DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUG_platformFreeFileMemory);
DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUG_platformWriteEntireFile);

#endif

#ifdef HANDMADE_LIVE_LOOP_EDITING

internal void win32BeginInputRecording(Win32State *win32State);

internal void win32EndInputRecording(Win32State *win32State);

internal void win32RecordInput(Win32State *win32State, GameInput *inputNewInstance);

internal void win32BeginRecordingPlayback(Win32State *win32State);

internal void win32EndRecordingPlayback(Win32State *win32State);

internal void win32PlaybackInput(Win32State *win32State, GameInput *inputNewInstance);

#endif

#endif