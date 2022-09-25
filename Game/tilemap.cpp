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

    // Reserve the tile chunk arrays from the memory block
    world->tilemap.tileChunks = gameMemoryBlockReserveArray(memoryBlock,
                                                            TileChunk,
                                                            (sizet)((sizet)world->tilemap.tileChunkDimensions * (sizet)world->tilemap.tileChunkDimensions));


    // Reserve the tiles within each tile chunk from the memory block
    #if 0
    sizet tilesStored = 0;
    for (size_t tileChunkY = 0; tileChunkY < world->tilemap.tileChunkDimensions; tileChunkY++) {
        for (size_t tileChunkX = 0; tileChunkX < world->tilemap.tileChunkDimensions; tileChunkX++) {
            world->tilemap.tileChunks[(tileChunkY * world->tilemap.tileChunkDimensions) + tileChunkX].tiles = (uint32 *)GameMemoryBlockReserveArray(memoryBlock,
                                                                                                                                                    sizeof(uint32),
                                                                                                                                                    (sizet)((sizet)world->tilemap.tileChunkTileDimensions * (sizet)world->tilemap.tileChunkTileDimensions));
            tilesStored += (world->tilemap.tileChunkTileDimensions * world->tilemap.tileChunkTileDimensions);
        }
    }

    world->tilemap.tilesStoredDimensions = (tilesStored / 2);
    #endif
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

void setTileColour(Colour *tileColour, uint32 tileValue)
{
    switch (tileValue) {
    default: // no tile value set...
        *tileColour = { (105.0f/255.0f), (153.0f/255.0f), 0.0f };
        break;

    case 1: 
        *tileColour = { (89.0f/255.0f), (89.0f/255.0f), (89.0f/255.0f) }; // stone floor
        break;

    case 2:
        *tileColour = { (38.0f/255.0f), (38.0f/255.0f), (38.0f/255.0f) }; // stone wall
        break;

    case 3:
        *tileColour = { (77.0f/255.0f), (77.0f/255.0f), (77.0f/255.0f) }; //passageway
        break;

    case 4:
        *tileColour = { 0.26f, 0.26f, 0.13f }; // earth/grass
        break;

    case 5:
        *tileColour = { 0.96f, 0.15f, 0.15f };
        break;

    case 6:
        *tileColour = { 0.25f, 1.0f, 0.0f };
        break;

    case 10:
        *tileColour = { (230.0f/255.0f), 0.f, 0.f };
        break;
    }
}

void setTileValue(GameMemoryBlock *memoryBlock, Tilemap *tilemap, uint32 absTileX, uint32 absTileY, uint32 value)
{
    uint32 tileChunkX = (absTileX / tilemap->tileChunkTileDimensions);
    uint32 tileChunkY = (absTileY / tilemap->tileChunkTileDimensions);

    if (!tilemap->tileChunks[(tileChunkY * tilemap->tileChunkDimensions) + tileChunkX].tiles){
        tilemap->tileChunks[(tileChunkY * tilemap->tileChunkDimensions) + tileChunkX].tiles =
            gameMemoryBlockReserveArray(memoryBlock,
                                        uint32,
                                        (sizet)((sizet)tilemap->tileChunkTileDimensions * (sizet)tilemap->tileChunkTileDimensions));

        tilemap->tilesStoredDimensions = tilemap->tileChunkTileDimensions;
    }

    uint32 *tile = tilemap->tileChunks[(tileChunkY * tilemap->tileChunkDimensions) + tileChunkX].tiles;
    tile += (absTileY * tilemap->tileDimensions) + absTileX;
    *tile = value;

    #if 0
    uint32 *tile = tilemap->tileChunks[(tileChunkY * tilemap->tileChunkDimensions) + tileChunkX].tiles;
    for (size_t y = 0; y < tilemap->tileChunkTileDimensions; y++){
        for (size_t x = 0; x < tilemap->tileChunkTileDimensions; x++){
            //*tile = 1;
            tile +=1;
        }
    }
    #endif
}

bool isTilemapTileFree(Tilemap tilemap, PlayerPositionData *playerPositionData)
{
    // Out of sparse storage memory bounds?
    if ((playerPositionData->activeTile.tileIndex.y > (tilemap.tilesStoredDimensions -1))
        || (playerPositionData->activeTile.tileIndex.x > (tilemap.tilesStoredDimensions -1)) ) {
        return false;
    }

    uint32 tileNumber = (playerPositionData->activeTile.tileIndex.y * tilemap.tileDimensions) + playerPositionData->activeTile.tileIndex.x;

    uint32 *tileState = (tilemap.tileChunks->tiles + tileNumber);

    if (tileState) {
        if (*tileState == 2) {
            return false;
        }
    }

    return true;
}