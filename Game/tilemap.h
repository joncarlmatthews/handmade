#ifndef HEADER_HH_TILEMAP
#define HEADER_HH_TILEMAP

#include "types.h"

//
// Tilemap
//====================================================
// Our tilemap is our entire game world

// How many tiles does one side of the entire tilemap have?
// 30x30 = 900
//#define TOTAL_TILE_DIMENSIONS 30

// How many chunks does one side of the enire tilemap" have?
#define TILE_CHUNK_DIMENSIONS 3

// How many tiles does one side of a "tile chunk" have?
// (Tile chunks are always square)
#define TILE_CHUNK_TILE_DIMENSIONS 10

// How many meters does one side of a tile have? (Tiles are always square)
#define TILE_DIMENSIONS_METERS 2.0f

typedef struct TileChunk
{
    // Pointer to all of the tile data
    uint32 *tiles;

} TileChunk;

typedef struct Tilemap
{
    // Total number of tile chunks per one side of a tilemap
    uint16 tileChunkDimensions;

    // Total number of tile chunks within the tilemap
    uint16 totalTileChunks;

    // Total number of tiles per one side of a tile chunk
    uint16 tileChunkTileDimensions;

    // Total number of tiles across 1 whole side of the tilemap
    uint32 tileDimensions;

    // Total number of tiles within all of the tile chunks
    uint32 totalTiles;

    uint32 tileChunkMask; // @TODO(JM)
    uint32 tileChunkShift; // @TODO(JM)

    // Height and width in pixels of an individual tile 
    uint16 tileHeightPx;
    uint16 tileWidthPx;

    // Height and width in pixels of each tile chunk 
    uint16 tileChunkHeightPx;
    uint16 tileChunkWidthPx;

    // Pointer to all of the tile chunks
    TileChunk *tileChunks;

} Tilemap;

typedef struct TilemapCoordinates
{

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

typedef struct GameMemoryBlock GameMemoryBlock;
typedef struct World World;
void initTilemap(GameMemoryBlock *memoryBlock,
                    World *world,
                    uint16 pixelsPerMeter,
                    uint16 tileChunkDimensions,
                    uint16 tileChunkTileDimensions,
                    float32 tileDimensionsMeters);

typedef struct PlayerPositionData PlayerPositionData;
bool isTilemapTileFree(Tilemap tilemap, PlayerPositionData *playerPositionData);

#endif
