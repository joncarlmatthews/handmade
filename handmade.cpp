#include "types.h"
#include <math.h> // For Sin
#include "handmade.h"

internal_func void gameUpdate(GameMemory *memory,
                                FrameBuffer *frameBuffer,
                                AudioBuffer *audioBuffer,
                                GameInput inputInstances[],
                                ControllerCounts *controllerCounts)
{
    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorageSizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->initialised) {
        gameState->sineWave = {0};
        gameState->greenOffset = 1;
        gameState->redOffset = 2;
        memory->initialised = true;
    }

    /**
     * Audio stuff
     */

    // @TODO(JM) change the sine wave cycles per second based on controller input
    gameState->sineWave.hertz = 250;
    gameState->sineWave.sizeOfWave = 100; // Volume

    // Calculate the total number of 4-byte audio sample groups that we will have per complete cycle.
    uint64 audioSampleGroupsPerCycle = ((audioBuffer->platformBufferSizeInBytes / audioBuffer->bytesPerSample) / gameState->sineWave.hertz);

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

        int16 audioSampleValue = (int16)(sine * gameState->sineWave.sizeOfWave);

        // Left channel (16-bits)
        *audioSample = audioSampleValue;

        // Move to the right sample (16-bits)
        audioSample++;

        // Right channel (16-bits)
        *audioSample = audioSampleValue;

        // Move cursor to the start of the next sample grouping.
        audioSample++;

        // Write another 4 to the running byte group index.
        byteGroupIndex = (uint32)((uint64)(byteGroupIndex + audioBuffer->bytesPerSample) % audioSampleGroupsPerCycle);
    }

    /**
     * Graphics stuff
     */
    uint16 movementSpeed = 30;

    for (uint8 i = 0; i < controllerCounts->connectedControllers; i++){

        if (!inputInstances->controllers[i].isConnected) {
            continue;
        }

        // Animate the screen.
        if (inputInstances->controllers[i].dPadUp.endedDown) {
            gameState->redOffset = (gameState->redOffset + movementSpeed);
        }

        if (inputInstances->controllers[i].dPadDown.endedDown) {
            gameState->redOffset = (gameState->redOffset - movementSpeed);
        }

        if (inputInstances->controllers[i].dPadRight.endedDown) {
            gameState->greenOffset = (gameState->greenOffset - movementSpeed);
        }

        if (inputInstances->controllers[i].dPadLeft.endedDown) {
            gameState->greenOffset = (gameState->greenOffset + movementSpeed);
        }

        if (inputInstances->controllers[i].isAnalog) {
            if (inputInstances->controllers[i].leftThumbstick.position.x) {
                gameState->greenOffset = (gameState->greenOffset - (int16)(inputInstances->controllers[i].leftThumbstick.position.x * movementSpeed));
            }

            if (inputInstances->controllers[i].leftThumbstick.position.y) {
                gameState->redOffset = (gameState->redOffset + (int16)(inputInstances->controllers[i].leftThumbstick.position.y * movementSpeed));
            }

            // Controller feedback.
            uint16 motor1Speed = 0;
            uint16 motor2Speed = 0;
            if ((inputInstances->controllers[i].leftThumbstick.position.x != 0)
                || (inputInstances->controllers[i].leftThumbstick.position.y != 0)) {
                motor2Speed = 35000;
            }

            //platformControllerVibrate(0, motor1Speed, motor2Speed);
        }
    }

    gameWriteFrameBuffer(frameBuffer, gameState->redOffset, gameState->greenOffset);
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

internal_func FrameBuffer* gameInitFrameBuffer(FrameBuffer *frameBuffer,
                                                uint32 height,
                                                uint32 width,
                                                uint16 bytesPerPixel,
                                                uint32 byteWidthPerRow,
                                                void *memory)
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

uint64 kibibytesToBytes(uint8 kibibytes)
{
    return (uint64)((uint64)1024 * (uint64)kibibytes);
}

uint64 mebibytesToBytes(uint8 mebibytes)
{
    return (uint64)(((uint64)1024 * kibibytesToBytes(1)) * mebibytes);
}

uint64 gibibytesToBytes(uint8 gibibytes)
{
    return (uint64)(((uint64)1024 * mebibytesToBytes(1)) * gibibytes);
}

uint64 tebibyteToBytes(uint8 tebibytes)
{
    return (uint64)(((uint64)1024 * gibibytesToBytes(1)) * tebibytes);
}

internal_func uint32 truncateToUint32Safe(uint64 value)
{
    assert((value <= 0xffffffff));
    return (uint32)value;
}

float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b == 0) {
        return 0;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}