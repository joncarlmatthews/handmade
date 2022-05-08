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

#if 0
internal_func
CurrentTileChunk setCurrentTileChunk(World world,
                                        int32 absolutePositionX,
                                        int32 absolutePositionY)
{
    CurrentTileChunk currentTileChunk = { 0 };
    currentTileChunk.chunkIndex.x = absolutePositionX >> world.tileChunkShift;
    currentTileChunk.chunkIndex.y = absolutePositionY >> world.tileChunkShift;

    currentTileChunk.tileIndex.x = absolutePositionX & world.tileChunkMask;
    currentTileChunk.tileIndex.y = absolutePositionX & world.tileChunkMask;

    return currentTileChunk;
}
#endif

EXTERN_DLL_EXPORT GAME_UPDATE(gameUpdate)
{
    // Init the World
    World world = { 0 };
    initWorld(*frameBuffer, &world, 1.0f, 60);

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
        for (uint y = (WORLD_TOTAL_TILE_DIMENSIONS-1); y > 0; y--){
            for (uint x = 0; x < WORLD_TOTAL_TILE_DIMENSIONS; x++) {
                gameState->worldTiles[worldY][x] = (uint32)origTiles[y][x];
            }
            worldY++;
        }

        // Character attributes
        gameState->player1.heightMetres  = (world.tileHeightMetres * 0.7f);
        gameState->player1.widthMetres   = ((float32)gameState->player1.heightMetres * 0.65f);
        gameState->player1.height  = (int16)metresToPixels(world, gameState->player1.heightMetres);
        gameState->player1.width   = (int16)metresToPixels(world, gameState->player1.widthMetres);

        gameState->player1.movementSpeedMPS = 2.8f; // Running
        gameState->player1.totalJumpMovement = 15.0f;

        // Character position
        gameState->player1.relativePosition.x = (5 * world.tileWidth);
        gameState->player1.relativePosition.y = (5 * world.tileHeight);

        gameState->player1.absolutePosition.x = gameState->player1.relativePosition.x;
        gameState->player1.absolutePosition.y = gameState->player1.relativePosition.y;

        // Initial starting point for the chunk tiles.
        gameState->worldPosition.chunkIndexX = 0;
        gameState->worldPosition.chunkIndexY = 0;

        gameState->worldPosition.chunkTiles
            = &gameState->worldTiles[gameState->worldPosition.chunkIndexX][gameState->worldPosition.chunkIndexY];

        // Set the world's current absolute tile position data based off of the
        // player's initial absolute x and y position.
        setTilePositionForPlayer(&gameState->worldPosition.absoluteTile,
                                    gameState->player1.absolutePosition,
                                    PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                    gameState->player1,
                                    world);

        setTilePositionForPlayer(&gameState->worldPosition.worldTileBottomLeft,
                                    gameState->player1.absolutePosition,
                                    PLAYER_POINT_POS::BOTTOM_LEFT,
                                    gameState->player1,
                                    world);

        setTilePositionForPlayer(&gameState->worldPosition.worldTileBottomMiddle,
                                    gameState->player1.absolutePosition,
                                    PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                    gameState->player1,
                                    world);

        setTilePositionForPlayer(&gameState->worldPosition.worldTileBottomRight,
                                    gameState->player1.absolutePosition,
                                    PLAYER_POINT_POS::BOTTOM_RIGHT,
                                    gameState->player1,
                                    world);

        // Set the world's current chunk relative tile position data based off
        // of the player's initial relative x and y position.
        setTilePositionForPlayer(&gameState->worldPosition.chunkTile,
                                    gameState->player1.relativePosition,
                                    PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                    gameState->player1,
                                    world);

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
    for (uint32 row = 0; row < world.tileChunkDimensions; row++){ // y
        for (uint32 column = 0; column < world.tileChunkDimensions; column++){ // x

            uint32 realX = modulo((column + gameState->worldPosition.chunkIndexX), world.tileDimensions);
            uint32 realY = modulo((row + gameState->worldPosition.chunkIndexY), world.tileDimensions);

            // Tile pointer
            uint32 *tileState = &gameState->worldTiles[realY][realX];

            // NULL pointer check
            if (!tileState){
                continue;
            }

            Colour tileColour = { 0.85f, 0.85f, 0.85f };

            // Wall tile?
            if (*tileState) {
                tileColour = { 0.349f, 0.349f, 0.349f };
            }

#ifdef HANDMADE_DEBUG_TILE_POS
            // Visualisation of the currently active tile
            if ((column == (uint32)gameState->worldPosition.chunkTile.x)
                && (row == (uint32)gameState->worldPosition.chunkTile.y)) {
                tileColour = { 1.0f, 1.0f, 0.701f };
            }
#endif

            uint32 tileXOffset = (world.tileWidth * column);
            uint32 tileYOffset = (world.tileHeight * row);

            writeRectangle(world,
                            frameBuffer,
                            tileXOffset,
                            tileYOffset,
                            world.tileWidth,
                            world.tileHeight,
                            tileColour,
                            true);
            
        }
    }

    // Player
    writeRectangle(world,
                    frameBuffer,
                    gameState->player1.relativePosition.x,
                    gameState->player1.relativePosition.y,
                    metresToPixels(world, gameState->player1.widthMetres),
                    metresToPixels(world, gameState->player1.heightMetres),
                    { 0.301f, 0.156f, 0.0f },
                    true);

    // Point visualisation
#ifdef HANDMADE_DEBUG_TILE_POS
    TilePosition pointVisulisation;
    setTilePositionForPlayer(&pointVisulisation,
                    gameState->player1.relativePosition,
                    PLAYER_POINT_POS::BOTTOM_MIDDLE,
                    gameState->player1,
                    world);

    writeRectangle(world,
                    frameBuffer,
                    pointVisulisation.pointPixelPositionAbs.x,
                    pointVisulisation.pointPixelPositionAbs.y,
                    1,
                    1,
                    { 0.4f, 1.0f, 0.2f },
                    true);
#endif

    // Mouse input testing
    if (inputInstances->mouse.leftClick.endedDown) {
        writeRectangle(world,
                        frameBuffer,
                        inputInstances->mouse.position.x,
                        inputInstances->mouse.position.y,
                        50,
                        50,
                        { 0.5f, 0.0f, 0.5f },
                        true);
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

    float32 pixelsPerSecond = (world->pixelsPerMetre * gameState->player1.movementSpeedMPS);
    float32 pixelsPerFrame = ceilf(pixelsPerSecond / (int32)ceilf((1000.0f / gameInput.msPerFrame)));

    posXYInt playerNewPosTmp = {0};
    playerNewPosTmp.x = gameState->player1.absolutePosition.x;
    playerNewPosTmp.y = gameState->player1.absolutePosition.y;

    if (controller.dPadLeft.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.x += (int32)(pixelsPerFrame * -1.0f);
    }

    if (controller.dPadRight.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.x += (int32)pixelsPerFrame;
    }

    if (controller.dPadUp.endedDown) {
        playerAttemptingMove = true;
        playerNewPosTmp.y += (int32)pixelsPerFrame;
    }

    if (controller.dPadDown.endedDown) {
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
        }

        if (controller.leftThumbstick.position.y) {
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

        posXYUInt playerNewPos = { 0 };
        playerNewPos.x = modulo(playerNewPosTmp.x, (world->tileWidth * WORLD_TOTAL_TILE_DIMENSIONS));
        playerNewPos.y = modulo(playerNewPosTmp.y, (world->tileWidth * WORLD_TOTAL_TILE_DIMENSIONS));

        uint32 movedUp = 0;
        uint32 movedDown = 0;
        uint32 movedLeft = 0;
        uint32 movedRight = 0;

        // Tilemap collision detection
        TilePosition middle;
        setTilePositionForPlayer(&middle,
                                    playerNewPos,
                                    PLAYER_POINT_POS::MIDDLE,
                                    gameState->player1,
                                    *world);

        TilePosition bottomMiddle;
        setTilePositionForPlayer(&bottomMiddle,
                                    playerNewPos,
                                    PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                    gameState->player1,
                                    *world);

#ifdef HANDMADE_DEBUG_TILE_POS
        // Visualisation
        TilePosition debugPoint = bottomMiddle;
#endif

        // @NOTE(JM) Bug, if move is invalid no move it taken
        if ((isWorldTileFree(*world, *gameState, middle))
                && (isWorldTileFree(*world, *gameState, bottomMiddle))) {

            if ((gameState->player1.absolutePosition.x != playerNewPos.x)
                || (gameState->player1.absolutePosition.y != playerNewPos.y)){

                playerHasMoved = true;

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

                gameState->player1.absolutePosition.x = playerNewPos.x;
                gameState->player1.absolutePosition.y = playerNewPos.y;
                gameState->player1.lastMoveDirections = (movedUp | movedDown | movedLeft | movedRight);
            }

        }else{

#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[200] = {};
            sprintf_s(buff, sizeof(buff),
                        "Tile x:%i y:%i (%i). Plr Pos x:%i y:%i. Plr New Pos: x:%i y:%i. CANNOT MOVE\n",
                        debugPoint.x,
                        debugPoint.y,
                        isWorldTileFree(*world, *gameState, debugPoint),
                        gameState->player1.absolutePosition.x,
                        gameState->player1.absolutePosition.y,
                        playerNewPos.x,
                        playerNewPos.y);
            memory->DEBUG_platformLog(buff);
#endif

        }

        if (playerHasMoved){

            setWorldPosition(*world, gameState, memory);

#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[500] = {};
            sprintf_s(buff, sizeof(buff),
"Chunk Index x:%i y:%i. \
World Tile x:%i y:%i. \
Chunk Tile x:%i y:%i. \
World Plr Pos x:%i y:%i. \
Chunk Plr Pos x:%i y:%i. \
Chunk Offset x:%i y:%i. \
\n",
                gameState->worldPosition.chunkIndexX,  gameState->worldPosition.chunkIndexY,
                gameState->worldPosition.worldTileBottomMiddle.x, gameState->worldPosition.worldTileBottomMiddle.y,
                gameState->worldPosition.chunkTileBottomMiddle.x, gameState->worldPosition.chunkTileBottomMiddle.y,
                gameState->player1.absolutePosition.x, gameState->player1.absolutePosition.y,
                gameState->player1.relativePosition.x, gameState->player1.relativePosition.y,
                gameState->worldPosition.chunkOffsetX, gameState->worldPosition.chunkOffsetY
            );
            memory->DEBUG_platformLog(buff);
#endif

        }

    }

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

    // Safe bounds checking
    #if 0
    if (gameState->player1.absolutePosition.y < 0) {
        gameState->player1.absolutePosition.y = 0;
    }
    if (((int64)gameState->player1.absolutePosition.y + gameState->player1.height) > frameBuffer->height) {
        gameState->player1.absolutePosition.y = (int32)(frameBuffer->height - gameState->player1.height);
    }
    if (((int64)gameState->player1.absolutePosition.x + gameState->player1.width) > frameBuffer->width) {
        gameState->player1.absolutePosition.x = (int32)(frameBuffer->width - gameState->player1.width);
    }
    if (gameState->player1.absolutePosition.x < 0) {
        gameState->player1.absolutePosition.x = 0;
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
 * Sets the tile position (the X and Y tile point index and the absolute pixel
 * position) based off of a given pixel X and Y
 * 
 * @param tilePoint         The TilePosition object to write the tile position data into
 * @param playerPixelPos    The X and Y pixel coords to base the calculation on. This is the top left point
 * @param pointPos          The offset from the playerPixelPos to apply
 * @param player            The player object containing the height/width etc
 * @param world             The world object
*/
internal_func
void setTilePositionForPlayer(TilePosition *tilePoint,
                                posXYUInt playerPixelPos,
                                PLAYER_POINT_POS pointPos,
                                Player player,
                                World world)
{
    float32 pixelInset  = 0.0f;
    float32 x           = 0.0f;
    float32 y           = 0.0f;

    // Apply point the offsets...
    switch (pointPos)
    {
    case PLAYER_POINT_POS::TOP_LEFT:
        x = (float32)playerPixelPos.x;
        y = (float32)playerPixelPos.y;
        break;
    case PLAYER_POINT_POS::TOP_MIDDLE:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.widthMetres) / 2.0f);
        y = ((float32)playerPixelPos.y + (float32)metresToPixels(world, player.heightMetres) - pixelInset);
        break;
    case PLAYER_POINT_POS::MIDDLE_LEFT:
        x = (float32)playerPixelPos.x;
        y = ((float32)playerPixelPos.y + ((float32)metresToPixels(world, player.heightMetres) / 2.0f) - pixelInset);
        break;
    case PLAYER_POINT_POS::MIDDLE:
        x = (float32)playerPixelPos.x + ((float32)metresToPixels(world, player.widthMetres) / 2.0f);
        y = (float32)playerPixelPos.y + ((float32)metresToPixels(world, player.heightMetres) / 2.0f);
        break;
    case PLAYER_POINT_POS::MIDDLE_RIGHT:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.widthMetres) - pixelInset);
        y = ((float32)playerPixelPos.y + ((float32)metresToPixels(world, player.heightMetres) / 2.0f) - pixelInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_MIDDLE:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.widthMetres) / 2.0f);
        y = ((float32)playerPixelPos.y + pixelInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_RIGHT:
        x = ((float32)playerPixelPos.x + (float32)metresToPixels(world, player.widthMetres) - pixelInset);
        y = ((float32)playerPixelPos.y + pixelInset);
        break;
    case PLAYER_POINT_POS::BOTTOM_LEFT:
        x = (float32)playerPixelPos.x;
        y = ((float32)playerPixelPos.y + pixelInset);
        break;
    default:
        assert(!"Tile point position not yet supported");
        break;
    }

    tilePoint->pointPosition = pointPos;

    // X and Y position coordinates of the tile (relative to the world)
    tilePoint->x = (int32)floorf(x / (float32)world.tileWidth);
    tilePoint->y = (int32)floorf(y / (float32)world.tileHeight);

    tilePoint->x = (modulo(tilePoint->x, WORLD_TOTAL_TILE_DIMENSIONS));
    tilePoint->y = (modulo(tilePoint->y, WORLD_TOTAL_TILE_DIMENSIONS));

    // The absolute pixel position of the point (in relation to the world)
    tilePoint->pointPixelPositionAbs.x = (uint32)x;
    tilePoint->pointPixelPositionAbs.y = (uint32)y;

    // The tile relative pixel position of the point (in relation to the tile itself)
    tilePoint->pointPixelPositionTileRel.x = (tilePoint->pointPixelPositionAbs.x - (tilePoint->x * world.tileWidth));
    tilePoint->pointPixelPositionTileRel.y = (tilePoint->pointPixelPositionAbs.y - (tilePoint->y * world.tileHeight));
}

internal_func
inline bool isWorldTileFree(World world, GameState gameState, TilePosition point)
{
    return true;

#if 0

    uint32 tileNumber = (point.y * world.tileDimensions) + point.x;

    uint32 *tileState = ((uint32 *)gameState.worldTiles + tileNumber);

    if (tileState) {
        if (*tileState) {
            return false;
        }
        return true;
    }

    return true;
#endif
}

internal_func
void initWorld(GameFrameBuffer frameBuffer,
                World *world,
                float32 tileHeight,
                uint8 pixelsPerMetre)
{
    world->tileDimensions = WORLD_TOTAL_TILE_DIMENSIONS;
    world->tileChunkDimensions = WORLD_TOTAL_TILE_CHUNK_DIMENSIONS;
    world->tileChunkMask = 0xFF;
    world->tileChunkShift = 8;

    world->tileHeightMetres = tileHeight;
    world->pixelsPerMetre = pixelsPerMetre;

    // @NOTE(JM) tiles and tile chunks are always square
    world->tileHeight = (uint16)((float32)pixelsPerMetre * tileHeight);
    world->tileWidth = world->tileHeight; 
    world->worldHeight = (world->tileHeight * world->tileChunkDimensions);
    world->worldWidth = (world->tileWidth * world->tileChunkDimensions);

    if (world->worldHeight > frameBuffer.height) {
        assert(!"Total tilemap pixel height > drawable screen height");
    }

    if (world->worldWidth > frameBuffer.width) {
        assert(!"Total tilemap pixel width > drawable screen width");
    }
}

/**
 * Sets the current world position by moving the tile chunk pointer (if needs be).
 *
 * Also, sets the data for both the absolute tile position and chunk-relative
 * tile position
 *
 * Finally, sets the player's chunk-relative X and Y pixel position
 *
 * @param world         The world
 * @param *gameState    Game state pointer
 * @param *memory       Game memory pointer (Debug only)
 * @return void
*/
internal_func
void setWorldPosition(World world, GameState *gameState, GameMemory *memory)
{
    // Make a copy of the world's current absolute tile position data
    TilePosition originalAbsoluteTilePos = gameState->worldPosition.absoluteTile;

    // Update the world's current tile position data based off of the player's
    // absolute x and y position. This is so we can determine if the player
    // has moved tiles.
    setTilePositionForPlayer(&gameState->worldPosition.absoluteTile,
                                gameState->player1.absolutePosition,
                                PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                gameState->player1,
                                world);

    setTilePositionForPlayer(&gameState->worldPosition.worldTileBottomLeft,
                                    gameState->player1.absolutePosition,
                                    PLAYER_POINT_POS::BOTTOM_LEFT,
                                    gameState->player1,
                                    world);

    setTilePositionForPlayer(&gameState->worldPosition.worldTileBottomMiddle,
                                gameState->player1.absolutePosition,
                                PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                gameState->player1,
                                world);

    setTilePositionForPlayer(&gameState->worldPosition.worldTileBottomRight,
                                gameState->player1.absolutePosition,
                                PLAYER_POINT_POS::BOTTOM_RIGHT,
                                gameState->player1,
                                world);

    // Has the absolute tile position x or y changed? AKA, has the player moved
    // tiles?
    if ((gameState->worldPosition.absoluteTile.x != originalAbsoluteTilePos.x)
            || (gameState->worldPosition.absoluteTile.y != originalAbsoluteTilePos.y)) {

        // X axis, moving right
        if ((gameState->player1.lastMoveDirections & (uint)PlayerMovementDirection::RIGHT)) {

            gameState->worldPosition.chunkIndexX++;
            gameState->worldPosition.chunkOffsetX += world.tileWidth;

#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[100] = {};
            sprintf_s(buff, sizeof(buff),
                    "Player moving tile right...\n");
            memory->DEBUG_platformLog(buff);
#endif

        // X axis, moving left
        }else if ((gameState->player1.lastMoveDirections & (uint)PlayerMovementDirection::LEFT)) {

            gameState->worldPosition.chunkIndexX--;
            gameState->worldPosition.chunkOffsetX -= world.tileWidth;

#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[100] = {};
            sprintf_s(buff, sizeof(buff),
                    "Player moving tile left...\n");
            memory->DEBUG_platformLog(buff);
#endif
        }

        // Y axis
        if ((gameState->player1.lastMoveDirections & (uint)PlayerMovementDirection::UP)) {

            gameState->worldPosition.chunkIndexY++;
            gameState->worldPosition.chunkOffsetY += world.tileHeight;

#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[100] = {};
            sprintf_s(buff, sizeof(buff),
                    "Player moving tile up...\n");
            memory->DEBUG_platformLog(buff);
#endif

        }else if ((gameState->player1.lastMoveDirections & (uint)PlayerMovementDirection::DOWN)) {

            gameState->worldPosition.chunkIndexY--;
            gameState->worldPosition.chunkOffsetY -= world.tileHeight;

#ifdef HANDMADE_DEBUG_TILE_POS
            char buff[100] = {};
            sprintf_s(buff, sizeof(buff),
                    "Player moving tile down...\n");
            memory->DEBUG_platformLog(buff);
#endif
        }

        // Advance the tile chunk pointer.
        gameState->worldPosition.chunkIndexX = modulo(gameState->worldPosition.chunkIndexX, WORLD_TOTAL_TILE_DIMENSIONS);
        gameState->worldPosition.chunkIndexY = modulo(gameState->worldPosition.chunkIndexY, WORLD_TOTAL_TILE_DIMENSIONS);
        gameState->worldPosition.chunkTiles
            = &gameState->worldTiles[gameState->worldPosition.chunkIndexY][gameState->worldPosition.chunkIndexX];

        // Wrap the tile chunk offset (Used to calculate the player's relative
        // pixel position)

    }

    // Set the player's chunk relative pixel position by calculating the offset
    // from the tile chunk relative to the world
#if 0
    int32 playerRelPosX = (gameState->worldPosition.absoluteTile_new[0].x - (WORLD_TOTAL_TILE_CHUNK_DIMENSIONS / 2));
    playerRelPosX = (playerRelPosX * world.tileWidth);
    playerRelPosX = (gameState->worldPosition.absoluteTile_new[0].pointPixelPositionAbs.x - playerRelPosX);

    gameState->player1.relativePosition.x = playerRelPosX;
#endif

    uint offsetPoints = (gameState->worldPosition.worldTileBottomMiddle.x - 5);

    uint32 offsetPixels (offsetPoints * world.tileWidth);

    gameState->player1.relativePosition.x
        = (gameState->player1.absolutePosition.x - offsetPixels);

    gameState->player1.relativePosition.y
        = (gameState->player1.absolutePosition.y);

    // Finally, update the world's current chunk relative tile position (based
    // off of the player's chunk relative x and y position.) We do this because we
    // always want the world's tile position to be relative to the viewable
    // tile chunk.
    setTilePositionForPlayer(&gameState->worldPosition.chunkTile,
                                gameState->player1.relativePosition,
                                PLAYER_POINT_POS::BOTTOM_MIDDLE,
                                gameState->player1,
                                world);
}

internal_func
inline int64 metresToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMetre * metres);
    return (int64)pixels;
}

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
                    Colour colour,
                    bool tilemapOverrunCheck)
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
    uint32 *row = (uint32*)buffer->memory;

    // Move to last row as starting position (bottom left of axis)
    row = (row + ((buffer->width * buffer->height) - buffer->width));

    // Move up to starting row
    row = (row - (buffer->width * yOffset));

    // Move in from left to starting absolutePosition
    row = (row + xOffset);

    // Up (rows)
    for (int64 i = 0; i < height; i++) {

        // Tilemap overrun checking
        if (tilemapOverrunCheck) {
            if ((yOffset + i) >= world.worldHeight) {
                continue;
            }
        }

        // Accross (columns)
        uint32* pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            // Tilemap overrun checking
            if (tilemapOverrunCheck) {
                if ((xOffset + x) >= world.worldWidth) {
                    continue;
                }
            }

            *pixel = hexColour;
            pixel = (pixel + 1);
        }

        // Move up one entire row
        row = (row - buffer->width);
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