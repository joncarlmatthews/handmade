#include <math.h>
#include "types.h"
#include "tilemap.h"
#include "game.h"
#include "player.h"
#include "utility.h"

/**
 * @NOTE(JM) the tilemap, tile chunks and individual tiles are always square
*/
void initTilemap(GameMemoryBlock *memoryBlock,
                    World *world,
                    uint16 pixelsPerMeter,
                    uint32 tileDimensionsBitShift,
                    uint32 tileChunkDimensionsBitShift,
                    uint32 tileChunkTileDimensionsBitShift,
                    float32 tileDimensionsMeters)
{
    world->tilemap.tileDimensionsBitShift  = tileDimensionsBitShift;
    world->tilemap.tileDimensionsBitMask   = ((1 << tileDimensionsBitShift) - 1);
    world->tilemap.tileDimensions          = (1 << tileDimensionsBitShift);

    world->tilemap.tileChunkDimensionsBitShift  = tileChunkDimensionsBitShift;
    world->tilemap.tileChunkDimensionsBitMask   = ((1 << tileChunkDimensionsBitShift) - 1);
    world->tilemap.tileChunkDimensions          = (1 << tileChunkDimensionsBitShift);

    world->tilemap.tileChunkTileDimensionsBitShift  = tileChunkTileDimensionsBitShift;
    world->tilemap.tileChunkTileDimensionsBitMask   = ((1 << tileChunkTileDimensionsBitShift) - 1);
    world->tilemap.tileChunkTileDimensions          = (1 << tileChunkTileDimensionsBitShift);

    // Check that the tilemap's total possible tile dimensions are at least
    // big enough to hold the number of tile chunks and tile chunk tile dimensions
    assert(world->tilemap.tileDimensions >= (world->tilemap.tileChunkDimensions * world->tilemap.tileChunkTileDimensions))

    world->tilemap.tileHeightPx = (uint32)((uint32)pixelsPerMeter * tileDimensionsMeters);
    world->tilemap.tileWidthPx = world->tilemap.tileHeightPx;

    // Reserve the tile chunk arrays from within the memory block
    world->tilemap.tileChunks = gameMemoryBlockReserveArray(memoryBlock,
                                                            TileChunk,
                                                            (sizet)((sizet)world->tilemap.tileChunkDimensions * (sizet)world->tilemap.tileChunkDimensions));
}

void setCoordinateData(TilemapCoordinates *coordinates, uint32 pixelX, uint32 pixelY, Tilemap tilemap)
{
    coordinates->pixelCoordinates.x = pixelX;
    coordinates->pixelCoordinates.y = pixelY;

    // Calculate the absolute x and y tile index relative to the entire tilemap
    uint32 tileIndexX = (int32)floorf((float32)pixelX / (float32)tilemap.tileWidthPx);
    uint32 tileIndexY = (int32)floorf((float32)pixelY / (float32)tilemap.tileHeightPx);

    coordinates->tileIndex.x = (modulo(tileIndexX, tilemap.tileDimensions));
    coordinates->tileIndex.y = (modulo(tileIndexY, tilemap.tileDimensions));

    // Calculate the x and y tile chunk index relative to the entire tilemap
    coordinates->chunkIndex.x = (int32)floorf((float32)pixelX / (float32)(tilemap.tileWidthPx * tilemap.tileChunkTileDimensions));
    coordinates->chunkIndex.y = (int32)floorf((float32)pixelY / (float32)(tilemap.tileHeightPx * tilemap.tileChunkTileDimensions));

    // @TODO(JM)
    coordinates->chunkRelativeTileIndex.x = modulo(coordinates->tileIndex.x, tilemap.tileChunkTileDimensions);
    coordinates->chunkRelativeTileIndex.y = modulo(coordinates->tileIndex.y, tilemap.tileChunkTileDimensions);

    // @TODO(JM)
    coordinates->chunkRelativePixelCoordinates.x = 0;
    coordinates->chunkRelativePixelCoordinates.y = 0;

    // @TODO(JM)
    coordinates->tileRelativePixelCoordinates.x = 0;
    coordinates->tileRelativePixelCoordinates.y = 0;
}

xyuint getTileChunkIndexForAbsTile(uint32 absTileX, uint32 absTileY, Tilemap tilemap)
{
    xyuint tileChunkIndex = { 0 };

    tileChunkIndex.x = (absTileX / tilemap.tileChunkTileDimensions);
    tileChunkIndex.y = (absTileY / tilemap.tileChunkTileDimensions);

    return tileChunkIndex;
}

TileChunk *getTileChunkForAbsTile(uint32 absTileX, uint32 absTileY, Tilemap tilemap)
{
    xyuint tileChunkIndex = getTileChunkIndexForAbsTile(absTileX, absTileY, tilemap);
    TileChunk *tileChunk = tilemap.tileChunks;
    tileChunk = (tileChunk + (tileChunkIndex.y * tilemap.tileChunkDimensions) + tileChunkIndex.x);
    return tileChunk;
}

void setTileValue(GameState *gameState, Tilemap *tilemap, uint32 absTileX, uint32 absTileY, uint32 value)
{
    TileChunk *tileChunk = getTileChunkForAbsTile(absTileX, absTileY, *tilemap);

    // Is this tile chunk out of the sparse storage memory bounds?
    if ((uint8 *)tileChunk > gameState->tileChunkMemoryBlock.lastAddressReserved
        || (uint8 *)tileChunk < gameState->tileChunkMemoryBlock.startingAddress){
        assert(!"Cannot set tile value for an absolute tile that sits outside of the available tile chunks")
    }

    if (!tileChunk->tiles){
        tileChunk->tiles = gameMemoryBlockReserveArray(&gameState->tilesMemoryBlock,
                                                        uint32,
                                                        (sizet)((sizet)tilemap->tileChunkTileDimensions * (sizet)tilemap->tileChunkTileDimensions));

    }

    uint32 *tile = tileChunk->tiles;
    tile += (absTileY * tilemap->tileChunkTileDimensions) + absTileX;
    *tile = value;
}

bool isTilemapTileFree(GameState *gameState, Tilemap tilemap, PlayerPositionData *playerPositionData)
{
    uint32 tilePosX = playerPositionData->activeTile.tileIndex.x;
    uint32 tilePosY = playerPositionData->activeTile.tileIndex.y;

    // Get the tile chunk based off of the absolute tile index
    TileChunk *tileChunk = getTileChunkForAbsTile(tilePosX, tilePosY, tilemap);

    // Is this tile chunk out of the sparse storage memory bounds?
    if ((uint8 *)tileChunk > gameState->tileChunkMemoryBlock.lastAddressReserved
        || (uint8 *)tileChunk < gameState->tileChunkMemoryBlock.startingAddress){
#if HANDMADE_LOCAL_BUILD
        return true;
#else
        return false;
#endif
    }

    // Have the tiles within this tile chunk been initialised?
    if (!tileChunk->tiles) {
#if HANDMADE_LOCAL_BUILD
        return true;
#else
        return false;
#endif
    }

    uint32 tileNumber = (tilePosY * tilemap.tileDimensions) + tilePosX;

    uint32 *tileState = (tilemap.tileChunks->tiles + tileNumber);

    if (tileState) {
        if (*tileState == 2) {
            return false;
        }
    }

    return true;
}

void setTileColour(Colour *tileColour, uint32 tileValue)
{
    switch (tileValue) {
    default:
        *tileColour =  { (255.0f/255.0f), (255.0f/255.0f), (0.0f/255.0f) }; // Default tile value
        break;

    case 1: 
        *tileColour = { (89.0f/255.0f), (89.0f/255.0f), (89.0f/255.0f) }; // stone floor
        break;

    case 2:
        *tileColour = { (38.0f/255.0f), (38.0f/255.0f), (38.0f/255.0f) }; // stone wall
        break;

    case 3:
        *tileColour = { (77.0f/255.0f), (77.0f/255.0f), (77.0f/255.0f) }; // passageway
        break;

    case 4:
        *tileColour = { (102.0f/255.0f), (102.0f/255.0f), (51.0f/255.0f) }; // earth/grass
        break;
    }
}

Colour getOutOfMemoryBoundsColour()
{
    return { (204.0f/255.0f), (51.0f/255.0f), 0.0f }; // red
}

Colour getUninitialisedTileChunkTilesColour()
{
    return { 0.0f, (102.0f/255.0f), (255.0f/255.0f) }; // blue
}