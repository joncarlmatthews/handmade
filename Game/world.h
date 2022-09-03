#ifndef HEADER_HH_WORLD
#define HEADER_HH_WORLD

#include "types.h"

// How many pixels to represent 1 meter?
#define WORLD_PIXELS_PER_METER 25

typedef struct World {

    // @see WORLD_TOTAL_TILE_DIMENSIONS
    uint16 totalTileDimensions;

    // @see WORLD_TILE_CHUNK_DIMENSIONS
    uint16 tileChunkDimensions;

    // Total number of tile chunks within the world
    uint32 totalTileChunks;

    uint32 tileChunkMask; // @TODO(JM)
    uint32 tileChunkShift; // @TODO(JM)

    // Height and width in pixels of an individual tile 
    uint16 tileHeightPx;
    uint16 tileWidthPx;

    // Height and width in pixels of each tile chunk 
    uint16 tileChunkHeightPx;
    uint16 tileChunkWidthPx;

    // Height and width in pixels of the entire world 
    uint16 worldHeightPx;
    uint16 worldWidthPx;

    // How many pixels = 1 metre in our World?
    uint16 pixelsPerMetre;
} World;

typedef struct WorldCoordinates {

    // The absolute pixel coordinates (relative to the world)
    xyuint pixelCoordinates;

    // x and y absolute tile index (relative to the world)
    xyuint tileIndex;

    // X and Y index of the current tile chunk
    xyuint chunkIndex;

    // The pixel coordinates relative to the tile chunk the pixelCoordinates are in
    xyuint chunkRelativePixelCoordinates;

    // The pixel coordinates relative to the tile the pixelCoordinates are in
    xyuint tileRelativePixelCoordinates;

} WorldCoordinates;

void initWorld(World *world,
                uint16 pixelsPerMetre,
                float32 tileDimensionsMeters,
                uint16 totalTileDimensions,
                uint16 tileChunkDimensions);

typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;

void setWorldPosition(GameState *gameState,
                        World world,
                        GameFrameBuffer *frameBuffer);

typedef struct PlayerPositionData PlayerPositionData;
bool isWorldTileFree(World world,
                        GameState *gameState,
                        PlayerPositionData *playerPositionData);

int64 metersToPixels(World world, float32 metres);

void setTileColour(Colour *tileColour, uint32 tileValue);

#endif