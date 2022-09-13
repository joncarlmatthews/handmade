#ifndef HEADER_HH_TILEMAP
#define HEADER_HH_TILEMAP

#include "types.h"

//
// Tilemap
//====================================================
// Our tilemap is our entire game world

// How many tiles does one side of the entire tilemap have?
// 30x30 = 900
#define TOTAL_TILE_DIMENSIONS 30

// How many tiles does one side of a "tile chunk" have?
// 10x10 = 100
#define TILE_CHUNK_DIMENSIONS 15

// How many meters does one side of a tile have?
// 2x2 = 4
#define TILE_DIMENSIONS_METERS 2.0f

typedef struct TileChunk {

    uint32 *tiles;

} TileChunk;

typedef struct Tilemap {

    // @see TOTAL_TILE_DIMENSIONS
    uint16 totalTileDimensions;

    // @see TILE_CHUNK_DIMENSIONS
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

    // Tile data
    uint32 *tiles;

} Tilemap;

typedef struct TilemapCoordinates {

    // The absolute pixel coordinates (relative to the entire tilemap)
    xyuint pixelCoordinates;

    // x and y absolute tile index (relative to the entire tilemap)
    xyuint tileIndex;

    // X and Y index of the current tile chunk
    xyuint chunkIndex;

    // The pixel coordinates relative to the tile chunk the pixelCoordinates are in
    xyuint chunkRelativePixelCoordinates;

    // The pixel coordinates relative to the tile the pixelCoordinates are in
    xyuint tileRelativePixelCoordinates;

} TilemapCoordinates;

void initTilemap(Tilemap *tilemap,
                    uint16 pixelsPerMeter,
                    float32 tileDimensionsMeters,
                    uint16 totalTileDimensions,
                    uint16 tileChunkDimensions);

typedef struct PlayerPositionData PlayerPositionData;
bool isTilemapTileFree(Tilemap tilemap, PlayerPositionData *playerPositionData);

#endif
