#ifndef HEADER_HH_PLAYER
#define HEADER_HH_PLAYER

#include "types.h"
#include "game.h"

typedef struct PlayerPositionData {
    PLAYER_POINT_POS pointPosition;
    TilePosition activeTile;
} PlayerPositionData;

void getPositionDataForPlayer(PlayerPositionData *positionData,
                                posXYUInt playerPixelPos,
                                PLAYER_POINT_POS pointPos,
                                Player player,
                                World world);

void getActiveTileForPlayer(TilePosition *tilePosition,
                                Player player,
                                World world);

#endif