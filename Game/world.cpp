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
    gameState->worldPosition.absPixelPos.x = gameState->player1.gamePosition.x;
    gameState->worldPosition.absPixelPos.y = gameState->player1.gamePosition.y;

    // Calculate the absolute tile index
    xyuint absTileIndex = geAbsTileIndexFromAbsPixel(gameState->player1.gamePosition.x,
                                                        gameState->player1.gamePosition.y,
                                                        gameState->world.tilemap);

    gameState->worldPosition.tileIndex.x = absTileIndex.x;
    gameState->worldPosition.tileIndex.y = absTileIndex.y;

    // Calculate the x and y tile chunk index relative to the entire tilemap
    xyzuint chunkIndex = getTileChunkIndexForAbsTile(absTileIndex.x,
                                                        absTileIndex.y,
                                                        gameState->player1.zIndex,
                                                        gameState->world.tilemap);

    gameState->worldPosition.chunkIndex.x = chunkIndex.x;
    gameState->worldPosition.chunkIndex.y = chunkIndex.y;

    // Get the tile index relative to the tile chunk
    xyuint chunkRelativeTileIndex = getChunkRelativeTileIndex(absTileIndex.x,
                                                                absTileIndex.y,
                                                                gameState->world.tilemap);

    gameState->worldPosition.chunkRelativeTileIndex.x = chunkRelativeTileIndex.x;
    gameState->worldPosition.chunkRelativeTileIndex.y = chunkRelativeTileIndex.y;

    xyuint tileRelPos = getTileRelativePixelPos(gameState->player1.gamePosition.x,
                                                gameState->player1.gamePosition.y,
                                                gameState->world.tilemap);

    gameState->worldPosition.tileRelativePixelPos.x = tileRelPos.x;
    gameState->worldPosition.tileRelativePixelPos.y = tileRelPos.y;
}

int64 metersToPixels(World world, float32 metres)
{
    float32 pixels = (world.pixelsPerMeter * metres);
    return (int64)pixels;
}