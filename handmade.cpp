#include "handmade.h"

internal_func void gameUpdate(FrameBuffer *frameBuffer, AudioBuffer *audioBuffer, GameController controllers[], uint8 maxControllers)
{
    /**
     * Audio stuff
     */
    local_persist_var SineWave sineWave = {0};

    // @TODO(JM) change the sine wave cycles per second based on controller input
    sineWave.hertz = 250;
    sineWave.sizeOfWave = 5000; // Volume

    // Calculate the total number of 4-byte audio sample groups that we will have per complete cycle.
    uint64 audioSampleGroupsPerCycle = ((audioBuffer->platformBufferSizeInBytes / audioBuffer->bytesPerSample) / sineWave.hertz);

    // At the start of which 4 byte group index we are starting our write from?
    // @TODO(JM) assert that this is a 4 byte boundry
    uint32 byteGroupIndex = 0;

    float32 percentageOfAngle = 0.0f;
    float32 angle = 0.0f;
    float32 radians = 0.0f;
    float32 sine = 0.0f;

    uint16 *audioSample = (uint16 *)audioBuffer->memory;

    // Iterate over each 2 - bytes and write the same data for both...
    for (uint32 i = 0; i < audioBuffer->samplesToWrite; i++) {

        percentageOfAngle = percentageOfAnotherf((float32)byteGroupIndex, (float32)audioSampleGroupsPerCycle);
        angle = (360.0f * (percentageOfAngle / 100.0f));
        radians = (angle * (PIf / 180.0f));
        sine = sinf(radians);

        int16 audioSampleValue = (int16)(sine * sineWave.sizeOfWave);

        // Left channel (16-bits)
        *audioSample = audioSampleValue;

        // Move to the right sample (16-bits)
        audioSample++;

        // Right channel (16-bits)
        *audioSample = audioSampleValue;

        // Move cursor to the start of the next sample grouping.
        audioSample++;

        // Write another 4 to the running byte group index.
        byteGroupIndex = ((byteGroupIndex + audioBuffer->bytesPerSample) % audioSampleGroupsPerCycle);
    }

    //gameWriteAudioBuffer(audioBuffer, 100);

    /**
     * Graphics stuff
     */
    local_persist_var int32 redOffset = 0;
    local_persist_var int32 greenOffset = 0;

    for (uint8 i = 0; i < maxControllers; i++){

        if (controllers[i].controllerReady) {

            // Animate the screen.
            redOffset       = (redOffset    + (controllers[i].leftThumbstickY >> 10));
            greenOffset     = (greenOffset  - (controllers[i].leftThumbstickX >> 10));

            // Controller feedback.
            uint16 motor1Speed = 0;
            uint16 motor2Speed = 0;
            if ((controllers[i].leftThumbstickY != 0) || (controllers[i].leftThumbstickX != 0)) {
                motor2Speed = 35000;
            }

            platformControllerVibrate(0, motor1Speed, motor2Speed);

            // Support for first controller only at this point.
            break;
        }
    }

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

internal_func FrameBuffer* gameInitFrameBuffer(FrameBuffer *frameBuffer, uint32 height, uint32 width, uint16 bytesPerPixel, uint32 byteWidthPerRow, void *memory)
{
    frameBuffer->height = height;
    frameBuffer->width = width;
    frameBuffer->bytesPerPixel = bytesPerPixel;
    frameBuffer->byteWidthPerRow = byteWidthPerRow;
    frameBuffer->memory = memory;

    return frameBuffer;
}

internal_func AudioBuffer* gameInitAudioBuffer(AudioBuffer *audioBuffer,
                                                uint16 samplesPerSecond,
                                                uint8 bytesPerSample,
                                                uint8 secondsWorthOfAudio,
                                                uint32 samplesToWrite,
                                                uint64 platformBufferSizeInBytes,
                                                uint32 platformLockOffsetInBytes)
{
    audioBuffer->samplesPerSecond           = samplesPerSecond;
    audioBuffer->bytesPerSample             = bytesPerSample;
    audioBuffer->secondsWorthOfAudio        = secondsWorthOfAudio;
    audioBuffer->samplesToWrite             = samplesToWrite;
    audioBuffer->platformBufferSizeInBytes  = platformBufferSizeInBytes;

    return audioBuffer;
}


internal_func void gameWriteAudioBuffer(AudioBuffer *buffer, int16 audioSampleValue)
{
    uint16 *audioSample = (uint16 *)buffer->memory;

    for (uint32 i = 0; i < buffer->samplesToWrite; i++){

        // Left channel (16-bits)
        *audioSample = audioSampleValue;

        // Move to the right sample (16-bits)
        audioSample++;

        // Right channel (16-bits)
        *audioSample = audioSampleValue;

        // Move cursor to the start of the next sample grouping.
        audioSample++;
    }
}

float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b == 0) {
        return 0;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}