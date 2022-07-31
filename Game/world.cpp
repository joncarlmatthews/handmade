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
    uint32 tileChunkStartPixelSliderX = modulo((gameState->worldPosition.tileChunkStartPixelSlider.x += xMoveAmtPx), (world.tileWidthPx * world.totalTileDimensions));
    uint32 tileChunkStartPixelSliderY = modulo((gameState->worldPosition.tileChunkStartPixelSlider.y += yMoveAmtPx), (world.tileHeightPx * world.totalTileDimensions));

    gameState->worldPosition.tileChunkStartPixelSlider.x = tileChunkStartPixelSliderX;
    gameState->worldPosition.tileChunkStartPixelSlider.y = tileChunkStartPixelSliderY;

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
        if (1 != *tileState) {
            return false;
        }
        return true;
    }

    return true;
}

int64 metresToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMetre * metres);
    return (int64)pixels;
}