#include <math.h>
#include "types.h"
#include "game.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"
#include "utility.h"

/**
 * @brief Sets the tilemap pixel coordinates for the World's active tile.
 * Active tile is based off of the bottom middle position of the player.
 * 
 * @param gameState 
 * @param frameBuffer 
*/
void setWorldPosition(GameState *gameState, GameFrameBuffer *frameBuffer)
{
    // Get the pixel coordinates of the bottom middle of the player.
    float32 yPixelInset = 0.0f;
    float32 x = ((float32)gameState->player1.absolutePosition.x + (float32)gameState->player1.widthPx / 2.0f);
    float32 y = ((float32)gameState->player1.absolutePosition.y + yPixelInset);

    x = floorf(x);
    y = floorf(y);

    setCoordinateData(&gameState->worldPosition, (uint32)x, (uint32)y, gameState->world->tilemap);

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