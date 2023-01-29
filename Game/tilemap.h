#ifndef HEADER_HH_TILEMAP
#define HEADER_HH_TILEMAP

#include "types.h"
#include "memory.h"

//
// Tilemap
//====================================================
// Tilemap is the entire game world

// How many bits of a 32-bit integer do we want to allocate to the tilemap's
// total tile dimensions*? (along one side)
#define TILE_DIMENSIONS_BIT_SHIFT 10

// How many bits of a 32-bit integer do we want to allocate to the total number
// of "tile chunks" dimensions*? (*along one side)
#define TILE_CHUNK_DIMENSIONS_BIT_SHIFT 2

// How many bits of a 32-bit integer do we want to allocate to the tile chunk's
// tile dimensions*? (*along one side)
#define TILE_CHUNK_TILE_DIMENSIONS_BIT_SHIFT 4

// How many meters does one side of an individual tile have? (Tiles are always square)
#define TILE_DIMENSIONS_METERS 2.0f

typedef struct TileChunk
{
    // Pointer to all of the tile data
    uint32 *tiles;

} TileChunk;

typedef struct Tilemap
{
    // Total possible number of tiles across 1 whole side of the tilemap
    // (Total tilemap tile dimensions are always square)
    uint32 tileDimensionsBitShift;
    uint32 tileDimensionsBitMask;
    uint32 tileDimensions;

    // Total number of tile chunks requested within the tilemap
    uint32 tileChunkDimensionsBitShift;
    uint32 tileChunkDimensionsBitMask;
    uint32 tileChunkDimensions;

    // Total number of tiles across one side of an individual tile chunk
    // (Tile chunks are always square)
    uint32 tileChunkTileDimensionsBitShift;
    uint32 tileChunkTileDimensionsBitMask;
    uint32 tileChunkTileDimensions;

    // Height and width in pixels of an individual tile
    // @TODO(JM) move these to renderer when we have one, tilemap shouldnt care
    // about pixels. See day 35 @45:00
    uint32 tileHeightPx;
    uint32 tileWidthPx;

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

typedef struct World World;
typedef struct GameState GameState;
void initTilemap(GameMemoryRegion memoryRegion,
                    GameState *gameState,
                    GameMemoryBlock *memoryBlock,
                    uint16 pixelsPerMeter,
                    uint32 tileDimensionsBitShift,
                    uint32 tileChunkDimensionsBitShift,
                    uint32 tileChunkTileDimensionsBitShift,
                    float32 tileDimensionsMeters);

void setCoordinateData(TilemapCoordinates *coordinates,
                        uint32 pixelX,
                        uint32 pixelY,
                        Tilemap tilemap);

void setTileColour(Colour *tileColour, uint32 tileValue);

typedef struct GameState GameState;
void setTileValue(GameMemoryRegion memoryRegion, GameState *gameState, Tilemap *tilemap, uint32 absTileX, uint32 absTileY, uint32 value);

typedef struct PlayerPositionData PlayerPositionData;
typedef struct GameState GameState;
bool isTilemapTileFree(GameState *gameState, Tilemap tilemap, PlayerPositionData *playerPositionData);

xyuint getTileChunkIndexForAbsTile(uint32 absTileX, uint32 absTileY, Tilemap tilemap);

TileChunk *getTileChunkForAbsTile(uint32 absTileX, uint32 absTileY, Tilemap tilemap);

xyuint getChunkRelativeTileIndex(uint32 absTileX, uint32 absTileY, Tilemap tilemap);

Colour getOutOfMemoryBoundsColour();
Colour getUninitialisedTileChunkTilesColour();

#endif
