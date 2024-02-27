#ifndef HEADER_HH_GAME
#define HEADER_HH_GAME

//
// GAME.H
//==============================================================================
// Public header file for game layer

// Project includes
#include "startup.h"
#include "game_types.h"
#include "math.h"
#include "intrinsics.h"
#include "utility.h"
#include "filesystem.h"
#include "random.h"
#include "memory.h"
#include "graphics.h"
#include "audio.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"

/**
 * Preprocessor definitions
 */

// Scroll type. Defaults to smooth
#define SCROLL_TYPE_SMOOTH 1
//#define SCROLL_TYPE_SCREEN 1

#if (!defined(SCROLL_TYPE_SMOOTH))
#define SCROLL_TYPE_SMOOTH 0
#endif

#if (!defined(SCROLL_TYPE_SCREEN))
#define SCROLL_TYPE_SCREEN 0
#endif

#if (!SCROLL_TYPE_SMOOTH) && (!SCROLL_TYPE_SCREEN)

#undef SCROLL_TYPE_SMOOTH
#define SCROLL_TYPE_SMOOTH 1

#endif

#if (SCROLL_TYPE_SMOOTH) && (SCROLL_TYPE_SCREEN)
    assert(!"Both scroll types cannot be enabled at the same time")
#endif

//==============================================================================
//==============================================================================
// Services that the platform layer must provide:
//==============================================================================
//==============================================================================

#if HANDMADE_LOCAL_BUILD

/*
 * Useage:
 * char buff[50] = {0};
 * memory->DEBUG_platformLog(buff, sizeof(buff), "Hello world\n");
 */
#define DEBUG_PLATFORM_LOG(name) int name(char* const buffer, sizet const sizeOfBuffer, char const* const format, ...)
typedef DEBUG_PLATFORM_LOG(DEBUGPlatformLog);

#endif

typedef struct PlatformThreadContext
{
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

/**
 * Toggle the fullscreen mode each time function is called.
 *
 * @param address Pointer to the memory block
 *
 * @return void
*/
#define PLATFORM_TOGGLE_FULLSCREEN(name) void name(void *platformStateWindows, void *platformStateMacOS, void *platformStateLinux)
typedef PLATFORM_TOGGLE_FULLSCREEN(PlatformToggleFullscreen);

//==============================================================================
//==============================================================================
// End platform layer services
//==============================================================================
//==============================================================================

//
// Graphics
//==============================================================================

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
//==============================================================================

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
//==============================================================================

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
            GameControllerBtnState option1;
            GameControllerBtnState option2;
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
    float32 deltaTime; // The time elapsed between the last frame and the one preceding it express in seconds
} GameInput;

//
// Game state & memory
//==============================================================================
typedef struct GameMemory
{
    void *platformStateWindows;
    void *platformStateMacOS;
    void *platformStateLinux;

    MemoryRegion permanentStorage;
    MemoryRegion transientStorage;

#ifdef HANDMADE_LIVE_LOOP_EDITING
    void *recordingStorageGameState;
    void *recordingStorageInput;
#endif

    // Flag to set whether or not our game memory has had its initial fill of data.
    bool32 initialised;

    // Absolute path to the folder that contains the running program
    char platformAbsPath[GAME_MAX_PATH];

    // Pointers to memory allocation deallocation functions with the platform layer
    PlatformAllocateMemory *platformAllocateMemory;
    PlatformFreeMemory *platformFreeMemory;

    PlatformToggleFullscreen *platformToggleFullscreen;

    // @NOTE(JM) Move this??
    PlarformControllerVibrate *platformControllerVibrate;

#ifdef HANDMADE_LOCAL_BUILD
    DEBUGPlatformLog *DEBUG_platformLog;
    DEBUGPlatformReadEntireFile *DEBUG_platformReadEntireFile;
    DEBUGPlatformFreeFileMemory *DEBUG_platformFreeFileMemory;
    DEBUGPlatformWriteEntireFile *DEBUG_platformWriteEntireFile;
#endif

} GameMemory;

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
                                void *platformStateWindows, \
                                void *platformStateMacOS, \
                                void *platformStateLinux, \
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

/**
 * The GameState essentially sits inside (overlays) the memory's permanent storage
 * region's bytes
*/
typedef struct GameState
{
    Player player1;
    World world;

    MemoryBlock tileChunksMemoryBlock;
    MemoryBlock tilesMemoryBlock;

    // The currently active world position based off of the player's
    // absolute position
    TilemapPosition worldPosition;

    TilemapPosition cameraPosition;

    SineWave sineWave;

    float32 angle;

} GameState;

#endif