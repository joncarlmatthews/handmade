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

internal_func
inline int64 metresToPixels(World world, float32 metres) {
    float32 pixels = (world.pixelsPerMetre * metres);
    return (int64)pixels;
}

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    // Tilemaps
    Tilemap tileMap1 = { TILES_1 };
    Tilemap tileMap2 = { TILES_2 };
    Tilemap tileMap3 = { TILES_3 };
    Tilemap tileMap4 = { TILES_4 };
    Tilemap tileMap5 = { TILES_5 };
    Tilemap tileMap6 = { TILES_6 };

    // Add the tile maps to the tile map array
    Tilemap tilemaps[2][3] = {
        tileMap1, tileMap2, tileMap3,
        tileMap4, tileMap5, tileMap6,
    };

    World world = {
        40,
        1.4f,
        60,
        60,
        { tileMap1, tileMap2, tileMap3,
            tileMap4, tileMap5, tileMap6,},
    };

    /**
     * Game state initialisation
     */
    assert(sizeof(GameState) <= memory->permanentStorageSizeInBytes);

    GameState *gameState = (GameState *)memory->permanentStorage;

    if (!memory->initialised) {

        gameState->player1.position.x = (STARTING_TILEMAP_POS_X * world.tilemapTileWidth);
        gameState->player1.position.y = (STARTING_TILEMAP_POS_Y * world.tilemapTileHeight);
        gameState->player1.height = 1.3f;
        gameState->player1.width = ((float32)gameState->player1.height * 0.65f);
        gameState->player1.movementSpeed = .2f;
        gameState->player1.totalJumpMovement = 15.0f;

        CurrentTilemap currentTilemap = { 0 };
        currentTilemap.tilemap = &world.tilemaps[STARTING_WORLD_TILEMAP_INDEX_Y][STARTING_WORLD_TILEMAP_INDEX_X];
        currentTilemap.tilemapIndex = {
            STARTING_WORLD_TILEMAP_INDEX_X,
            STARTING_WORLD_TILEMAP_INDEX_Y
        };
        currentTilemap.tile = {
            STARTING_TILEMAP_POS_X,
            STARTING_TILEMAP_POS_Y
        };

        gameState->currentTilemap = currentTilemap;

        gameState->sineWave = { 0 };

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

    controllerHandlePlayer(gameState,
                            memory,
                            frameBuffer,
                            audioBuffer,
                            inputInstances[0],
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

    // Draw tiles
    for (uint32 row = 0; row < TILEMAP_SIZE_Y; row++){
        for (uint32 column = 0; column < TILEMAP_SIZE_X; column++){
            uint32 *tileState = ((uint32 *)gameState->currentTilemap.tilemap->tiles + ((row * TILEMAP_SIZE_X) + column));

            // NULL pointer check
            if (!tileState){
                continue;
            }

            Colour tileColour = { 0.85f, 0.85f, 0.85f };
            if (*tileState) {
                tileColour = { 0.349f, 0.349f, 0.349f };
            }

            if ((column == (uint32)gameState->currentTilemap.tile.x)
                && (row == (uint32)gameState->currentTilemap.tile.y)) {
                tileColour = { 1.0f, 1.0f, 0.701f };
            }

            uint32 tileXOffset = (world.tilemapTileWidth * column);
            uint32 tileYOffset = (world.tilemapTileHeight * row);
            writeRectangle(frameBuffer,
                            tileXOffset,
                            tileYOffset,
                            world.tilemapTileWidth,
                            world.tilemapTileHeight,
                            tileColour);
            
        }
    }

    // Player
    writeRectangle(frameBuffer,
                    gameState->player1.position.x,
                    gameState->player1.position.y,
                    metresToPixels(world, gameState->player1.width),
                    metresToPixels(world, gameState->player1.height),
                    { 0.301f, 0.156f, 0.0f });

    // Point visualisation
#ifdef HANDMADE_DEBUG_TILE_POS
    writeRectangle(frameBuffer,
                    (int32)gameState->debug_xyPoints[2].x,
                    (int32)gameState->debug_xyPoints[2].y,
                    5,
                    5,
                    { 0.4f, 1.0f, 0.2f });
#endif

    // Mouse input testing
    if (inputInstances->mouse.leftClick.endedDown) {
        writeRectangle(frameBuffer,
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

internal_func
void controllerHandlePlayer(GameState *gameState,
                            GameMemory *memory,
                            GameFrameBuffer *frameBuffer,
                            GameAudioBuffer *audioBuffer,
                            GameInput gameInput,
                            uint8 selectedController,
                            World *world)
{
    GameControllerInput controller = gameInput.controllers[selectedController];

    // Basic player movement
    bool playerAttemptingMove = false;
    bool playerHasMoved = false;

    posXYInt playerNewPos = {0};
    playerNewPos.x = gameState->player1.position.x;
    playerNewPos.y = gameState->player1.position.y;

    if (controller.dPadLeft.endedDown) {
        playerAttemptingMove = true;
        playerNewPos.x += (int32)(gameInput.msPerFrame * (gameState->player1.movementSpeed * -1.0f));
    }

    if (controller.dPadRight.endedDown) {
        playerAttemptingMove = true;
        playerNewPos.x += (int32)(gameInput.msPerFrame * gameState->player1.movementSpeed);
    }

    if (controller.dPadUp.endedDown) {
        playerAttemptingMove = true;
        playerNewPos.y += (int32)(gameInput.msPerFrame * (gameState->player1.movementSpeed * -1.0f));
    }

    if (controller.dPadDown.endedDown) {
        playerAttemptingMove = true;
        playerNewPos.y += (int32)(gameInput.msPerFrame * gameState->player1.movementSpeed);
    }

    if (controller.isAnalog) {

        if (controller.leftThumbstick.position.x) {
            playerAttemptingMove = true;
            if (controller.leftThumbstick.position.x >= 0.0f) {
                playerNewPos.x += (int32)(gameInput.msPerFrame * gameState->player1.movementSpeed);
            }else{
                playerNewPos.x += (int32)(gameInput.msPerFrame * (gameState->player1.movementSpeed * -1.0f));
            }
        }

        if (controller.leftThumbstick.position.y) {
            playerAttemptingMove = true;
            if (controller.leftThumbstick.position.y >= 0.0f) {
                playerNewPos.y += (int32)(gameInput.msPerFrame * (gameState->player1.movementSpeed * -1.0f));
            }
            else {
                playerNewPos.y += (int32)(gameInput.msPerFrame * gameState->player1.movementSpeed);
            }
        }
    }

    if (playerAttemptingMove) {

        // Tilemap collision detection

        TilePoint middle = { 0 };
        getTilemapTile(&middle,
                        playerNewPos,
                        PLAYER_POINT_POS::MIDDLE,
                        gameState->player1,
                        *world,
                        gameState);

        TilePoint topMiddle = { 0 };
        getTilemapTile(&topMiddle,
                        playerNewPos,
                        PLAYER_POINT_POS::TOP_MIDDLE,
                        gameState->player1,
                        *world,
                        gameState);

        TilePoint bottomMiddle = {0};
        getTilemapTile(&bottomMiddle,
                        playerNewPos,
                        PLAYER_POINT_POS::BOTTOM_MIDDLE,
                        gameState->player1,
                        *world,
                        gameState);

        TilePoint bottomLeft = { 0 };
        getTilemapTile(&bottomLeft,
                        playerNewPos,
                        PLAYER_POINT_POS::BOTTOM_LEFT,
                        gameState->player1,
                        *world,
                        gameState);

        TilePoint bottomRight = { 0 };
        getTilemapTile(&bottomRight,
                        playerNewPos,
                        PLAYER_POINT_POS::BOTTOM_RIGHT,
                        gameState->player1,
                        *world,
                        gameState);

        TilePoint debugPoint = bottomMiddle;

        // @NOTE(JM) Bug, if move is invalid no move it taken
        if ((isWorldTileFree(*gameState, topMiddle))
                && (isWorldTileFree(*gameState, bottomMiddle))
                && (isWorldTileFree(*gameState, bottomLeft))
                && (isWorldTileFree(*gameState, bottomRight))) {

            if ((gameState->player1.position.x != playerNewPos.x)
                || (gameState->player1.position.y != playerNewPos.y)){

                gameState->player1.position.x = playerNewPos.x;
                gameState->player1.position.y = playerNewPos.y;

                playerHasMoved = true;
            }
        
            char buff[200] = {};
            sprintf_s(buff, sizeof(buff),
                        "Tilemaps Index x:%i y:%i Tile x:%i y:%i (%i). Plr Pos x:%i y:%i. Plr New Pos: x:%i y:%i.\n",
                        gameState->currentTilemap.tilemapIndex.x,
                        gameState->currentTilemap.tilemapIndex.y,
                        debugPoint.x,
                        debugPoint.y,
                        isWorldTileFree(*gameState, debugPoint),
                        gameState->player1.position.x,
                        gameState->player1.position.y,
                        playerNewPos.x,
                        playerNewPos.y);
            memory->DEBUG_platformLog(buff);

        }else{

            char buff[200] = {};
            sprintf_s(buff, sizeof(buff),
                        "Tilemaps Index x:%i y:%i Tile x:%i y:%i (%i). Plr Pos x:%i y:%i. Plr New Pos: x:%i y:%i. CANNOT MOVE\n",
                        gameState->currentTilemap.tilemapIndex.x,
                        gameState->currentTilemap.tilemapIndex.y,
                        debugPoint.x,
                        debugPoint.y,
                        isWorldTileFree(*gameState, debugPoint),
                        gameState->player1.position.x,
                        gameState->player1.position.y,
                        playerNewPos.x,
                        playerNewPos.y);
            memory->DEBUG_platformLog(buff);

        }

        if (playerHasMoved){

            getTilemapTile(&gameState->currentTilemap.tile,
                            gameState->player1.position,
                            PLAYER_POINT_POS::BOTTOM_MIDDLE,
                            gameState->player1,
                            *world,
                            gameState);

            setCurrentTilemap(world, gameState->currentTilemap.tile, gameState, memory);

            char buff[200] = {};
            sprintf_s(buff, sizeof(buff),
                        "== Active Tilemaps Index x:%i y:%i Active Tile x:%i y:%i (%i). Plr Pos x:%i y:%i. ==\n",
                        gameState->currentTilemap.tilemapIndex.x,
                        gameState->currentTilemap.tilemapIndex.y,
                        debugPoint.x,
                        debugPoint.y,
                        isWorldTileFree(*gameState, debugPoint),
                        gameState->player1.position.x,
                        gameState->player1.position.y);
            memory->DEBUG_platformLog(buff);
        }

    }

    // Temp jump code
#if 0
    if ((controller.down.endedDown) && (0 == gameState->player1.jumping)) {
        gameState->player1.jumping = 1;
        gameState->player1.jumpDuration = 20.0f;
        gameState->player1.jumpRunningFrameCtr = 0.0f;
        gameState->player1.jumpStartPos = gameState->player1.position.y;
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
        int32 newPos        = (int32)(gameState->player1.position.y - amtToMove);

        // Have we hit the top?
        if (newPos < 0) {
            gameState->player1.position.y = 0;
            gameState->player1.jumpDuration = (gameState->player1.jumpRunningFrameCtr / 2);
        } else {
            gameState->player1.position.y = newPos;
        }

        // Jump finished?
        if (jumpAngle >= 360.0f) {
            gameState->player1.jumping = 0;
            // Force reset the position.
            // @NOTE(JM) This is a bug if you land on a new level.
            gameState->player1.position.y = gameState->player1.jumpStartPos; 
        }
    }
#endif

    // Safe bounds checking
    #if 0
    if (gameState->player1.position.y < 0) {
        gameState->player1.position.y = 0;
    }
    if (((int64)gameState->player1.position.y + gameState->player1.height) > frameBuffer->height) {
        gameState->player1.position.y = (int32)(frameBuffer->height - gameState->player1.height);
    }
    if (((int64)gameState->player1.position.x + gameState->player1.width) > frameBuffer->width) {
        gameState->player1.position.x = (int32)(frameBuffer->width - gameState->player1.width);
    }
    if (gameState->player1.position.x < 0) {
        gameState->player1.position.x = 0;
    }
    #endif
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
 * Returns the active X and Y tile point index based off of a given pixel X and Y
 * 
 * @param tilePoint         The TilePoint object to write the X and Y into
 * @param playerPixelPos    The X and Y point to base the calculation on. This is the top left point
 * @param pointPos          The offset from the playerPixelPos X and Y to apply
 * @param player            The player object containing the height/width etc
 * @param world             The world object
 * @param gameState         The game state (Debug only)
*/
internal_func
void getTilemapTile(TilePoint *tilePoint,
                    posXYInt playerPixelPos,
                    PLAYER_POINT_POS pointPos,
                    Player player,
                    World world,
                    GameState *gameState)
{
    uint8 debugIndex    = 0;
    float32 pixelInset  = 6.0f;
    float32 x           = 0.0f;
    float32 y           = 0.0f;

    // Apply the offsets...
    switch (pointPos)
    {
    case PLAYER_POINT_POS::TOP_LEFT:
        x = (float32)playerPixelPos.x;
        y = (float32)playerPixelPos.y;
        debugIndex = 0;
        break;
    case PLAYER_POINT_POS::TOP_MIDDLE:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.width) / 2.0f);
        y = (float32)playerPixelPos.y;
        debugIndex = 0;
        break;
    case PLAYER_POINT_POS::MIDDLE:
        x = (float32)playerPixelPos.x + ((float32)metresToPixels(world, player.width) / 2.0f);
        y = (float32)playerPixelPos.y + ((float32)metresToPixels(world, player.height) / 2.0f);
        debugIndex = 1;
        break;
    case PLAYER_POINT_POS::BOTTOM_MIDDLE:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.width) / 2.0f);
        y = ((float32)playerPixelPos.y + (float32)metresToPixels(world, player.height) - pixelInset);
        debugIndex = 2;
        break;
    case PLAYER_POINT_POS::BOTTOM_RIGHT:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.width) - pixelInset);
        y = ((float32)playerPixelPos.y + (float32)metresToPixels(world, player.height) - pixelInset);
        debugIndex = 3;
        break;
    case PLAYER_POINT_POS::BOTTOM_LEFT:
        x = (float32)playerPixelPos.x;
        y = ((float32)playerPixelPos.y + (float32)metresToPixels(world, player.height) - pixelInset);
        debugIndex = 4;
        break;
    default:
        assert(!"Tile point position not yet supported");
        break;
    }

#ifdef HANDMADE_DEBUG_TILE_POS
    gameState->debug_xyPoints[debugIndex] = { x, y };
#endif

    tilePoint->x = (int8)floorf(x / (float32)world.tilemapTileWidth);
    tilePoint->y = (int8)floorf(y / (float32)world.tilemapTileHeight);
}

internal_func
void setCurrentTilemap(World *world, TilePoint point, GameState *gameState, GameMemory *memory)
{
    bool movingTilemap = false;

    // Moving left
    if (point.x < 0) {
        movingTilemap = true;
        gameState->currentTilemap.tilemap--;
        gameState->currentTilemap.tilemapIndex.x -= 1;
        gameState->player1.position.x = (int32)((TILEMAP_SIZE_X * world->tilemapTileWidth) - (metresToPixels(*world, gameState->player1.width) - (metresToPixels(*world, gameState->player1.width) / 2)));
    }

    // Moving right
    if (point.x >= TILEMAP_SIZE_X) {
        movingTilemap = true;
        gameState->currentTilemap.tilemap += 1;
        gameState->currentTilemap.tilemapIndex.x += 1;
        gameState->player1.position.x = (int32)((metresToPixels(*world, gameState->player1.width) / 2) *-1);
    }

    // Moving up
    if (point.y < 0) {
        movingTilemap = true;
        gameState->currentTilemap.tilemap -= WORLD_TILEMAP_COUNT_X;
        gameState->currentTilemap.tilemapIndex.y -= 1;
        gameState->player1.position.y = (int32)(((TILEMAP_SIZE_Y-1) * world->tilemapTileHeight));
    }

    // Moving down
    if (point.y >= TILEMAP_SIZE_Y) {
        movingTilemap = true;
        gameState->currentTilemap.tilemap += WORLD_TILEMAP_COUNT_X;
        gameState->currentTilemap.tilemapIndex.y += 1;
        gameState->player1.position.y = (int32)(metresToPixels(*world, gameState->player1.height) *-1);

        getTilemapTile(&gameState->currentTilemap.tile,
                           gameState->player1.position,
                           PLAYER_POINT_POS::BOTTOM_MIDDLE,
                           gameState->player1,
                           *world,
                           gameState);
    }

    if (movingTilemap){
        char buff[100] = { 0 };
        sprintf_s(buff, sizeof(buff), "*SWITCHING TILEMAP*\n");
        memory->DEBUG_platformLog(buff);
    }
}

internal_func
inline bool isWorldTileFree(GameState gameState, TilePoint point)
{
    Tilemap *tilemapToCheck = gameState.currentTilemap.tilemap;
    posXYInt currentTilemapIndex = gameState.currentTilemap.tilemapIndex;

    if (point.x < 0) {

        // Bounds checking. Already at the left-most edge?
        if (currentTilemapIndex.x == 0) {
            point.x = 0;
            return false;
        }

        point.x = (TILEMAP_SIZE_X -1);
        tilemapToCheck = (tilemapToCheck -1);
    }

    if (point.x >= TILEMAP_SIZE_X) {

        // Bounds checking. Already at the right-most edge?
        if (currentTilemapIndex.x == (WORLD_TILEMAP_COUNT_X - 1)) {
            point.x = (TILEMAP_SIZE_X - 1);
            return false;
        }

        point.x = 0;
        tilemapToCheck += 1;
    }

    if (point.y >= TILEMAP_SIZE_Y) {

        // Bounds checking. Already at the bottom edge?
        if (currentTilemapIndex.y == (WORLD_TILEMAP_COUNT_Y - 1)) {
            point.y = (TILEMAP_SIZE_Y - 1);
            return false;
        }

        point.y = 0;
        tilemapToCheck += WORLD_TILEMAP_COUNT_X;
    }

    if (point.y < 0) {

        // Bounds checking. Already at the top edge?
        if (currentTilemapIndex.y == 0) {
            point.y = 0;
            return false;
        }

        point.y = (TILEMAP_SIZE_Y - 1);
        tilemapToCheck -= WORLD_TILEMAP_COUNT_X;
    }

    uint tileNumber = (point.y * TILEMAP_SIZE_X) + point.x;

    uint32 *tileState = ((uint32*)tilemapToCheck->tiles + tileNumber);

    if (tileState) {
        if (*tileState) {
            return false;
        }
        return true;
    }

    return true;
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