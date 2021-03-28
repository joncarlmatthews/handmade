#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE

#include "types.h"
#include <math.h> // For Sin

 // Macro definitions
#define global_var          static // Global variables
#define local_persist_var   static // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static // Functions that are only available within the file they're declared in

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
    // How many bits per individual left or right channel? 
    // (16-bits for the left channel's sample, 16-bits for the right channel's sample.)
    uint8 bitsPerChannel;

    // How many bytes to store per "sample"? A single sample is the grouping of one left + right channel.
    // The sum of left + right channel is 32-bits, therefore each sample needs 4 bytes
    // Data written to and read from a the buffer must always start at the beginning of a 16-bit, 4-byte sample "block"
    uint8 bytesPerSample;

    // Byte count of our buffer's memory
    uint64 bufferSizeInBytes;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

} AudioBuffer;

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

internal_func void gameUpdateAndRender(FrameBuffer *frameBuffer, int redOffset, int greenOffset, AudioBuffer *audioBuffer);

internal_func void gameWriteFrameBuffer(FrameBuffer *buffer, int redOffset, int greenOffset);

internal_func void gameWriteAudioBuffer(AudioBuffer *buffer);

internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer, uint8 bitsPerChannel, uint8 bytesPerSample, uint64 bufferSizeInBytes);

internal_func void platformSayHello();

#endif