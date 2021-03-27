#include "handmade.h"

internal_func int64_t ceiling(int64_t number, int64_t factor)
{
    int isPositive = (int)(number >= 0);
    return ((number + isPositive * (factor - 1)) / factor) * factor;
}

static int counter = 0;

internal_func void gameUpdateAndRender(FrameBuffer *frameBuffer, 
                                        int redOffset, 
                                        int greenOffset, 
                                        AudioBuffer *audioBuffer)
{
    counter++;
    platformSayHello(&counter);
    gameWriteAudioBuffer(audioBuffer);
    gameWriteFrameBuffer(frameBuffer, redOffset, greenOffset);
}

internal_func void gameWriteFrameBuffer(FrameBuffer *buffer, int redOffset, int greenOffset)
{
    // Create a pointer to bitmapMemory
    // In order for us to have maximum control over the pointer arithmatic, we cast it to
    // an 1 byte datatype. This enables us to step through the memory block 1 byte
    // at a time.
    uint8_t *row = (uint8_t*)buffer->memory;

    // Create a loop that iterates for the same number of rows we have for the viewport. 
    // (We know the number of pixel rows from the viewport height)
    // We name the iterator x to denote the x axis (along the corridor)
    for (uint32_t x = 0; x < buffer->height; x++) {

        // We know that each pixel is 4 bytes wide (bytesPerPixel) so we make
        // our pointer the same width to grab the relevant block of memory for
        // each pixel. (32 bits = 4 bytes)

        uint32_t *pixel = (uint32_t*)row;

        // Create a loop that iterates for the same number of columns we have for the viewport.
        // (We know the number of pixel columns from the viewport width)
        // We name the iterator y to denote the y axis (up the stairs)
        for (uint32_t y = 0; y < buffer->width; y++) {

            /*
             * Write to this pixel...
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

            uint8_t red     = (uint8_t)(x + redOffset);     // Chop off anything after the first 8 bits of the variable x + offset
            uint8_t green   = (uint8_t)(y + greenOffset);   // Chop off anything after the first 8 bits of the variable y + offset
            uint8_t blue    = 0;

            *pixel = ((red << 16) | (green << 8) | blue);

            // Move the pointer forward to the start of the next 4 byte block
            pixel = (pixel + 1);
        }

        // Move the row pointer forward by the byte width of the row so that for
        // the next iteration of the row we're then starting at the first byte
        // of that particular row
        row = (row + buffer->byteWidthPerRow);
    }
}

internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer, uint8_t bitsPerChannel, uint8_t bytesPerSample, uint32_t bufferSizeInBytes)
{
    audioBuffer->bitsPerChannel = bitsPerChannel;
    audioBuffer->bufferSizeInBytes = bufferSizeInBytes;
    audioBuffer->bytesPerSample = bytesPerSample;
    audioBuffer->memory = VirtualAlloc(NULL, bufferSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return audioBuffer;
}


internal_func void gameWriteAudioBuffer(AudioBuffer *buffer)
{
    uint16_t audioSample = (uint16_t)buffer->memory;
    for (size_t i = 0; i < (buffer->bufferSizeInBytes / buffer->bytesPerSample); i++){

    }
}