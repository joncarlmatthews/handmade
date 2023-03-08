#include <math.h>
#include "types.h"
#include "game.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"
#include "utility.h"

/**
 * @brief Sets the tilemap position data for the world's active tile based
 * off of the players game position
 * 
 * @param gameState 
 * @param frameBuffer 
*/
void setWorldPosition(GameState *gameState, GameFrameBuffer *frameBuffer)
{
    setTilemapPositionData(&gameState->worldPosition,
                            gameState->player1.gamePosition.x,
                            gameState->player1.gamePosition.y,
                            gameState->player1.zIndex,
                            gameState->world.tilemap);
}

int64 metersToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMeter * metres);
    return (int64)pixels;
}