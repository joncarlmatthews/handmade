#pragma once

#include "types.h"
#include "game.h"
#include "world.h"
#include "player.h"
#include "utility.h"

/**
 * Updates the World Position
 *
 * @param int32         xMoveAmtPx  How much movement has been made on the X axis
 * @param int32         yMoveAmtPx  How much movement has been made on the Y axis
 * @param World         world       The world
 * @param GameState     *gameState  Game state pointer
 * 
 * @return void
*/
void updateWorldPosition(int32 xMoveAmtPx,
                            int32 yMoveAmtPx,
                            World world,
                            GameState *gameState)
{
    // Update the X and Y movement tracking
    uint32 tileChunkStartPixelSliderX = modulo((gameState->worldPosition.cameraPositionPx.x += xMoveAmtPx), (world.tileWidthPx * world.totalTileDimensions));
    uint32 tileChunkStartPixelSliderY = modulo((gameState->worldPosition.cameraPositionPx.y += yMoveAmtPx), (world.tileHeightPx * world.totalTileDimensions));

    gameState->worldPosition.cameraPositionPx.x = tileChunkStartPixelSliderX;
    gameState->worldPosition.cameraPositionPx.y = tileChunkStartPixelSliderY;

    // Calculate the currently active tile based on player1's position and
    // write it to the world position
    getActiveTileForPlayer(&gameState->worldPosition.activeTile,
                            gameState->player1,
                            world);
}

bool isWorldTileFree(World world,
                        GameState gameState,
                        PlayerPositionData playerPositionData)
{
    uint32 tileNumber = (playerPositionData.activeTile.tileIndex.y * world.totalTileDimensions) + playerPositionData.activeTile.tileIndex.x;

    uint32 *tileState = ((uint32*)gameState.worldTiles + tileNumber);

    if (tileState) {
        if (*tileState == 2) {
            return false;
        }
    }

    return true;
}

int64 metresToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMetre * metres);
    return (int64)pixels;
}

void setTileColour(Colour *tileColour, uint32 tileValue)
{
    switch (tileValue) {
        default:
            *tileColour = { 0.94f, 0.94f, 0.94f };
            break;

        case 1:
            *tileColour = { 0.96f, 0.15f, 0.56f };
            break;

        case 2:
            *tileColour = { 0.15f, 0.18f, 0.96f };
            break;

        case 3:
            *tileColour = { 39.0f, 0.96f, 0.16f };
            break;

        case 4:
            *tileColour = { 0.96f, 0.76f, 0.019f };
            break;

        case 5:
            *tileColour = { 0.96f, 0.15f, 0.15f };
            break;

        case 6:
            *tileColour = { 0.25f, 1.0f, 0.0f };
            break;
    }
}