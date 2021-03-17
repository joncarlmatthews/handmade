#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE

// Typedefs that specify exact-width integer types for increased code portability.
/*
 * char:        (1)     int8_t  / uint8_t   (-128 127)          (0 255)
 * short:       (2)     int16_t / uint16_t  (-32,768 32,767)    (0 65,536)
 * int (long):  (4)     int32_t / uint32_t  (-2.1bn to 2.1bn)   (0 to 4.2bn)
 * long long:   (8)     int64_t / uint64_t  (-9qn 9qn)          (0-18qn)
 */
#include <stdint.h>
#include <math.h> // For Sin

 // Macro definitions
#define global_var          static // Global variables
#define local_persist_var   static // Static variables within a local scope (e.g. case statement, function)
#define internal_func       static // Functions that are only available within the file they're declared in

// Typedefs
typedef uint32_t    bool32; // For 0 or "> 0 I don't care" booleans
typedef float       float32;
typedef double      float64;

/**
 * Struct for the screen buffer
 */
typedef struct GameFrameBuffer
{
    // The width in pixels of the buffer.
    uint32_t width;

    // The height in pixels of the buffer.
    uint32_t height;

    // 1 byte each for R, G & B and 1 byte for padding to match byte boundries (4)
    // Therefore our pixels are always 32-bits wide and are in Little Endian 
    // memory order (backwards) 0xBBGGRRPP
    uint16_t bytesPerPixel;

    // The number of bytes per row. (width * bytesPerPixel)
    uint32_t byteWidthPerRow;

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
    uint8_t bitsPerChannel;

    // How many bytes to store per "sample"? A single sample is the grouping of one left + right channel.
    // The sum of left + right channel is 32-bits, therefore each sample needs 4 bytes
    // Data written to and read from a the buffer must always start at the beginning of a 16-bit, 4-byte sample "block"
    uint8_t bytesPerSample;

    // Byte count of our buffer's memory
    uint64_t bufferSizeInBytes;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void *memory;

} AudioBuffer;

internal_func void gameUpdateAndRender(FrameBuffer *frameBuffer, int redOffset, int greenOffset, AudioBuffer *audioBuffer);
internal_func void gameWriteFrameBuffer(FrameBuffer *buffer, int redOffset, int greenOffset);
internal_func void gameWriteAudioBuffer(AudioBuffer *buffer);
internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer, uint8_t bitsPerChannel, uint8_t bytesPerSample, uint64_t bufferSizeInBytes);

#endif