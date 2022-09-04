#pragma once

#include <math.h>
#include "types.h"
#include "game.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"
#include "utility.h"

// @TODO(JM) move this to tilemap.cpp
void initTilemap(Tilemap *tilemap,
                    uint16 pixelsPerMetre,
                    float32 tileDimensionsMeters,
                    uint16 totalTileDimensions,
                    uint16 tileChunkDimensions)
{
    tilemap->totalTileDimensions = totalTileDimensions;
    tilemap->tileChunkDimensions = tileChunkDimensions;
    tilemap->totalTileChunks = ((tilemap->totalTileDimensions / tilemap->tileChunkDimensions) * 2);

    // @TODO(JM)
    //tilemap->tileChunkMask = 0xFF;
    //tilemap->tileChunkShift = 8;

    // @NOTE(JM) tiles and tile chunks are always square
    tilemap->tileHeightPx = (uint16)(pixelsPerMetre * tileDimensionsMeters);
    tilemap->tileWidthPx = tilemap->tileHeightPx;
    tilemap->tileChunkHeightPx = (tilemap->tileHeightPx * tilemap->tileChunkDimensions);
    tilemap->tileChunkWidthPx = tilemap->tileChunkHeightPx;

    // @NOTE(JM) not sure how to allocate this on the heap, as this array is
    // too large for the stack...
    uint8 origTiles[TOTAL_TILE_DIMENSIONS][TOTAL_TILE_DIMENSIONS] = ALL_TILES;

    // Copy the tiles into the world tiles, making it so that the Y axis
    // goes up
    uint32 worldY = 0;
    for (int32 y = (TOTAL_TILE_DIMENSIONS-1); y >= 0; y--){
        for (uint x = 0; x < TOTAL_TILE_DIMENSIONS; x++) {
            tilemap->tiles[worldY][x] = (uint32)origTiles[y][x];
        }
        worldY++;
    }
}

void initWorld(World *world,
                Tilemap tilemap,
                uint32 pixelsPerMetre)
{
    world->pixelsPerMetre = (uint16)pixelsPerMetre;

    world->worldHeightPx = (tilemap.tileHeightPx * tilemap.totalTileDimensions);
    world->worldWidthPx = (tilemap.tileWidthPx * tilemap.totalTileDimensions);

    world->tilemap = tilemap;
}

void setWorldPosition(GameState *gameState, GameFrameBuffer *frameBuffer)
{
    // Active tile is based off of the bottom middle position of the player.

    // Get the pixel coordinates of the bottom middle of the player.
    float32 yPixelInset = 0.0f;
    float32 x = ((float32)gameState->player1.absolutePosition.x + (float32)gameState->player1.widthPx / 2.0f);
    float32 y = ((float32)gameState->player1.absolutePosition.y + yPixelInset);

    gameState->worldPosition.pixelCoordinates.x = (int32)floorf(x);
    gameState->worldPosition.pixelCoordinates.y = (int32)floorf(y);

    // Calculate the absolute x and y tile index relative to the World
    uint32 tileIndexX = (int32)floorf((float32)x / (float32)gameState->world.tilemap.tileWidthPx);
    uint32 tileIndexY = (int32)floorf((float32)y / (float32)gameState->world.tilemap.tileHeightPx);

    gameState->worldPosition.tileIndex.x = (modulo(tileIndexX, gameState->world.tilemap.totalTileDimensions));
    gameState->worldPosition.tileIndex.y = (modulo(tileIndexY, gameState->world.tilemap.totalTileDimensions));

    // Calculate the x and y tile chunk index relative to the world
    gameState->worldPosition.chunkIndex.x = (int32)floorf((float32)x / (float32)(gameState->world.tilemap.tileWidthPx * gameState->world.tilemap.tileChunkDimensions));
    gameState->worldPosition.chunkIndex.y = (int32)floorf((float32)y / (float32)(gameState->world.tilemap.tileHeightPx * gameState->world.tilemap.tileChunkDimensions));

    // @TODO(JM)
    gameState->worldPosition.chunkRelativePixelCoordinates.x = 0;
    gameState->worldPosition.chunkRelativePixelCoordinates.y = 0;

    // @TODO(JM)
    gameState->worldPosition.tileRelativePixelCoordinates.x = 0;
    gameState->worldPosition.tileRelativePixelCoordinates.y = 0;

    // Update the camera position
    gameState->cameraPositionPx.x = modulo((gameState->player1.absolutePosition.x - (frameBuffer->widthPx / 2)), gameState->world.worldWidthPx);
    gameState->cameraPositionPx.y = modulo((gameState->player1.absolutePosition.y - (frameBuffer->heightPx / 2)), gameState->world.worldHeightPx);
}

// @TODO(JM) move to tilemap.cpp
bool isWorldTileFree(GameState *gameState,
                        PlayerPositionData *playerPositionData)
{
    uint32 tileNumber = (playerPositionData->activeTile.tileIndex.y * gameState->world.tilemap.totalTileDimensions) + playerPositionData->activeTile.tileIndex.x;

    uint32 *tileState = ((uint32*)gameState->world.tilemap.tiles + tileNumber);

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