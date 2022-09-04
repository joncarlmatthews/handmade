#ifndef HEADER_HH_WORLD
#define HEADER_HH_WORLD

#include "types.h"
#include "tilemap.h"

// How many pixels to represent 1 meter?
#define WORLD_PIXELS_PER_METER 25

typedef struct World {

    // Height and width in pixels of the entire World 
    uint16 worldHeightPx;
    uint16 worldWidthPx;

    // How many pixels = 1 meter in our World?
    uint16 pixelsPerMetre;

    Tilemap tilemap;
} World;

void initWorld(World *world, Tilemap tilemap, uint32 pixelsPerMetre);

typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;

void setWorldPosition(GameState *gameState, GameFrameBuffer *frameBuffer);

typedef struct PlayerPositionData PlayerPositionData;
bool isWorldTileFree(GameState *gameState, PlayerPositionData *playerPositionData);

int64 metersToPixels(World world, float32 metres);

void setTileColour(Colour *tileColour, uint32 tileValue);

#endif