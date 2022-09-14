#include <math.h>
#include "types.h"
#include "tilemap.h"
#include "game.h"
#include "player.h"
#include "utility.h"

void initTilemap(GameMemoryBlock *memoryBlock,
                    World *world,
                    uint16 pixelsPerMeter,
                    uint16 tileChunkDimensions,
                    uint16 tileChunkTileDimensions,
                    float32 tileDimensionsMeters)
{
    world->tilemap.tileChunkDimensions = tileChunkDimensions;
    world->tilemap.totalTileChunks = (world->tilemap.tileChunkDimensions * world->tilemap.tileChunkDimensions);
    world->tilemap.tileChunkTileDimensions = tileChunkTileDimensions;
    world->tilemap.tileDimensions = (world->tilemap.tileChunkDimensions * world->tilemap.tileChunkTileDimensions);
    world->tilemap.totalTiles = (world->tilemap.totalTileChunks * (world->tilemap.tileChunkTileDimensions * world->tilemap.tileChunkTileDimensions));

    // @TODO(JM)
    //world->tilemap.tileChunkMask = 0xFF;
    //world->tilemap.tileChunkShift = 8;

    // @NOTE(JM) tiles and tile chunks are always square
    world->tilemap.tileHeightPx = (uint16)(pixelsPerMeter * tileDimensionsMeters);
    world->tilemap.tileWidthPx = world->tilemap.tileHeightPx;
    world->tilemap.tileChunkHeightPx = (world->tilemap.tileHeightPx * world->tilemap.tileChunkTileDimensions);
    world->tilemap.tileChunkWidthPx = world->tilemap.tileChunkHeightPx;

    // Reserve the tile chunk arrays from the memory block
    world->tilemap.tileChunks = (TileChunk *)GameMemoryBlockReserveArray(memoryBlock,
                                                                        sizeof(TileChunk),
                                                                        world->tilemap.totalTileChunks);


    // Reserve the tiles within each tile chunk from the memory block
    for (size_t tileChunkY = 0; tileChunkY < world->tilemap.tileChunkDimensions; tileChunkY++) {
        for (size_t tileChunkX = 0; tileChunkX < world->tilemap.tileChunkDimensions; tileChunkX++) {
            world->tilemap.tileChunks[(tileChunkY * world->tilemap.tileChunkDimensions) + tileChunkX].tiles = (uint32 *)GameMemoryBlockReserveArray(memoryBlock,
                                                                                                                                                    sizeof(uint32),
                                                                                                                                                    (world->tilemap.tileChunkTileDimensions * world->tilemap.tileChunkTileDimensions));
        }
    }
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

bool isTilemapTileFree(Tilemap tilemap, PlayerPositionData *playerPositionData)
{
    uint32 tileNumber = (playerPositionData->activeTile.tileIndex.y * tilemap.tileDimensions) + playerPositionData->activeTile.tileIndex.x;

    uint32 *tileState = (tilemap.tileChunks->tiles + tileNumber);

    if (tileState) {
        if (*tileState == 2) {
            return false;
        }
    }

    return true;
}