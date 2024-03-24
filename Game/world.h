#ifndef HEADER_HH_WORLD
#define HEADER_HH_WORLD

#include "tilemap.h"

typedef struct World {

    // Height and width in pixels of the entire World 
    uint32 worldHeightPx;
    uint32 worldWidthPx;

    // How many pixels = 1 meter in our World?
    uint16 pixelsPerMeter;

    Tilemap tilemap;
} World;

typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;

void setWorldPosition(GameState *gameState, GameFrameBuffer *frameBuffer);

void setCameraPosition(GameState *gameState, GameFrameBuffer *frameBuffer);

uint32 metersToPixels(World world, float32 metres);


#endif