#include <math.h> // For Sin
#include "game.h"

// Include the definitions of the utility/helper Functions that are
// shared across the game and platform layer
#include "..\Util\util.cpp"

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
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
        gameState->setBG = 0;

        for (size_t i = 0; i < countArray(gameState->sineWaveHertz); i++)
        {
            uint16 hertz;

            switch (i) {
                default:
                case 0:
                    hertz = 60;
                    break;
                case 1:
                    hertz = 100;
                    break;
                case 2:
                    hertz = 200;
                    break;
                case 3:
                    hertz = 300;
                    break;
                case 4:
                    hertz = 400;
                    break;
            }

            gameState->sineWaveHertz[i] = hertz;
        }

        gameState->sineWaveHertzPos = 2;

        memory->initialised = true;
    }

    /**
     * Audio stuff
     */

    // @TODO(JM) change the sine wave cycles per second based on controller input
    int16 sineWaveHertzPos = gameState->sineWaveHertzPos;

    if (inputInstances->controllers[1].dPadUp.endedDown) {
        if (sineWaveHertzPos != (countArray(gameState->sineWaveHertz) - 1)) {
            sineWaveHertzPos = (sineWaveHertzPos + 1);
        }
    }

    if (inputInstances->controllers[1].dPadDown.endedDown) {
        if (sineWaveHertzPos != 0) {
            sineWaveHertzPos = (sineWaveHertzPos - 1);
        }
    }

    gameState->sineWaveHertzPos = sineWaveHertzPos;

    gameState->sineWave.hertz = gameState->sineWaveHertz[gameState->sineWaveHertzPos];
    gameState->sineWave.sizeOfWave = 1000; // Volume

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
    for (uint32 i = 0; i < audioBuffer->noOfSamplesToWrite; i++) {

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

            (memory->platformControllerVibrate)(0, motor1Speed, motor2Speed);
        }
    }

    writeFrameBuffer(gameState, frameBuffer, ancillaryPlatformLayerData, gameState->redOffset, gameState->greenOffset, audioBuffer);

    gameState->setBG = 1;
}

internal_func void writeFrameBuffer(GameState *gameState,
                                    GameFrameBuffer *buffer,
                                    AncillaryPlatformLayerData ancillaryPlatformLayerData,
                                    int redOffset,
                                    int greenOffset,
                                    GameAudioBuffer *audioBuffer)
{
    // Background fill
    if (!gameState->setBG) {
        writeRectangle(buffer, 0x343434, buffer->height, buffer->width, 0, 0);
    }
    

#if defined(HANDMADE_LOCAL_BUILD) && defined(HANDMADE_DEBUG_AUDIO)

    float32 coefficient = ((float32)buffer->width / (float32)audioBuffer->platformBufferSizeInBytes);

    // Audio buffer box
    if (!gameState->setBG) {
        {
            uint16 height = 100;
            uint16 width = (uint16)((float32)audioBuffer->platformBufferSizeInBytes * coefficient);
            uint32 yOffset = 100;
            writeRectangle(buffer, 0x000066, height, width, yOffset, 0);
        }
    }

    // Play cursor (green)
    {
        uint16 height = 100;
        uint16 width = 10;
        uint32 yOffset = 100;
        uint32 xOffset = (uint32)((float32)ancillaryPlatformLayerData.audioBuffer.playCursorPosition * coefficient);
        writeRectangle(buffer, 0x006600, height, width, yOffset, xOffset);
    }

    // Write cursor + lock size (amount written) (red)
    {
        uint16 height = 100;
        uint32 width = (uint32)((float32)ancillaryPlatformLayerData.audioBuffer.lockSizeInBytes * coefficient);
        uint32 yOffset = 100;
        uint32 xOffset = (uint32)((float32)ancillaryPlatformLayerData.audioBuffer.writeCursorPosition * coefficient);
        writeRectangle(buffer, 0xcc0000, height, width, yOffset, xOffset);
    }

#endif

}

EXTERN_DLL_EXPORT GAME_INIT_FRAME_BUFFER(gameInitFrameBuffer)
{
    frameBuffer->height = height;
    frameBuffer->width = width;
    frameBuffer->bytesPerPixel = bytesPerPixel;
    frameBuffer->byteWidthPerRow = byteWidthPerRow;
    frameBuffer->memory = memory;

    return frameBuffer;
}

EXTERN_DLL_EXPORT GAME_INIT_AUDIO_BUFFER(gameInitAudioBuffer)
{
    if ( (noOfBytesToWrite <= 0) || (bytesPerSample <= 0) ) {
        return audioBuffer;
    }

    uint32 noOfSamplesToWrite = (noOfBytesToWrite / bytesPerSample);

    if (audioBuffer->noOfSamplesToWrite != noOfSamplesToWrite) {

        // @TODO(JM) move the audio memory to the GameMemory object
        if (!audioBuffer->initialised) {
            audioBuffer->initialised = 1;
            audioBuffer->memory = memory->platformAllocateMemory(noOfBytesToWrite);
        } else {
            memory->platformFreeMemory(audioBuffer->memory);
            audioBuffer->memory = memory->platformAllocateMemory(noOfBytesToWrite);
        }
    }
    audioBuffer->bytesPerSample             = bytesPerSample;
    audioBuffer->noOfSamplesToWrite         = noOfSamplesToWrite;
    audioBuffer->platformBufferSizeInBytes  = platformBufferSizeInBytes;

    return audioBuffer;
}

internal_func void writeRectangle(GameFrameBuffer *buffer, uint32 hexColour, uint64 height, uint64 width, uint64 yOffset, uint64 xOffset)
{
    uint32 *row = (uint32 *)buffer->memory;

    // Move down to starting row
    row = (row + (buffer->width * yOffset));
    row = (row + xOffset);

    // Down
    for (uint64 i = 0; i < height; i++) {
        // Accross
        uint32 *pixel = (uint32 *)row;
        for (uint64 x = 0; x < width; x++) {
            *pixel = hexColour;
            pixel = (pixel + 1);
        }
        row = (row + buffer->width);
    }
}