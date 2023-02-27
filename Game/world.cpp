#include <math.h>
#include "types.h"
#include "game.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"
#include "utility.h"

/**
 * @brief Sets the tilemap pixel coordinates for the World's active tile.
 * 
 * @param gameState 
 * @param frameBuffer 
*/
void setWorldPosition(GameState *gameState, GameFrameBuffer *frameBuffer)
{
    setTileCoordinateData(&gameState->worldPosition,
                            gameState->player1.gamePosition.x,
                            gameState->player1.gamePosition.y,
                            gameState->world.tilemap);
}

int64 metersToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMeter * metres);
    return (int64)pixels;
}