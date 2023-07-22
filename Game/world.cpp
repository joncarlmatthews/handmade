#include "world.h"
#include "game.h"

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
                            intrin_roundF32ToUI32(gameState->player1.gamePosition.x),
                            intrin_roundF32ToUI32(gameState->player1.gamePosition.y),
                            gameState->player1.zIndex,
                            gameState->world.tilemap);
}

uint32 metersToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMeter * metres);
    return (int64)pixels;
}