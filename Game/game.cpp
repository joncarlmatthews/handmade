// Wide-strings.
// Defines wchar_t
// @see https://www.cplusplus.com/reference/cwchar/
#include <wchar.h>

// Unicode characters.
// Defines char16_t, char32_t
// @see https://www.cplusplus.com/reference/cuchar/
#include <uchar.h>

// Common mathematical operations and transformations.
// Defines floor, floorf, M_PI
// @see https://www.cplusplus.com/reference/cmath/
#define _USE_MATH_DEFINES
#include <math.h>

// Common input/output operations.
// Defines: sprintf_s
// @see https://www.cplusplus.com/reference/cstdio/
#include <stdio.h>

#include "types.h"
#include "utility.h"
#include "game.h"
#include "world.h"
#include "player.h"

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    // Init the World
    World world = { 0 };
    initWorld(&world,
                WORLD_PIXELS_PER_METER,
                TILE_DIMENSIONS_METERS,
                WORLD_TOTAL_TILE_DIMENSIONS,
                WORLD_TILE_CHUNK_DIMENSIONS);

    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorageSizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->initialised) {       

        // @NOTE(JM) not sure how to allocate this on the heap, as this array is
        // too large for the stack...
        uint8 origTiles[WORLD_TOTAL_TILE_DIMENSIONS][WORLD_TOTAL_TILE_DIMENSIONS] = ALL_TILES;

        // Copy the tiles into the world tiles, making it so that the Y axis
        // goes up
        uint32 worldY = 0;
        for (int32 y = (WORLD_TOTAL_TILE_DIMENSIONS-1); y >= 0; y--){
            for (uint x = 0; x < WORLD_TOTAL_TILE_DIMENSIONS; x++) {
                gameState->worldTiles[worldY][x] = (uint32)origTiles[y][x];
            }
            worldY++;
        }

        // Character attributes
        gameState->player1.heightMeters  = PLAYER_HEIGHT_METERS;
        gameState->player1.widthMeters   = ((float32)gameState->player1.heightMeters * 0.65f);
        gameState->player1.heightPx  = (int16)metersToPixels(world, gameState->player1.heightMeters);
        gameState->player1.widthPx   = (int16)metersToPixels(world, gameState->player1.widthMeters);

        // Movement speed (assume always running)
        // https://www.calculateme.com/speed/kilometers-per-hour/to-meters-per-second/13
        gameState->player1.movementSpeedMPS = PLAYER_SPEED;
        gameState->player1.totalJumpMovement = 15.0f;

        // Initial character starting position. Start the player in the middle
        // of screen
        gameState->player1.fixedPosition.x = (frameBuffer->widthPx / 2);
        gameState->player1.fixedPosition.y = (frameBuffer->heightPx / 2);
        gameState->player1.absolutePosition.x = gameState->player1.fixedPosition.x;
        gameState->player1.absolutePosition.y = gameState->player1.fixedPosition.y;
       
        // Calculate the currently active tile based on player1's position and
        // write it to the World Position data
        setWorldPosition(gameState, world, frameBuffer);

        //gameState->cameraPositionPx.x = 0;
        //gameState->cameraPositionPx.y = 0;

        memory->initialised = true;
    }

    /**
    * Handle controller input...
    */

    // Main controller loop
    // @TODO(JM) Support for multiple controllers.See below for single controller support.
    /*
    for (uint8 i = 0; i < controllerCounts->connectedControllers; i++){

        if (!inputInstances->controllers[i].isConnected) {
            continue;
        }
    }
    */

    // Which controller has the user selected as the main controller?
    uint8 userSelectedMainController = 0; // @TODO(JM) make this selectable through a UI

    playerHandleMovement(gameState,
                            memory,
                            frameBuffer,
                            audioBuffer,
                            &inputInstances[0],
                            userSelectedMainController,
                            &world);

    /**
     * Audio stuff...
     */

#ifdef HANDMADE_DEBUG_AUDIO
    audioBufferWriteSineWave(gameState, audioBuffer);
#endif // HANDMADE_DEBUG_AUDIO
   

    /**
     * Write the frame buffer...
     * 
     */

    // Draw the tile map.
    // @NOTE(JM) Drawing this pixel by pixel, this is incapable of hitting 60fps
    // @TODO(JM) Optimise this!
    for (uint32 y = 0; y < frameBuffer->heightPx; y++) {
        for (uint32 x = 0; x < frameBuffer->widthPx; x++) {

            uint32 tilePosY = modulo(((y + gameState->cameraPositionPx.y) / world.tileHeightPx), world.totalTileDimensions);
            uint32 tilePosX = modulo(((x + gameState->cameraPositionPx.x) / world.tileWidthPx), world.totalTileDimensions);

            uint32 *tileValue = ((uint32*)gameState->worldTiles + ((tilePosY * world.totalTileDimensions) + tilePosX));

            // Null pointer check
            if (!tileValue) {
                continue;
            }

            Colour pixelColour = {};
            setTileColour(&pixelColour, *tileValue);

            if ( tilePosX == gameState->worldPosition.tileIndex.x
                && tilePosY == gameState->worldPosition.tileIndex.y ){
                pixelColour.r = 0.0f;
                pixelColour.g = 0.6f;
                pixelColour.b = 0.2f;
            }

            writeRectangle(world, frameBuffer, x, y, 1, 1, pixelColour);
        }
    }

    // Draw player
    writeRectangle(world,
                    frameBuffer,
                    gameState->player1.fixedPosition.x,
                    gameState->player1.fixedPosition.y,
                    gameState->player1.widthPx,
                    gameState->player1.heightPx,
                    { 0.301f, 0.156f, 0.0f });

    // Mouse input testing
    if (inputInstances->mouse.leftClick.endedDown) {
        writeRectangle(world,
                        frameBuffer,
                        inputInstances->mouse.position.x,
                        inputInstances->mouse.position.y,
                        50,
                        50,
                        { 0.5f, 0.0f, 0.5f });
    }

#if defined(HANDMADE_DEBUG_AUDIO)
    frameBufferWriteAudioDebug(gameState, frameBuffer, audioBuffer);
#endif

}

void playerHandleMovement(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput *gameInput,
                            uint8 selectedController,
                            World *world)
{
    GameControllerInput controller = gameInput->controllers[selectedController];

    // Basic player movement...

    bool playerAttemptingMove = false;

    // Normalise pixel movement regardless of framerate
    // @NOTE(JM) The truncated fractions cause issues with different framerates.
    // Not sure how to resolve at this point.
    float32 pixelsPerSecond = (world->pixelsPerMetre * gameState->player1.movementSpeedMPS);
    float32 pixelsPerFrame = (pixelsPerSecond / gameInput->fps);

    // Ensure the player can at least move!
    if (pixelsPerFrame < 1.0f){
        pixelsPerFrame = 1.0f;
    }

#if 0
    {
        char buff[400] = {};
        sprintf_s(buff, sizeof(buff),
    "MS per frame: %f. \
FPS: %f. \
Pixels per second: %f. \
Pixels per frame: %f\n",
gameInput->msPerFrame,
gameInput->fps,
pixelsPerSecond,
pixelsPerFrame);
        memory->DEBUG_platformLog(buff);
    }
#endif

    xyint playerNewPosTmp = {0};
    playerNewPosTmp.x = gameState->player1.absolutePosition.x;
    playerNewPosTmp.y = gameState->player1.absolutePosition.y;


    if (controller.dPadLeft.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.x += (int32)(pixelsPerFrame * -1.0f);
    }else if (controller.dPadRight.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.x += (int32)pixelsPerFrame;
    }else if (controller.dPadUp.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.y += (int32)pixelsPerFrame;
    }else if (controller.dPadDown.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.y += (int32)(pixelsPerFrame * -1.0f);
    }

    if (controller.isAnalog) {

        if (controller.leftThumbstick.position.x) {
            playerAttemptingMove = true;
            if (controller.leftThumbstick.position.x >= 0.0f) {
                playerNewPosTmp.x += (int32)pixelsPerFrame;
            }else{
                playerNewPosTmp.x += (int32)(pixelsPerFrame * -1.0f);
            }
        }else if (controller.leftThumbstick.position.y) {
            playerAttemptingMove = true;
            if (controller.leftThumbstick.position.y >= 0.0f) {
                playerNewPosTmp.y += (int32)(pixelsPerFrame * -1.0f);
            }
            else {
                playerNewPosTmp.y += (int32)pixelsPerFrame;
            }
        }
    }

    if (playerAttemptingMove) {

        xyuint playerNewPos = { 0 };
        playerNewPos.x = modulo(playerNewPosTmp.x, (world->tileWidthPx * world->totalTileDimensions));
        playerNewPos.y = modulo(playerNewPosTmp.y, (world->tileHeightPx * world->totalTileDimensions));

        // Player movement direction
        uint32 movedUp = 0;
        uint32 movedDown = 0;
        uint32 movedLeft = 0;
        uint32 movedRight = 0;
        uint32 lastMoveDirections = 0;

        if (playerNewPos.y > gameState->player1.absolutePosition.y) {
            movedUp += (1 << 0); // UP
        }
        if (playerNewPos.y < gameState->player1.absolutePosition.y) {
            movedDown += (1 << 1); // DOWN
        }
        if (playerNewPos.x < gameState->player1.absolutePosition.x) {
            movedLeft += (1 << 2); // LEFT
        }
        if (playerNewPos.x > gameState->player1.absolutePosition.x) {
            movedRight += (1 << 3); // RIGHT
        }

        lastMoveDirections = (movedUp | movedDown | movedLeft | movedRight);

        // Tilemap collision detection
        PlayerPositionData middle;
        getPositionDataForPlayer(&middle,
                                    playerNewPos,
                                    PLAYER_POINT_POS::MIDDLE,
                                    gameState->player1,
                                    *world);

        PlayerPositionData bottomLeft;
        getPositionDataForPlayer(&bottomLeft,
                                    playerNewPos,
                                    PLAYER_POINT_POS::BOTTOM_LEFT,
                                    gameState->player1,
                                    *world);

        PlayerPositionData bottomRight;
        getPositionDataForPlayer(&bottomRight,
                                    playerNewPos,
                                    PLAYER_POINT_POS::BOTTOM_RIGHT,
                                    gameState->player1,
                                    *world);

#ifdef HANDMADE_DEBUG_TILE_POS
        // Visualisation
        PlayerPositionData debugPoint = middle;
#endif
        
        // Can the move to the new tile be taken?
        // @NOTE(JM) bug where rounding means player doesnt get a close as
        // possible to certain tiles when a move is invalid
        if ((false == (isWorldTileFree(*world, gameState, &middle)))
                || (false == (isWorldTileFree(*world, gameState, &bottomLeft)))
                || (false == (isWorldTileFree(*world, gameState, &bottomRight)))) {

#if 0
#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[400] = {};
            sprintf_s(buff, sizeof(buff),
    "CANNOT MOVE. \
Plr Proposed World Pos: x:%i y:%i. \
Plr Proposed World Tile: x:%i y:%i. \
Plr Actual World Pos: x:%i y:%i. \
playerNewPos.x,
playerNewPos.y,
debugPoint.activeTile.tileIndex.x,
debugPoint.activeTile.tileIndex.y,
gameState->player1.absolutePosition.x,
gameState->player1.absolutePosition.y);
            memory->DEBUG_platformLog(buff);
#endif
#endif

        } else {

            // ...yes, the move to the new tile can be taken

            // Sense check that the player actually moved
            if ( (gameState->player1.absolutePosition.x != playerNewPos.x)
                    || (gameState->player1.absolutePosition.y != playerNewPos.y) ){

                gameState->player1.absolutePosition.x = playerNewPos.x;
                gameState->player1.absolutePosition.y = playerNewPos.y;
                gameState->player1.lastMoveDirections = lastMoveDirections;

                setWorldPosition(gameState, *world, frameBuffer);

#ifdef HANDMADE_DEBUG_TILE_POS
                char buff[500] = {};
                sprintf_s(buff, sizeof(buff),
    "MOVED. \
Plr World Pos x:%i y:%i. \
World Tile x:%i y:%i. \
Chunk Index x:%i y:%i. \
Camera pos x:%i y:%i. \
\n",
gameState->player1.absolutePosition.x, gameState->player1.absolutePosition.y,
gameState->worldPosition.tileIndex.x, gameState->worldPosition.tileIndex.y,
gameState->worldPosition.chunkIndex.x, gameState->worldPosition.chunkIndex.y,
gameState->cameraPositionPx.x, gameState->cameraPositionPx.y
);
                memory->DEBUG_platformLog(buff);
#endif
            }

        }

    } // player attempting move

    // Temp jump code
#if 0
    if ((controller.down.endedDown) && (0 == gameState->player1.jumping)) {
        gameState->player1.jumping = 1;
        gameState->player1.jumpDuration = 20.0f;
        gameState->player1.jumpRunningFrameCtr = 0.0f;
        gameState->player1.jumpStartPos = gameState->player1.absolutePosition.y;
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
        int32 newPos        = (int32)(gameState->player1.absolutePosition.y - amtToMove);

        // Have we hit the top?
        if (newPos < 0) {
            gameState->player1.absolutePosition.y = 0;
            gameState->player1.jumpDuration = (gameState->player1.jumpRunningFrameCtr / 2);
        } else {
            gameState->player1.absolutePosition.y = newPos;
        }

        // Jump finished?
        if (jumpAngle >= 360.0f) {
            gameState->player1.jumping = 0;
            // Force reset the absolutePosition.
            // @NOTE(JM) This is a bug if you land on a new level.
            gameState->player1.absolutePosition.y = gameState->player1.jumpStartPos; 
        }
    }
#endif

}

#if defined(HANDMADE_DEBUG_AUDIO)

internal_func void frameBufferWriteAudioDebug(GameState *gameState, GameFrameBuffer *buffer, GameAudioBuffer *audioBuffer)
{
    float32 coefficient = ((float32)buffer->widthPx / (float32)audioBuffer->platformBufferSizeInBytes);

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
 * Simple pixel loop.
 *
 * x = along the corridoor, y = up the stairs.
 *
 * Therefore x is concerned with the screen buffer's width,
 * and y is concerned with the screen buffer's height.
 * 
 */
internal_func
void writeRectangle(World world,
                    GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    Colour colour)
{
    // Bounds checking
    if (xOffset >= buffer->widthPx) {
        return;
    }

    if (yOffset >= buffer->heightPx) {
        return;
    }

    // Min x
    if (xOffset < 0) {
        width = (width - (xOffset * -1));
        if (width <= 0) {
            return;
        }
        xOffset = 0;
    }

    // Min y
    if (yOffset < 0) {
        height = (height - (yOffset * -1));
        if (height <= 0) {
            return;
        }
        yOffset = 0;
    }

    // Max x
    int64 maxX = (xOffset + width);

    if (maxX > buffer->widthPx) {
        maxX = (buffer->widthPx - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    int64 maxY = (yOffset + height);

    if (maxY > buffer->heightPx) {
        maxY = (buffer->heightPx - yOffset);
        if (height > maxY) {
            height = maxY;
        }
    }

    // Set the colour
    uint32 alpha    = ((uint32)(255.0f * colour.a) << 24);
    uint32 red      = ((uint32)(255.0f * colour.r) << 16);
    uint32 green    = ((uint32)(255.0f * colour.g) << 8);
    uint32 blue     = ((uint32)(255.0f * colour.b) << 0);

    uint32 hexColour = (alpha | red | green | blue);

    // Write the memory
    uint32 *row = (uint32*)buffer->memory;

    // Move to last row as starting position (bottom left of axis)
    row = (row + ((buffer->widthPx * buffer->heightPx) - buffer->widthPx));

    // Move up to starting row
    row = (row - (buffer->widthPx * yOffset));

    // Move in from left to starting absolutePosition
    row = (row + xOffset);

    // Up (rows)
    for (int64 i = 0; i < height; i++) {

        // Accross (columns)
        uint32 *pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            *pixel = hexColour;
            pixel = (pixel + 1);
        }

        // Move up one entire row
        row = (row - buffer->widthPx);
    }
}

internal_func
void audioBufferWriteSineWave(GameState* gameState, GameAudioBuffer* audioBuffer)
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

    uint16 *audioSample = (uint16*)audioBuffer->memory;

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

EXTERN_DLL_EXPORT GAME_INIT_FRAME_BUFFER(gameInitFrameBuffer)
{
    frameBuffer->heightPx = height;
    frameBuffer->widthPx = width;
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
            audioBuffer->memory = memory->platformAllocateMemory(thread, 0, noOfBytesToWrite);
        } else {
            memory->platformFreeMemory(thread, audioBuffer->memory);
            audioBuffer->memory = memory->platformAllocateMemory(thread, 0, noOfBytesToWrite);
        }
    }
    audioBuffer->bytesPerSample = bytesPerSample;
    audioBuffer->noOfSamplesToWrite = noOfSamplesToWrite;
    audioBuffer->platformBufferSizeInBytes = platformBufferSizeInBytes;

    return audioBuffer;
}