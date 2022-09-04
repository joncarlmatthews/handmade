#include "types.h"
#include "tilemap.h"
#include "tilemaps.h"
#include "player.h"

void initTilemap(Tilemap *tilemap,
                    uint16 pixelsPerMetre,
                    float32 tileDimensionsMeters,
                    uint16 totalTileDimensions,
                    uint16 tileChunkDimensions)
{
    tilemap->totalTileDimensions = totalTileDimensions;
    tilemap->tileChunkDimensions = tileChunkDimensions;
    tilemap->totalTileChunks = ((tilemap->totalTileDimensions / tilemap->tileChunkDimensions) * 2);

    // @TODO(JM)
    //tilemap->tileChunkMask = 0xFF;
    //tilemap->tileChunkShift = 8;

    // @NOTE(JM) tiles and tile chunks are always square
    tilemap->tileHeightPx = (uint16)(pixelsPerMetre * tileDimensionsMeters);
    tilemap->tileWidthPx = tilemap->tileHeightPx;
    tilemap->tileChunkHeightPx = (tilemap->tileHeightPx * tilemap->tileChunkDimensions);
    tilemap->tileChunkWidthPx = tilemap->tileChunkHeightPx;

    // @NOTE(JM) not sure how to allocate this on the heap, as this array is
    // too large for the stack...
    uint8 origTiles[TOTAL_TILE_DIMENSIONS][TOTAL_TILE_DIMENSIONS] = ALL_TILES;

    // Copy the tiles into the world tiles, making it so that the Y axis
    // goes up
    uint32 worldY = 0;
    for (int32 y = (TOTAL_TILE_DIMENSIONS-1); y >= 0; y--){
        for (uint x = 0; x < TOTAL_TILE_DIMENSIONS; x++) {
            tilemap->tiles[worldY][x] = (uint32)origTiles[y][x];
        }
        worldY++;
    }
}

bool isTilemapTileFree(Tilemap tilemap, PlayerPositionData *playerPositionData)
{
    uint32 tileNumber = (playerPositionData->activeTile.tileIndex.y * tilemap.totalTileDimensions) + playerPositionData->activeTile.tileIndex.x;

    uint32 *tileState = ((uint32*)tilemap.tiles + tileNumber);

    if (tileState) {
        if (*tileState == 2) {
            return false;
        }
    }

    return true;
}