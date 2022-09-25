#ifndef HEADER_HH_GAME
#define HEADER_HH_GAME

#include "types.h"
#include "memory.h"
#include "tilemap.h"
#include "player.h"

#if HANDMADE_LOCAL_BUILD

    // Flags:
    // #define HANDMADE_DEBUG_TILE_POS
    // #define HANDMADE_LIVE_LOOP_EDITING
    // #define HANDMADE_DEBUG
    // #define HANDMADE_DEBUG_FPS
    // #define HANDMADE_DEBUG_CLOCKCYCLES
    // #define HANDMADE_DEBUG_AUDIO
#endif

#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

// Return the number of elements in a static array
#define countArray(arr) (sizeof(arr) / sizeof((arr)[0]))

// Maximum number of supported controllers
// 1 keyboard, 4 gamepad controllers.
#define MAX_CONTROLLERS 5

//====================================================
//====================================================
// Services that the platform layer must provide
//====================================================
//====================================================

typedef struct PlatformThreadContext {
    uint8 placeholder;
} PlatformThreadContext;

 /**
  * @brief Requests the platform layer allocated @link bytes worth of memory
  *
  * @param memoryStartAddress The location in memory to start the allocation
  * @param bytes The amount of bytes to allocate
  * @return void pointer to the blockmemory
 */
#define PLATFORM_ALLOCATE_MEMORY(name) void *name(PlatformThreadContext *thread, uint64 memoryStartAddress, uint64 memorySizeInBytes)
typedef PLATFORM_ALLOCATE_MEMORY(PlatformAllocateMemory);

/**
 * @brief Requests the platform layer free the memory for a given address
 *
 * @param address Pointer to the memory block
 *
 * @return void
*/
#define PLATFORM_FREE_MEMORY(name) void name(PlatformThreadContext *thread, void *address)
typedef PLATFORM_FREE_MEMORY(PlatformFreeMemory);

/*
 * Vibrates the controller
 *
 * @param uint8 controllerIndex which controller to vibrate
 * @param uint16 speed to vibrate motor 1
 * @param uint16 speed to vibrate motor 2
 *
 * @return void
 */
#define PLATFORM_CONTROLLER_VIBRATE(name) void name(PlatformThreadContext *thread, uint8 controllerIndex, uint16 motor1Speed, uint16 motor2Speed)
typedef PLATFORM_CONTROLLER_VIBRATE(PlarformControllerVibrate);

/*
 * Definitions for local builds only. E.g. helper functions/structures
 * to aid debugging. None of these calls should remain in code that
 * ships, hence no stubs for if HANDMADE_LOCAL_BUILD isn't defined.
 */
#if HANDMADE_LOCAL_BUILD

    #define DEBUG_PLATFORM_LOG(name) void name(char *buff)
    typedef DEBUG_PLATFORM_LOG(DEBUGPlatformLog);

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
    #define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DEBUG_file name(PlatformThreadContext *thread, char *filename)
    typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile);

    /*
     * Free file memory read from DEBUG_platformReadEntireFile
     */
    #define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(PlatformThreadContext *thread, DEBUG_file *file)
    typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory);

    /*
     * Write bytes into a new file
     */
    #define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(PlatformThreadContext *thread, char *filename, void *memory, uint32 memorySizeInBytes)
    typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile);

#endif

//====================================================
//====================================================
// End platform layer services
//=====================================================
//====================================================

//
// Graphics
//====================================================

#define FRAME_BUFFER_PIXEL_WIDTH  1280
#define FRAME_BUFFER_PIXEL_HEIGHT 720

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
    uint32 widthPx;

    // The height in pixels of the buffer.
    uint32 heightPx;

    // 1 byte each for R, G & B and 1 byte for padding to match byte boundries (4)
    // Therefore our pixels are always 32-bits wide and are in Little Endian 
    // memory order (backwards) 0xBBGGRRPP
    uint16 bytesPerPixel;

    // The number of bytes per row. (width * bytesPerPixel)
    uint32 byteWidthPerRow;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

} GameFrameBuffer;

//
// Audio
//====================================================

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

#if defined(HANDMADE_DEBUG_AUDIO)

    unsigned long playCursorPosition;
    unsigned long writeCursorPosition;
    unsigned long lockSizeInBytes;

#endif

} GameAudioBuffer;

//
// Controller/input
//====================================================

typedef struct ControllerCounts
{
    uint8 gameMaxControllers;
    uint8 platformMaxControllers;
    uint8 connectedControllers;
} ControllerCounts;

typedef struct GameControllerBtnState
{
    bool endedDown;
    bool wasDown;
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

typedef struct GameMouseInput {
    bool isConnected;
    GameControllerBtnState leftClick;
    GameControllerBtnState rightClick;
    struct position {
        int32 x;
        int32 y;
    } position;
} GameMouseInput;

typedef struct GameInput
{
    GameMouseInput mouse;
    GameControllerInput controllers[MAX_CONTROLLERS];
    float32 msPerFrame; // How many miliseconds are we taking per frame? E.g. 16.6 or 33.3
    uint8 targetFPS; // Our target FPS
    float32 fps; // What is our actual FPS
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

//
// Game state & memory
//====================================================
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

#if HANDMADE_LOCAL_BUILD
    void *recordingStorageGameState;
    void *recordingStorageInput;
#endif

    /*
     * Flag to set whether or not our game memory has had its initial fill of data.
     */
    bool32 initialised;

    PlatformAllocateMemory *platformAllocateMemory;
    PlatformFreeMemory *platformFreeMemory;

    // @NOTE(JM) Move this??
    PlarformControllerVibrate *platformControllerVibrate;

#if HANDMADE_LOCAL_BUILD
    DEBUGPlatformLog *DEBUG_platformLog;
    DEBUGPlatformReadEntireFile *DEBUG_platformReadEntireFile;
    DEBUGPlatformFreeFileMemory *DEBUG_platformFreeFileMemory;
    DEBUGPlatformWriteEntireFile *DEBUG_platformWriteEntireFile;
#endif

} GameMemory;

typedef struct GameState
{
    Player player1;

    GameMemoryBlock worldMemoryBlock;
    World *world;

    // The currently active world position based off of the player's absolute position
    TilemapCoordinates worldPosition;

    // X and Y pixel coordinates for the camera's starting position (to start drawing from)
    // Camera is drawn out to dimensions of GameFrameBuffer.width/height
    xyuint cameraPositionPx;

    SineWave sineWave;
} GameState;

//
// Graphics
//====================================================

/**
 * Insert a rectangle into the frame buffer
 *
 * @param xOffset   Offset, in pixels along the x axis to start drawing from
 * @param yOffset   Offset, in pixels along the y axis to start drawing from
 * @param width     Width, in pixels, to draw
 * @param height    Height, in pixels, to draw
 * @return void
 */
internal_func
void writeRectangle(GameFrameBuffer* buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    Colour colour);

//
// Audio
//====================================================
internal_func
void audioBufferWriteSineWave(GameState *gameState,
                                GameAudioBuffer *audioBuffer);
internal_func
void frameBufferWriteAudioDebug(GameState *gameState,
                                GameFrameBuffer *buffer,
                                GameAudioBuffer *audioBuffer);



//====================================================
//====================================================
// Functions that the game layer exposes to the platform
// layer. These functions are externally exposed from
// within the DLL
//====================================================
//====================================================

/**
 * @brief Main game loop update method
 * 
 * @param memory Pointer to game memory
 * @param frameBuffer Pointer to the initialised frame buffer
 * @param audioBuffer Pointer to the initialised audio buffer 
 * @param inputInstances Array of GameInput devices
 * @param controllerCounts Pointer to the ControllerCounts structs
 * 
 * @return void
*/
#define GAME_UPDATE(name) void name(PlatformThreadContext *thread, \
                                    GameMemory *memory, \
                                    GameFrameBuffer *frameBuffer, \
                                    GameAudioBuffer *audioBuffer, \
                                    GameInput inputInstances[], \
                                    ControllerCounts *controllerCounts)
typedef GAME_UPDATE(GameUpdate);

/**
 * @brief Initialises the game's frame buffer ready for writing.
 *
 * @param frameBuffer
 * @param height
 * @param width
 * @param bytesPerPixel
 * @param byteWidthPerRow
 * @parm memory
 *
 * @return GameFrameBuffer Pointer to the GameFrameBuffer struct
*/
#define GAME_INIT_FRAME_BUFFER(name) GameFrameBuffer* name(PlatformThreadContext *thread, \
                                                            GameFrameBuffer *frameBuffer, \
                                                            uint32 height, \
                                                            uint32 width, \
                                                            uint16 bytesPerPixel, \
                                                            uint32 byteWidthPerRow, \
                                                            void *memory)
typedef GAME_INIT_FRAME_BUFFER(GameInitFrameBuffer);


/**
 * @brief Initialises the game's audio buffer ready for writing.
 * 
 * @param audioBuffer 
 * @param noOfBytesToWrite 
 * @param bytesPerSample 
 * @param platformBufferSizeInBytes
 * 
 * @return GameAudioBuffer Pointer to the GameAudioBuffer struct
*/
#define GAME_INIT_AUDIO_BUFFER(name) GameAudioBuffer* name(PlatformThreadContext *thread, \
                                                            GameMemory *memory, \
                                                            GameAudioBuffer *audioBuffer, \
                                                            uint32 noOfBytesToWrite, \
                                                            uint8 bytesPerSample, \
                                                            uint64 platformBufferSizeInBytes)
typedef GAME_INIT_AUDIO_BUFFER(GameInitAudioBuffer);


/**
 * Struct to assign pointers to internal game code functions
 * so the platform layer can access them.
*/
typedef struct GameCode {
    void *dllHandle;
    GameUpdate *gameUpdate;
    GameInitFrameBuffer *gameInitFrameBuffer;
    GameInitAudioBuffer *gameInitAudioBuffer;
} GameCode;

//====================================================
//====================================================
// End exposed game layer functions
//====================================================
//====================================================

#endif