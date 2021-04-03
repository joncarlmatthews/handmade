#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE

#include "types.h"
#include <math.h> // For Sin

 // Macro definitions
#define global_var          static // Global variables
#define local_persist_var   static // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static // Functions that are only available within the file they're declared in

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

    // Byte count of our buffer's memory
    //uint64 bufferSizeInBytes;

    // Target frames per second for our frame buffer.
    uint8 fps;

    // How many samples should we be writing to next?
    uint32 samplesToWrite;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

} AudioBuffer;

typedef struct GameController
{
    bool16 controllerReady;
    bool16 btnUpDepressed;
    bool16 btnDownDepressed;
    bool16 btnLeftDepressed;
    bool16 btnRightDepressed;
    bool16 btnStartDepressed;
    bool16 btnBackDepressed;
    bool16 btnShoulderLeftDepressed;
    bool16 btnShoulderRightDepressed;
    bool16 btnADepressed;
    bool16 btnBDepressed;
    bool16 btnCDepressed;
    bool16 btnDDepressed;

    int16 leftThumbstickX;
    int16 leftThumbstickY;

    int16 rightThumbstickX;
    int16 rightThumbstickY;

} GameController;

typedef struct SineWave
{
    // Hertz is the same as "cycles per second".
    // The number of cycles that occur in one second is equal to the
    // frequency of the signal in hertz (abbreviated Hz).
    // Hertz set the tone. E.g. 256 = middle C.
    // @link https://bit.ly/2PCUzdH
    // @link https://en.wikipedia.org/wiki/Scientific_pitch
    uint16 hertz;

    // Last sine wave value within the buffer that we wrote out.
    float32 runningSineValue;

    // The taller the wave, the louder the sound.
    uint16 sizeOfWave;

} SineWave;

internal_func void gameUpdate(FrameBuffer *frameBuffer, AudioBuffer *audioBuffer, GameController controllers[], uint8 maxControllers);

internal_func FrameBuffer* gameInitFrameBuffer(FrameBuffer *frameBuffer, uint32 height, uint32 width, uint16 bytesPerPixel, uint32 byteWidthPerRow, void *memory);

internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer, uint16 samplesPerSecond, uint8 bytesPerSample, uint8 secondsWorthOfAudio, uint32 samplesToWrite);

internal_func void gameWriteFrameBuffer(FrameBuffer *buffer, int redOffset, int greenOffset);

internal_func void gameWriteAudioBuffer(AudioBuffer *buffer, int16 audioSampleValue);

internal_func void platformControllerVibrate(uint8 controllerIndex, uint16 motor1Speed, uint16 motor2Speed);

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @author Jon Matthews
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
float32 percentageOfAnotherf(float32 a, float32 b);

#endif