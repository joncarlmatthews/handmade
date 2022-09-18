#ifndef HEADER_HH_TILEMAP
#define HEADER_HH_TILEMAP

#include "types.h"

//
// Tilemap
//====================================================
// Our tilemap is our entire game world

// How many "tile chunks" does one side of the enire tilemap have?
// @NOTE(JM) temp as tilemap storage will be sparse
#define TILE_CHUNK_DIMENSIONS 30

// How many bits do we want to allocate to the tile chunk's tile dimensions?
#define TILE_CHUNK_TILE_DIMENSIONS_BIT_SHIFT 7

// How many meters does one side of an individual tile have? (Tiles are always square)
#define TILE_DIMENSIONS_METERS 2.0f

typedef struct TileChunk
{
    // Pointer to all of the tile data
    uint32 *tiles;

} TileChunk;

typedef struct Tilemap
{
    // Total number of tile chunks per one side of a tilemap
    uint32 tileChunkDimensions;

    // Total number of tile chunks within the tilemap
    // @NOTE(JM) temp as tilemap storage will be sparse
    uint32 totalTileChunks;

    // Total number of tiles per one side of a tile chunk
    // (Tile chunks are always square)
    uint32 tileChunkTileDimensionsBitShift;
    uint32 tileChunkTileDimensionsBitMask;
    uint32 tileChunkTileDimensions;

    // Total number of tiles across 1 whole side of the tilemap
    uint32 tileDimensions;

    // Total number of tiles within all of the tile chunks
    // @NOTE(JM) temp as tilemap storage will be sparse
    uint64 totalTiles;

    // Height and width in pixels of an individual tile 
    uint32 tileHeightPx;
    uint32 tileWidthPx;

    // Height and width in pixels of each tile chunk 
    uint32 tileChunkHeightPx;
    uint32 tileChunkWidthPx;

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

    // X and Y tile index relative to the tile chunk
    xyuint chunkRelativeTileIndex;

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
                    uint32 tileChunkDimensions,
                    uint32 tileChunkTileDimensionsShift,
                    float32 tileDimensionsMeters);

void setCoordinateData(TilemapCoordinates *coordinates,
                        uint32 pixelX,
                        uint32 pixelY,
                        Tilemap tilemap);

typedef struct PlayerPositionData PlayerPositionData;
bool isTilemapTileFree(Tilemap tilemap, PlayerPositionData *playerPositionData);

#endif
