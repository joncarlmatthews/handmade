#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE


#define HANDMADE_DEBUG_FPS
#define HANDMADE_DEBUG_AUDIO

// If assertion isn't true, write to the null pointer and crash the program.
#if HANDMADE_LOCAL_BUILD
#define assert(expression) if (!(expression)){ int *address = 0x0; *address = 0; }
#else
#define assert(expression)
#endif

// Generic stuff
#define PIf 3.14159265359f

// Global variables
#define global_var static

// Static variables within a local scope (e.g. case statement, function)
#define local_persist_var static

// Functions that are only available within the translation unit they're declared in.
// This helps the compiler out, knowing that there is no external linking to be done.
#define internal_func static 

// Return the number of elements in a static array
#define countArray(arr) (sizeof(arr) / sizeof((arr)[0]))

// Maximum number of supported controllers
// 1 keyboard, 4 gamepad controllers.
#define MAX_CONTROLLERS 5

/*
 * Struct for the screen buffer
 * 
 * Each pixel looks like this (in hex): 00 00 00 00
 * Each of the 00 represents 1 of our 4-byte pixels.
 *
 * As the order of bytes is little endian, the RGB bytes are backwards
 * when writting to them:
 *
 * B    G   R   Padding
 * 00   00  00  00
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

typedef struct ControllerCounts
{
    uint8 gameMaxControllers;
    uint8 platformMaxControllers;
    uint8 connectedControllers;
} ControllerCounts;

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
    bool32 isConnected;
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

typedef struct AncillaryPlatformLayerData {
    struct {
        DWORD playCursorPosition;
        DWORD writeCursorPosition;
        DWORD lockSizeInBytes;
        DWORD lockOffsetInBytes;
    } audioBuffer;
} AncillaryPlatformLayerData;

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

typedef struct GameState
{
    SineWave sineWave;
    int32 redOffset;
    int32 greenOffset;
} GameState;

typedef struct GameMemory
{
    /*
     * Memory address of data we want to store permanently throughout the game.
     */
    void *permanentStorage;

    /*
     * Size, in bytes of the permanent memory storage.
     */
    uint64 permanentStorageSizeInBytes;

    /*
     * Memory address of data we want to store temporarily throughout the game.
     */
    void *transientStorage;

    /*
     * Size, in bytes of the temporary memory storage.
     */
    uint64 transientStorageSizeInBytes;

    /*
     * Flag to set whether or not our game memory has had its initial fill of data.
     */
    bool32 initialised;

} GameMemory;

internal_func void gameUpdate(GameMemory *memory,
                                FrameBuffer *frameBuffer,
                                AudioBuffer *audioBuffer,
                                GameInput inputInstances[],
                                uint8 maxControllers,
                                AncillaryPlatformLayerData ancillaryPlatformLayerData);

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
                                        AncillaryPlatformLayerData ancillaryPlatformLayerData,
                                        int redOffset,
                                        int greenOffset,
                                        AudioBuffer *audioBuffer);

internal_func void writeRectangle(FrameBuffer *buffer, uint32 hexColour, uint64 height, uint64 width, uint64 yOffset, uint64 xOffset);

/*
 * Truncates 8-bytes (uint64) to 4-bytes (uint32). If in debug mode,
 * the code will assert if the value passed in is larger than 4 bytes
 */
internal_func uint32 truncateToUint32Safe(uint64 value);

/*
 * Helper functions to translate kibibytes, mebibytes and gibibytes
 * to bytes (IEC binary standard)
 *
 * @see https://en.wikipedia.org/wiki/Byte#Multiple-byte_units
 * @see https://www.quora.com/Is-1-GB-equal-to-1024-MB-or-1000-MB
 */
uint64 kibibytesToBytes(uint8 kibibytes);
uint64 mebibytesToBytes(uint8 mebibytes);
uint64 gibibytesToBytes(uint8 gibibytes);
uint64 tebibyteToBytes(uint8 tebibytes);

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
float32 percentageOfAnotherf(float32 a, float32 b);

/*
 * *********************************************
 * Services that the platform layer must provide
 * *********************************************
 */

/*
 * printf style output debugger
 *
 * @param char format Format specifier. E.g. "Var is %i\n"
 * @param optional command separated list of variables
 */
internal_func void platformLog(char *format, ...);

/**
 * Function for vibrating the controller
 */
internal_func void platformControllerVibrate(uint8 controllerIndex,
                                                uint16 motor1Speed,
                                                uint16 motor2Speed);

/*
 * Definitions for local builds only. E.g. helper functions/structures
 * to aid debugging. None of these calls should remain in code that
 * ships, hence no stubs for if HANDMADE_LOCAL_BUILD isn't defined.
 */
#if HANDMADE_LOCAL_BUILD

typedef struct DEBUG_file
{
    void *memory;
    uint32 sizeinBytes;

} DEBUG_file;

/*
 * Read an entire file into memory
 *
 * @note call DEBUG_platformFreeFileMemory in a subsequent call.
 */
internal_func DEBUG_file DEBUG_platformReadEntireFile(char *filename);

/*
 * Free file memory read from DEBUG_platformReadEntireFile
 */
internal_func void DEBUG_platformFreeFileMemory(DEBUG_file *file);

/*
 * Write bytes into a new file
 */
internal_func bool32 DEBUG_platformWriteEntireFile(char *filename, void *memory, uint32 memorySizeInBytes);

#endif

#endif