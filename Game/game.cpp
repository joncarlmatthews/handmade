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

        gameState->bgColour = 0x000066;
        gameState->player1.height = 25;
        gameState->player1.width = 25;
        gameState->player1.totalJumpMovement = 20.0f;
        gameState->player1.movementSpeed = 30;

        gameState->sineWave = { 0 };

        memory->initialised = true;
    }

    /**
     * Audio stuff...
     */

    //audioBufferWriteSineWave(gameState, audioBuffer);

    /**
     * Handle controller input...
     */

    // Main controller loop
    for (uint8 i = 0; i < controllerCounts->connectedControllers; i++){

        if (!inputInstances->controllers[i].isConnected) {
            continue;
        }

        // @TODO(JM) Support for multiple controllers. See below for single controller support.
    }

    // Which controller has the user selected as the main controller?
    uint8 userSelectedMainController = 1; // @TODO(JM) make this selectable through a UI

    controllerHandlePlayer(gameState, frameBuffer, audioBuffer, inputInstances->controllers[userSelectedMainController]);

    /**
     * Write the frame buffer...
     * 
     */
    frameBufferWriteBackground(gameState, frameBuffer, audioBuffer);
    frameBufferWritePlayer(gameState, frameBuffer, audioBuffer);

#if defined(HANDMADE_DEBUG_AUDIO)
    frameBufferWriteAudioDebug(gameState, frameBuffer, audioBuffer);
#endif

}

internal_func void audioBufferWriteSineWave(GameState *gameState, GameAudioBuffer *audioBuffer)
{
    gameState->sineWave.hertz = 100;
    gameState->sineWave.sizeOfWave = 1000; // Volume

    // Calculate the total number of 4-byte audio sample groups that we will have per complete cycle.
    uint64 audioSampleGroupsPerCycle = ((audioBuffer->platformBufferSizeInBytes / audioBuffer->bytesPerSample) / gameState->sineWave.hertz);

    // At the start of which 4 byte group index we are starting our write from?
    // @TODO(JM) assert that this is a 4 byte boundry
    uint32 byteGroupIndex = 0;

    float32 percentageOfAngle = 0.0f;
    float32 angle = 0.0f;
    float32 radians = 0.0f;
    float64 sine = 0.0f;

    uint16 *audioSample = (uint16 *)audioBuffer->memory;

    // Iterate over each 2 - bytes and write the same data for both...
    for (uint32 i = 0; i < audioBuffer->noOfSamplesToWrite; i++) {

        percentageOfAngle = percentageOfAnotherf((float32)byteGroupIndex, (float32)audioSampleGroupsPerCycle);
        angle = (360.0f * (percentageOfAngle / 100.0f));
        radians = (angle * ((float32)M_PI / 180.0f));
        sine = sin(radians);

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
}

internal_func void controllerHandlePlayer(GameState *gameState, GameFrameBuffer *frameBuffer, GameAudioBuffer *audioBuffer, GameControllerInput controller)
{
    if (controller.up.endedDown) {
        gameState->bgColour = 0xffffff;
    }

    // Temp jump code
    if ((controller.down.endedDown) && (0 == gameState->player1.jumping)) {
        gameState->player1.jumping = 1;
        gameState->player1.jumpDuration = 20.0f;
        gameState->player1.jumpRunningFrameCtr = 0.0f;
        gameState->player1.jumpStartPos = gameState->player1.posY;
    }

    if (gameState->player1.jumping) {

        float32 jumpPercentageOfAngle = 0.0f;
        float32 jumpAngle = 0.0f;
        float32 jumpRadians = 0.0f;
        float32 jumpSine = 0.0f;

        gameState->player1.jumpRunningFrameCtr++;

        jumpPercentageOfAngle = percentageOfAnotherf(gameState->player1.jumpRunningFrameCtr, gameState->player1.jumpDuration);
        jumpAngle = (360.0f * (jumpPercentageOfAngle / 100.0f));
        jumpRadians = (jumpAngle * ((float32)M_PI / 180.0f));
        jumpSine = sinf(jumpRadians);

        if (jumpAngle >= 0.0f && jumpAngle <= 180.0f) {
            gameState->player1.jumpDirection = JUMP_UP;
        } else {
            gameState->player1.jumpDirection = JUMP_DOWN;
        }

        float32 amtToMove   = (gameState->player1.totalJumpMovement * jumpSine);
        int32 newPos        = (int32)(gameState->player1.posY - amtToMove);

        // Have we hit the top?
        if (newPos < 0) {
            gameState->player1.posY = 0;
            gameState->player1.jumpDuration = (gameState->player1.jumpRunningFrameCtr / 2);
        } else {
            gameState->player1.posY = newPos;
        }

        // Jump finished?
        if (jumpAngle >= 360.0f) {
            gameState->player1.jumping = 0;
            // Force reset the position.
            // @NOTE(JM) This is a bug if you land on a new level.
            gameState->player1.posY = gameState->player1.jumpStartPos; 
        }
    }

    // Basic player movement
    if (controller.dPadUp.endedDown) {
        gameState->player1.posY = (gameState->player1.posY - gameState->player1.movementSpeed);
    }

    if (controller.dPadDown.endedDown) {
        gameState->player1.posY = (gameState->player1.posY + gameState->player1.movementSpeed);
    }

    if (controller.dPadLeft.endedDown) {
        gameState->player1.posX = (gameState->player1.posX - gameState->player1.movementSpeed);
    }

    if (controller.dPadRight.endedDown) {
        gameState->player1.posX = (gameState->player1.posX + gameState->player1.movementSpeed);
    }

    if (controller.isAnalog) {
        if (controller.leftThumbstick.position.x) {
            gameState->player1.posX = (gameState->player1.posX + (int32)(gameState->player1.movementSpeed * controller.leftThumbstick.position.x));
        }
        if (controller.leftThumbstick.position.y) {
            gameState->player1.posY = (gameState->player1.posY - (int32)(gameState->player1.movementSpeed * controller.leftThumbstick.position.y));
        }
    }

    // Safe bounds checking
    if (gameState->player1.posY < 0) {
        gameState->player1.posY = 0;
    }
    if (((int64)gameState->player1.posY + gameState->player1.height) > frameBuffer->height) {
        gameState->player1.posY = (int32)(frameBuffer->height - gameState->player1.height);
    }
    if (((int64)gameState->player1.posX + gameState->player1.width) > frameBuffer->width) {
        gameState->player1.posX = (int32)(frameBuffer->width - gameState->player1.width);
    }
    if (gameState->player1.posX < 0) {
        gameState->player1.posX = 0;
    }
}

internal_func void frameBufferWriteBackground(GameState *gameState, GameFrameBuffer *buffer, GameAudioBuffer *audioBuffer)
{
    writeRectangle(buffer, gameState->bgColour, buffer->height, buffer->width, 0, 0);
}

internal_func void frameBufferWritePlayer(GameState *gameState, GameFrameBuffer *buffer, GameAudioBuffer *audioBuffer)
{
    writeRectangle(buffer, 0xff00ff, gameState->player1.height, gameState->player1.width, gameState->player1.posY, gameState->player1.posX);
}

#if defined(HANDMADE_DEBUG_AUDIO)

internal_func void frameBufferWriteAudioDebug(GameState *gameState, GameFrameBuffer *buffer, GameAudioBuffer *audioBuffer)
{
    float32 coefficient = ((float32)buffer->width / (float32)audioBuffer->platformBufferSizeInBytes);

    // Audio buffer box
    {
        uint16 height = 100;
        uint16 width = (uint16)((float32)audioBuffer->platformBufferSizeInBytes * coefficient);
        uint32 yOffset = 100;
        writeRectangle(buffer, 0x3333ff, height, width, yOffset, 0);
    }

    // Play cursor (green)
    {
        uint16 height = 100;
        uint16 width = 10;
        uint32 yOffset = 100;
        uint32 xOffset = (uint32)((float32)audioBuffer->playCursorPosition * coefficient);
        writeRectangle(buffer, 0x669900, height, width, yOffset, xOffset);
    }

    // Write cursor + lock size (amount written) (red)
    {
        uint16 height = 100;
        uint32 width = (uint32)((float32)audioBuffer->lockSizeInBytes * coefficient);
        uint32 yOffset = 100;
        uint32 xOffset = (uint32)((float32)audioBuffer->writeCursorPosition * coefficient);
        writeRectangle(buffer, 0xcc0000, height, width, yOffset, xOffset);
    }
}

#endif

/**
 * Simple pixel loop. Note, no safety bounds checking
 * 
 */
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
    if ((noOfBytesToWrite <= 0) || (bytesPerSample <= 0)) {
        return audioBuffer;
    }

    uint32 noOfSamplesToWrite = (noOfBytesToWrite / bytesPerSample);

    if (audioBuffer->noOfSamplesToWrite != noOfSamplesToWrite) {

        // @TODO(JM) move the audio memory to the GameMemory object
        if (!audioBuffer->initialised) {
            audioBuffer->initialised = 1;
            audioBuffer->memory = memory->platformAllocateMemory(0, noOfBytesToWrite);
        } else {
            memory->platformFreeMemory(audioBuffer->memory);
            audioBuffer->memory = memory->platformAllocateMemory(0, noOfBytesToWrite);
        }
    }
    audioBuffer->bytesPerSample = bytesPerSample;
    audioBuffer->noOfSamplesToWrite = noOfSamplesToWrite;
    audioBuffer->platformBufferSizeInBytes = platformBufferSizeInBytes;

    return audioBuffer;
}