#include "handmade.h"

static int redOffset = 0;
static int greenOffset = 0;

internal_func void gameUpdateAndRender(FrameBuffer *frameBuffer, AudioBuffer *audioBuffer, GameController controllers[], uint8 maxControllers)
{
    gameWriteAudioBuffer(audioBuffer);

    for (uint8 i = 0; i < maxControllers; i++){

        if (controllers[i].controllerReady) {

            // Animate the screen.
            redOffset       = (redOffset + (controllers[i].leftThumbstickY >> 12));
            greenOffset     = (greenOffset - (controllers[i].leftThumbstickX >> 12));

            // Controller feedback.
            uint16 motor1Speed = 0;
            uint16 motor2Speed = 0;
            if ((controllers[i].leftThumbstickY != 0) || (controllers[i].leftThumbstickX != 0)) {
                motor2Speed = 65000;
            }

            platformControllerVibrate(0, motor1Speed, motor2Speed);

            // Support for first controller only at this point.
            break;
        }
    }
    

    // @TODO(JM) change the sine wave cycles per second based on controller input
    // ...

    gameWriteFrameBuffer(frameBuffer, redOffset, greenOffset);
}

internal_func void gameWriteFrameBuffer(FrameBuffer *buffer, int redOffset, int greenOffset)
{
    // Create a pointer to bitmapMemory
    // In order for us to have maximum control over the pointer arithmatic, we cast it to
    // an 1 byte datatype. This enables us to step through the memory block 1 byte
    // at a time.
    uint8 *row = (uint8*)buffer->memory;

    // Create a loop that iterates for the same number of rows we have for the viewport. 
    // (We know the number of pixel rows from the viewport height)
    // We name the iterator x to denote the x axis (along the corridor)
    for (uint32 x = 0; x < buffer->height; x++) {

        // We know that each pixel is 4 bytes wide (bytesPerPixel) so we make
        // our pointer the same width to grab the relevant block of memory for
        // each pixel. (32 bits = 4 bytes)

        uint32 *pixel = (uint32*)row;

        // Create a loop that iterates for the same number of columns we have for the viewport.
        // (We know the number of pixel columns from the viewport width)
        // We name the iterator y to denote the y axis (up the stairs)
        for (uint32 y = 0; y < buffer->width; y++) {

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

            uint8 red     = (uint8)(x + redOffset);     // Chop off anything after the first 8 bits of the variable x + offset
            uint8 green   = (uint8)(y + greenOffset);   // Chop off anything after the first 8 bits of the variable y + offset
            uint8 blue    = 0;

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

internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer, uint8 bitsPerChannel, uint8 bytesPerSample, uint32 bufferSizeInBytes)
{
    audioBuffer->bitsPerChannel = bitsPerChannel;
    audioBuffer->bufferSizeInBytes = bufferSizeInBytes;
    audioBuffer->bytesPerSample = bytesPerSample;
    audioBuffer->memory = VirtualAlloc(NULL, bufferSizeInBytes, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    return audioBuffer;
}


internal_func void gameWriteAudioBuffer(AudioBuffer *buffer)
{
    uint16 audioSample = (uint16)buffer->memory;
    for (size_t i = 0; i < (buffer->bufferSizeInBytes / buffer->bytesPerSample); i++){

    }
}