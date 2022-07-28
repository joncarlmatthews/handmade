#ifndef HEADER_HH_WORLD
#define HEADER_HH_WORLD

#include "types.h"
#include "game.h"
#include "player.h"

//
// World/Tilemaps (reference GameState)
//====================================================

void updateWorldPosition(int32 xMoveAmtPx,
                            int32 yMoveAmtPx,
                            World world,
                            GameState *gameState);

bool isWorldTileFree(World world,
                        GameState gameState,
                        PlayerPositionData playerPositionData);

int64 metresToPixels(World world, float32 metres);

#endif