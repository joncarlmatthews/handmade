// Wide-strings.
// Defines wchar_t
// @see https://www.cplusplus.com/reference/cwchar/
#include <wchar.h>

// Unicode characters.
// Defines char16_t, char32_t
// @see https://www.cplusplus.com/reference/cuchar/
#include <uchar.h>

// Common mathematical operations and transformations.
// Defines floor, M_PI
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

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorageSizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->initialised) {

        gameState->player1.posX = 60;
        gameState->player1.posY = 60;
        gameState->player1.height = 60;
        gameState->player1.width = 60;
        gameState->player1.totalJumpMovement = 15.0f;

        gameState->sineWave = { 0 };

        memory->initialised = true;
    }

    // Tilemaps
    TileMap tileMap1 = { 0 };
    uint32 tileMap1Tiles[TILEMAP_SIZE_Y][TILEMAP_SIZE_X] = TILES_1;
    tileMap1.tiles = (uint32*)tileMap1Tiles;

    TileMap tileMap2 = { 0 };
    uint32 tileMap2Tiles[TILEMAP_SIZE_Y][TILEMAP_SIZE_X] = TILES_2;
    tileMap2.tiles = (uint32*)tileMap2Tiles;

    TileMap tileMap3 = { 0 };
    uint32 tileMap3Tiles[TILEMAP_SIZE_Y][TILEMAP_SIZE_X] = TILES_2;
    tileMap3.tiles = (uint32*)tileMap3Tiles;

    TileMap tileMap4 = { 0 };
    uint32 tileMap4Tiles[TILEMAP_SIZE_Y][TILEMAP_SIZE_X] = TILES_2;
    tileMap4.tiles = (uint32*)tileMap4Tiles;

    TileMap tileMap5 = { 0 };
    uint32 tileMap5Tiles[TILEMAP_SIZE_Y][TILEMAP_SIZE_X] = TILES_2;
    tileMap5.tiles = (uint32*)tileMap5Tiles;

    TileMap tileMap6 = { 0 };
    uint32 tileMap6Tiles[TILEMAP_SIZE_Y][TILEMAP_SIZE_X] = TILES_2;
    tileMap6.tiles = (uint32*)tileMap6Tiles;

    // Add the tile maps to the tile map array
    TileMap tileMaps[2][3] = {
        tileMap1, tileMap2, tileMap3,
        tileMap4, tileMap5, tileMap6,
    };

    World world = {
        60,
        60,
        (TileMap*)&tileMaps,
    };

    // Set the currently active tilemap
    TileMap *currentTileMap = &tileMaps[0][0];

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
    uint8 userSelectedMainController = 1; // @TODO(JM) make this selectable through a UI

    controllerHandlePlayer(gameState,
                            memory,
                            frameBuffer,
                            audioBuffer,
                            inputInstances[0],
                            userSelectedMainController,
                            &world,
                            currentTileMap);

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

    // Draw tiles
    for (uint32 row = 0; row < TILEMAP_SIZE_Y; row++){
        for (uint32 column = 0; column < TILEMAP_SIZE_X; column++){
            uint32 *tileState = (currentTileMap->tiles + ((row * TILEMAP_SIZE_X) + column));
            if (tileState){
                Colour tileColour = { 0.85f, 0.85f, 0.85f };
                if (*tileState) {
                    tileColour = { 0.349f, 0.349f, 0.349f };
                }
                uint32 tileXOffset = (world.tileMapWidth * column);
                uint32 tileYOffset = (world.tileMapHeight * row);
                writeRectangle(frameBuffer, tileXOffset, tileYOffset, world.tileMapWidth, world.tileMapHeight, tileColour);
            }
            
        }
    }

    // Player
    writeRectangle(frameBuffer, gameState->player1.posX, gameState->player1.posY, gameState->player1.width, gameState->player1.height, { 0.9f, 0.0f, 0.0f });

    // Mouse input testing
    if (inputInstances->mouse.leftClick.endedDown) {
        writeRectangle(frameBuffer, inputInstances->mouse.position.x, inputInstances->mouse.position.y, 50, 50, { 0.5f, 0.0f, 0.5f });
    }

#if defined(HANDMADE_DEBUG_AUDIO)
    frameBufferWriteAudioDebug(gameState, frameBuffer, audioBuffer);
#endif

}

internal_func
inline bool canMove(TileMap tileMap, TilePoint point)
{
    uint tilePos = (point.y * TILEMAP_SIZE_X) + point.x;

    uint32 *tileState = (tileMap.tiles + tilePos);

    if (tileState){
        if (*tileState){
            return false;
        }
        return true;
    }

    return true;
}

internal_func
void controllerHandlePlayer(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput gameInput,
                            uint8 selectedController,
                            World *world,
                            TileMap *tileMap)
{
    GameControllerInput controller = gameInput.controllers[selectedController];

    // Basic player movement
    bool playerMoving = false;
    float32 speed = 0.2f;

    posXYInt playerNewPos = {0};
    playerNewPos.x = gameState->player1.posX;
    playerNewPos.y = gameState->player1.posY;

    if (controller.dPadLeft.endedDown) {
        playerMoving = true;
        playerNewPos.x += (int32)(gameInput.msPerFrame * (speed * -1));
    }

    if (controller.dPadRight.endedDown) {
        playerMoving = true;
        playerNewPos.x += (int32)(gameInput.msPerFrame * speed);
    }

    if (controller.dPadUp.endedDown) {
        playerMoving = true;
        playerNewPos.y += (int32)(gameInput.msPerFrame * (speed * -1));
    }

    if (controller.dPadDown.endedDown) {
        playerMoving = true;
        playerNewPos.y += (int32)(gameInput.msPerFrame * speed);
    }

    if (controller.isAnalog) {

        if (controller.leftThumbstick.position.x) {
            playerMoving = true;
            if (controller.leftThumbstick.position.x >= 0.0f) {
                playerNewPos.x += (int32)(gameInput.msPerFrame * speed);
            }else{
                playerNewPos.x += (int32)(gameInput.msPerFrame * (speed * -1));
            }
        }

        if (controller.leftThumbstick.position.y) {
            playerMoving = true;
            if (controller.leftThumbstick.position.y >= 0.0f) {
                playerNewPos.y += (int32)(gameInput.msPerFrame * (speed * -1));
            }
            else {
                playerNewPos.y += (int32)(gameInput.msPerFrame * speed);
            }
        }
    }

    // Tilemap collision detection
    TilePoint bottomMiddle = {0};
    setTilePoint(&bottomMiddle, TILE_POINT_POS::TILE_POINT_BOTTOM_MIDDLE, *world, gameState->player1, playerNewPos);

    TilePoint bottomLeft = { 0 };
    setTilePoint(&bottomLeft, TILE_POINT_POS::TILE_POINT_BOTTOM_LEFT, *world, gameState->player1, playerNewPos);

    TilePoint bottomRight = { 0 };
    setTilePoint(&bottomRight, TILE_POINT_POS::TILE_POINT_BOTTOM_RIGHT, *world, gameState->player1, playerNewPos);

#if 0
    TilePoint bottomLeft = {
        (uint8)((float32)playerNewPos.x / (float32)world->tileMapHeight),
        (uint8)(((float32)playerNewPos.y + (float32)gameState->player1.height - 6.0f) / (float32)world->tileMapHeight)
    };

    TilePoint bottomRight = {
        (uint8)(((float32)playerNewPos.x + (float32)gameState->player1.width - 6.0f) / (float32)world->tileMapWidth),
        (uint8)(((float32)playerNewPos.y + (float32)gameState->player1.height - 6.0f) / (float32)world->tileMapHeight)
    };

    Point topMiddle = {
        (uint8)(((float32)playerNewPos.x + (float32)gameState->player1.width / 2.0f) / (float32)world->tileMapWidth),
        (uint8)((float32)playerNewPos.y / (float32)world->tileMapHeight)
    };

    Point topLeft = {
        (uint8)((float32)playerNewPos.x / (float32)world->tileMapWidth),
        (uint8)((float32)playerNewPos.y / (float32)world->tileMapHeight)
    };

    Point topRight = {
        (uint8)(((float32)playerNewPos.x + (float32)gameState->player1.width) / (float32)world->tileMapWidth),
        (uint8)((float32)playerNewPos.y / (float32)world->tileMapHeight),
    };


    if (playerMoving){
        TilePoint debugPoint = bottomMiddle;
        char buff[100] = {};
        sprintf_s(buff, sizeof(buff), "Tile x:%i y:%i (%i)\n", debugPoint.x, debugPoint.y, canMove(*tileMap, debugPoint));
        memory->DEBUG_platformLog(buff);
    }
#endif

    // @NOTE(JM) Bug, if move is invalid no move it taken
    if ( (canMove(*tileMap, bottomMiddle))
            && (canMove(*tileMap, bottomLeft))
            && (canMove(*tileMap, bottomRight))) {
        gameState->player1.posX = playerNewPos.x;
        gameState->player1.posY = playerNewPos.y;
    }

    // Temp jump code
#if 0
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
#endif

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

    uint16* audioSample = (uint16*)audioBuffer->memory;

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

/**
 * Simple pixel loop.
 *
 * x = along the corridoor, y = down the stairs.
 *
 * Therefore x is concerned with the screen buffer's width,
 * and y is concerned with the screen buffer's height.
 * 
 */
internal_func
void writeRectangle(GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    Colour colour)
{
    // Bounds checking
    if (xOffset >= buffer->width) {
        return;
    }

    if (yOffset >= buffer->height) {
        return;
    }

    // Min x
    if (xOffset < 0) {
        width = (width - (xOffset*-1));
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

    if (maxX > buffer->width) {
        maxX = (buffer->width - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    int64 maxY = (yOffset + height);

    if (maxY > buffer->height) {
        maxY = (buffer->height - yOffset);
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
    uint32 *row = (uint32 *)buffer->memory;

    // Move down to starting row
    row = (row + (buffer->width * yOffset));

    // Move in from left to starting position
    row = (row + xOffset);

    // Down (rows)
    for (int64 i = 0; i < height; i++) {

        // Accross (columns)
        uint32 *pixel = (uint32 *)row;
        for (int64 x = 0; x < width; x++) {
            *pixel = hexColour;
            pixel = (pixel + 1);
        }

        // Move down one entire row
        row = (row + buffer->width);
    }
}

internal_func
void setTilePoint(TilePoint* tilePoint, TILE_POINT_POS pointPos, World world, Player player, posXYInt playerNewPos)
{
    float32 pixelInset = 6.0f;
    switch (pointPos)
    {
    case TILE_POINT_POS::TILE_POINT_BOTTOM_MIDDLE:
        tilePoint->x = (uint8)(((float32)playerNewPos.x + (float32)player.width / 2.0f) / (float32)world.tileMapWidth);
        tilePoint->y = (uint8)(((float32)playerNewPos.y + (float32)player.height - pixelInset) / (float32)world.tileMapHeight);
        break;
    case TILE_POINT_POS::TILE_POINT_BOTTOM_RIGHT:
        tilePoint->x = (uint8)(((float32)playerNewPos.x + (float32)player.width - pixelInset) / (float32)world.tileMapWidth);
        tilePoint->y = (uint8)(((float32)playerNewPos.y + (float32)player.height - pixelInset) / (float32)world.tileMapHeight);
        break;
    case TILE_POINT_POS::TILE_POINT_BOTTOM_LEFT:
        tilePoint->x = (uint8)((float32)playerNewPos.x / (float32)world.tileMapHeight);
        tilePoint->y = (uint8)(((float32)playerNewPos.y + (float32)player.height - pixelInset) / (float32)world.tileMapHeight);
        break;
    default:
        assert(!"Tile point position not yet supported");
        break;
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