#ifndef HEADER_HH_TILEMAP
#define HEADER_HH_TILEMAP

#include "types.h"
#include "utility.h"
#include "memory.h"

//
// Tilemap
//====================================================
// Tilemap is the entire game world

// How many bits of a 32-bit integer do we want to allocate to the tilemap's
// tile dimensions*? (along one side)
// 0 = 1x1
// 1 = 2x2
// 2 = 4x4 etc
#define TILE_DIMENSIONS_BIT_SHIFT 10

// How many bits of a 32-bit integer do we want to allocate to the total number
// of possible "tile chunks" dimensions*? (*along one side)
// 
// Note:
// 1) Tiles can only be stored within a tile chunk, not arbitrarily anywhere in the world 
// 2) Not all tile chunks have to be used.
#define TILE_CHUNK_DIMENSIONS_BIT_SHIFT 6

// How many z-planes should be allocated? Min 1
#define TILEMAP_Z_PLANES 3

// How many bits of a 32-bit integer do we want to allocate to each tile chunk's
// total tile dimensions*? (*along one side)
// Note that a tile chunk doesnt have to write tile data to all of its tiles.
// Temp note: A tile chunk's tiles can contain an entire room, many rooms,
// part of another room etc etc. A single tile chunk's tiles does not necessarily = 1 room
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
    // Total number of tiles across 1 whole side of the tilemap
    // (Total tilemap tile dimensions are always square)
    uint32 tileDimensionsBitShift;
    uint32 tileDimensions;

    // Total number of tile chunks requested within the tilemap
    uint32 tileChunkDimensionsBitShift;
    uint32 tileChunkDimensions;

    // Total number of tiles across one side of an individual tile chunk
    // (Tile chunks are always square)
    uint32 tileChunkTileDimensionsBitShift;
    uint32 tileChunkTileDimensions;

    // Height and width in pixels of an individual tile
    // @TODO(JM) move these to renderer when we have one, tilemap shouldnt care
    // about pixels. See day 35 @45:00
    uint32 tileHeightPx;
    uint32 tileWidthPx;

    // Pointer to all of the tile chunks
    TileChunk *tileChunks;

    uint32 tilesPerScreenX;
    uint32 tilesPerHalfScreenX;
    uint32 tilesPerScreenY;
    uint32 tilesPerHalfScreenY;

} Tilemap;

typedef struct TilemapPosition
{
    // The absolute pixel position (relative to the entire tilemap) that all
    // other members of this struct are based off of.
    xyuint absPixelPos;

    // X and Y absolute tile index (relative to the entire tilemap)
    xyzuint tileIndex;

    // X and Y index of the current tile chunk
    xyuint chunkIndex;

    // X and Y tile index relative to the tile chunk
    xyuint chunkRelativeTileIndex;

    // The tile relative pixel position
    xyuint tileRelativePixelPos;

    // Pointer to the active tile chunk
    TileChunk *tileChunk;

    uint32 *activeTile;

} TilemapPosition;

typedef struct World World;
typedef struct GameState GameState;
typedef struct GameFrameBuffer GameFrameBuffer;
typedef struct GameState GameState;

void initTilemap(MemoryRegion *memoryRegion,
                    GameState *gameState,
                    MemoryBlock *memoryBlock,
                    uint16 pixelsPerMeter,
                    uint32 tileDimensionsBitShift,
                    uint32 tileChunkDimensionsBitShift,
                    uint32 tilemapTotalZPlanes,
                    uint32 tileChunkTileDimensionsBitShift,
                    float32 tileDimensionsMeters,
                    GameFrameBuffer *frameBuffer);

void setTilemapPositionData(TilemapPosition *coordinates,
                            uint32 pixelX,
                            uint32 pixelY,
                            uint32 zIndex,
                            Tilemap tilemap);

void setTileColour(Colour *tileColour, uint32 tileValue);

void setTileValue(MemoryRegion memoryRegion,
                    GameState *gameState,
                    uint32 absTileX,
                    uint32 absTileY,
                    uint32 absTileZ,
                    uint32 value);

typedef struct PlayerPositionData PlayerPositionData;
typedef struct GameState GameState;
bool isTilemapTileFree(GameState *gameState, Tilemap tilemap, PlayerPositionData *playerPositionData);

xyuint geAbsTileIndexFromAbsPixel(uint32 pixelX, uint32 pixelY, Tilemap tilemap);

xyzuint getTileChunkIndexForAbsTile(uint32 absTileX, uint32 absTileY, uint32 absTileZ, Tilemap tilemap);

TileChunk *getTileChunkForAbsTile(uint32 absTileX, uint32 absTileY, uint32 absTileZ, Tilemap tilemap);
TileChunk *getTileChunkFromTileChunkIndex(xyzuint tileChunkIndex, Tilemap tilemap);
xyuint getChunkRelativeTileIndex(uint32 absTileX, uint32 absTileY, Tilemap tilemap);
xyuint getTileRelativePixelPos(uint32 pixelX, uint32 pixelY, Tilemap tilemap);
Colour getOutOfTileChunkMemoryBoundsColour();
Colour getUninitialisedTileChunkTilesColour();

#endif
