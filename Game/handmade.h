#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE

#include "types.h"

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

} GameFrameBuffer;

/**
 * Struct for the audio buffer
 */
typedef struct GameAudioBuffer
{
    // Memory and properties initialised?
    bool8 initialised;

    // How many bytes to store per sample? 1 sample = left + right channel
    uint8 bytesPerSample;

    // Write entire buffer? If FALSE, minFramesWorthOfAudio should be defined. 
    bool8 writeEntireBuffer;

    // As a minimum, how many frames worth of audio should we write per game loop?
    // We add extra frames so we have reserve audio should the game hit
    // a slowdown in the frame rate. This mitigates audio cuts.
    // Ignored if writeEntireBuffer is TRUE
    uint8 minFramesWorthOfAudio;

    // How many samples should we be writing to next?
    uint32 noOfSamplesToWrite;

    // How many bytes should we be writing to next?
    uint32 noOfBytesToWrite;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

    // Byte count of the platform's buffer memory
    uint64 platformBufferSizeInBytes;

} GameAudioBuffer;

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
    uint16 sineWaveHertz[5] = { 60, 100, 200, 300, 400};
    int16 sineWaveHertzPos = 2;
    uint8 setBG = false;
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
                                GameFrameBuffer *frameBuffer,
                                GameAudioBuffer *audioBuffer,
                                GameInput inputInstances[],
                                ControllerCounts *controllerCounts,
                                AncillaryPlatformLayerData ancillaryPlatformLayerData);

/**
 * Initialises the game's frame buffer ready for writing.
 *
 */
internal_func GameFrameBuffer* gameInitFrameBuffer(GameFrameBuffer *frameBuffer,
                                                uint32 height,
                                                uint32 width,
                                                uint16 bytesPerPixel,
                                                uint32 byteWidthPerRow,
                                                void *memory);

/**
 * Initialises the game's audio buffer ready for writing.
 *
 */
internal_func GameAudioBuffer* gameInitAudioBuffer(GameAudioBuffer *audioBuffer,
                                                    uint32 noOfBytesToWrite,
                                                    uint8 bytesPerSample,
                                                    uint64 platformBufferSizeInBytes);

internal_func void gameWriteFrameBuffer(GameState *gameState,
                                        GameFrameBuffer *buffer,
                                        AncillaryPlatformLayerData ancillaryPlatformLayerData,
                                        int redOffset,
                                        int greenOffset,
                                        GameAudioBuffer *audioBuffer);

internal_func void writeRectangle(GameFrameBuffer *buffer, uint32 hexColour, uint64 height, uint64 width, uint64 yOffset, uint64 xOffset);

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
internal_func uint64 kibibytesToBytes(uint8 kibibytes);
internal_func uint64 mebibytesToBytes(uint8 mebibytes);
internal_func uint64 gibibytesToBytes(uint8 gibibytes);
internal_func uint64 tebibyteToBytes(uint8 tebibytes);

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
internal_func float32 percentageOfAnotherf(float32 a, float32 b);

internal_func void *platformAllocateMemory(uint32 bytes);
internal_func void platformFreeMemory(void *address);

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