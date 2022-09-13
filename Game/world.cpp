#include <math.h>
#include "types.h"
#include "game.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"
#include "utility.h"

void initWorld(World *world,
                Tilemap tilemap,
                uint32 pixelsPerMeter)
{
    world->pixelsPerMeter = (uint16)pixelsPerMeter;

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
    uint32 tileIndexX = (int32)floorf((float32)x / (float32)(*gameState->world).tilemap.tileWidthPx);
    uint32 tileIndexY = (int32)floorf((float32)y / (float32)(*gameState->world).tilemap.tileHeightPx);

    gameState->worldPosition.tileIndex.x = (modulo(tileIndexX, (*gameState->world).tilemap.totalTileDimensions));
    gameState->worldPosition.tileIndex.y = (modulo(tileIndexY, (*gameState->world).tilemap.totalTileDimensions));

    // Calculate the x and y tile chunk index relative to the world
    gameState->worldPosition.chunkIndex.x = (int32)floorf((float32)x / (float32)((*gameState->world).tilemap.tileWidthPx * (*gameState->world).tilemap.tileChunkDimensions));
    gameState->worldPosition.chunkIndex.y = (int32)floorf((float32)y / (float32)((*gameState->world).tilemap.tileHeightPx * (*gameState->world).tilemap.tileChunkDimensions));

    // @TODO(JM)
    gameState->worldPosition.chunkRelativePixelCoordinates.x = 0;
    gameState->worldPosition.chunkRelativePixelCoordinates.y = 0;

    // @TODO(JM)
    gameState->worldPosition.tileRelativePixelCoordinates.x = 0;
    gameState->worldPosition.tileRelativePixelCoordinates.y = 0;

    // Update the camera position
    gameState->cameraPositionPx.x = modulo((gameState->player1.absolutePosition.x - (frameBuffer->widthPx / 2)), (*gameState->world).worldWidthPx);
    gameState->cameraPositionPx.y = modulo((gameState->player1.absolutePosition.y - (frameBuffer->heightPx / 2)), (*gameState->world).worldHeightPx);
}

int64 metersToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMeter * metres);
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