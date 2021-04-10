#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE

/**
 * Macro definitions
 */

// Global variables
#define global_var static

// Static variables within a local scope (e.g. case statement, function)
#define local_persist_var static

// Functions that are only available within the file they're declared in
#define internal_func static 

// Return the number of elements in a static array
#define countArray(arr) (sizeof(arr) / sizeof((arr)[0]))

// Maximum number of supported controllers
#define MAX_CONTROLLERS 4

/**
 * Struct for the screen buffer
 */
typedef struct GameFrameBuffer
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

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

} FrameBuffer;

/**
 * Struct for the audio buffer
 */
typedef struct GameAudioBuffer
{
    // Number of audio "samples" that our buffer contains.
    // 1 sample = the grouping of a single left + right channel output.
    // 48,000 samples per second, 4 bytes per sample = 192,000 bytes per buffer (assuming 1 second buffer).
    uint32 samplesPerSecond;

    // How many bytes to store per sample?
    uint8 bytesPerSample;

    // How many seconds worth of the audio should our buffer hold?
    uint8 secondsWorthOfAudio;

    // Target frames per second for our frame buffer.
    uint8 fps;

    // How many samples should we be writing to next?
    uint32 samplesToWrite;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

    // Byte count of the platform's buffer memory
    uint64 platformBufferSizeInBytes;

    // Last position within the buffer that we wrote to.
    uint32 platformRunningByteIndex;

} AudioBuffer;

typedef struct GameControllerBtnState
{
    bool32 halfTransitionCount;
    bool32 endedDown;
} GameControllerBtnState;

typedef struct GameControllerThumbstickState
{
    /*
     * Thumstick X and Y values.
     * Maximum ranges are between 1.0 and -1.0
     */
    struct position {
        float32 x;
        float32 y;
    } position;
} GameControllerThumbstickState;

typedef struct GameControllerInput
{
    bool32 isAnalog;

    union {
        GameControllerBtnState dPadButtons[4];
        struct {
            GameControllerBtnState dPadUp;
            GameControllerBtnState dPadDown;
            GameControllerBtnState dPadLeft;
            GameControllerBtnState dPadRight;
        };
    };

    union {
        GameControllerBtnState buttons[6];
        struct {
            GameControllerBtnState up;
            GameControllerBtnState down;
            GameControllerBtnState left;
            GameControllerBtnState right;
            GameControllerBtnState shoulderL1;
            GameControllerBtnState shoulderR1;
        };
    };

    GameControllerThumbstickState leftThumbstick;
    GameControllerThumbstickState rightThumbstick;

} GameControllerInput;

typedef struct GameInput
{
    GameControllerInput controllers[MAX_CONTROLLERS];
} GameInput;

typedef struct SineWave
{
    // Hertz is the same as "cycles per second".
    // The number of cycles that occur in one second is equal to the
    // frequency of the signal in hertz (abbreviated Hz).
    // Hertz set the tone. E.g. 256 = middle C.
    // @link https://bit.ly/2PCUzdH
    // @link https://en.wikipedia.org/wiki/Scientific_pitch
    uint16 hertz;

    // The taller the wave, the louder the sound.
    uint16 sizeOfWave;

} SineWave;

/*
 * Platform specific printf style output debugger
 *
 * @param char format Format specifier. E.g. "Var is %i\n"
 * @param optional command separated list of variables
 */
internal_func void platformLog(char *format, ...);

/**
 * Platform specific function for vibrating a controller
 */
internal_func void platformControllerVibrate(uint8 controllerIndex,
                                                uint16 motor1Speed,
                                                uint16 motor2Speed);

internal_func void gameUpdate(FrameBuffer *frameBuffer,
                                AudioBuffer *audioBuffer,
                                GameInput inputInstances[],
                                uint8 maxControllers);

internal_func FrameBuffer* gameInitFrameBuffer(FrameBuffer *frameBuffer,
                                                uint32 height,
                                                uint32 width,
                                                uint16 bytesPerPixel,
                                                uint32 byteWidthPerRow,
                                                void *memory);

/**
 * Initialises the game audio buffer ready for writing.
 *
 */
internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer,
                                                uint16 samplesPerSecond,
                                                uint8 bytesPerSample,
                                                uint8 secondsWorthOfAudio,
                                                uint32 samplesToWrite,
                                                uint64 platformBufferSizeInBytes,
                                                uint32 platformLockOffsetInBytes);

internal_func void gameWriteFrameBuffer(FrameBuffer *buffer,
                                            int redOffset,
                                            int greenOffset);

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
float32 percentageOfAnotherf(float32 a, float32 b);

#endif