#pragma once

#include <math.h>
#include "types.h"
#include "game.h"
#include "world.h"
#include "player.h"
#include "utility.h"

void initWorld(World *world,
                uint16 pixelsPerMetre,
                float32 tileDimensionsMeters,
                uint16 totalTileDimensions,
                uint16 tileChunkDimensions)
{
    world->totalTileDimensions = totalTileDimensions;
    world->tileChunkDimensions = tileChunkDimensions;
    world->totalTileChunks = ((world->totalTileDimensions / world->tileChunkDimensions) * 2);

    // @TODO(JM)
    //world->tileChunkMask = 0xFF;
    //world->tileChunkShift = 8;

    world->pixelsPerMetre = pixelsPerMetre;

    // @NOTE(JM) tiles and tile chunks are always square
    world->tileHeightPx = (uint16)(world->pixelsPerMetre * tileDimensionsMeters);
    world->tileWidthPx = world->tileHeightPx;
    world->tileChunkHeightPx = (world->tileHeightPx * world->tileChunkDimensions);
    world->tileChunkWidthPx = (world->tileWidthPx * world->tileChunkDimensions);
}

void setWorldPosition(GameState *gameState, World world, GameFrameBuffer *frameBuffer)
{
    // Active tile is based off of the bottom middle position of the player.

    // Get the pixel coordinates of the bottom middle of the player.
    float32 yPixelInset = 0.0f;
    float32 x = ((float32)gameState->player1.absolutePosition.x + (float32)gameState->player1.widthPx / 2.0f);
    float32 y = ((float32)gameState->player1.absolutePosition.y + yPixelInset);

    gameState->worldPosition.pixelCoordinates.x = (int32)floorf(x);
    gameState->worldPosition.pixelCoordinates.y = (int32)floorf(y);

    // Calculate the absolute x and y tile index relative to the World
    uint32 tileIndexX = (int32)floorf((float32)x / (float32)world.tileWidthPx);
    uint32 tileIndexY = (int32)floorf((float32)y / (float32)world.tileHeightPx);

    gameState->worldPosition.tileIndex.x = (modulo(tileIndexX, world.totalTileDimensions));
    gameState->worldPosition.tileIndex.y = (modulo(tileIndexY, world.totalTileDimensions));

    // Calculate the x and y tile chunk index relative to the world
    gameState->worldPosition.chunkIndex.x = (int32)floorf((float32)x / (float32)(world.tileWidthPx * world.tileChunkDimensions));
    gameState->worldPosition.chunkIndex.y = (int32)floorf((float32)y / (float32)(world.tileHeightPx * world.tileChunkDimensions));

    // @TODO(JM)
    gameState->worldPosition.chunkRelativePixelCoordinates.x = 0;
    gameState->worldPosition.chunkRelativePixelCoordinates.y = 0;

    // @TODO(JM)
    gameState->worldPosition.tileRelativePixelCoordinates.x = 0;
    gameState->worldPosition.tileRelativePixelCoordinates.y = 0;

    // Update the camera position
    gameState->cameraPositionPx.x = modulo((gameState->player1.absolutePosition.x - (frameBuffer->widthPx / 2)), (world.tileWidthPx * world.totalTileDimensions));
    gameState->cameraPositionPx.y = modulo((gameState->player1.absolutePosition.y - (frameBuffer->heightPx / 2)), (world.tileHeightPx * world.totalTileDimensions));
}

bool isWorldTileFree(World world,
                        GameState *gameState,
                        PlayerPositionData *playerPositionData)
{
    uint32 tileNumber = (playerPositionData->activeTile.tileIndex.y * world.totalTileDimensions) + playerPositionData->activeTile.tileIndex.x;

    uint32 *tileState = ((uint32*)gameState->worldTiles + tileNumber);

    if (tileState) {
        if (*tileState == 2) {
            return false;
        }
    }

    return true;
}

int64 metersToPixels(World world, float32 metres)
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