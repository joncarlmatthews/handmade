#include "tilemap.h"
#include "game.h"
#include "player.h"

/**
 * @NOTE(JM) the tilemap, tile chunks and individual tiles are always square
*/
void initTilemap(MemoryRegion *memoryRegion,
                    GameState *gameState,
                    MemoryBlock *memoryBlock,
                    uint16 pixelsPerMeter,
                    uint32 tileDimensionsBitShift,
                    uint32 tileChunkDimensionsBitShift,
                    uint32 tilemapTotalZPlanes,
                    uint32 tileChunkTileDimensionsBitShift,
                    float32 tileDimensionsMeters,
                    GameFrameBuffer *frameBuffer)
{
    assert(tilemapTotalZPlanes > 0);

    gameState->world.tilemap.tileDimensionsBitShift     = tileDimensionsBitShift;
    gameState->world.tilemap.tileDimensions             = (1 << tileDimensionsBitShift);

    gameState->world.tilemap.tileChunkDimensionsBitShift    = tileChunkDimensionsBitShift;
    gameState->world.tilemap.tileChunkDimensions            = (1 << tileChunkDimensionsBitShift);

    gameState->world.tilemap.tileChunkTileDimensionsBitShift    = tileChunkTileDimensionsBitShift;
    gameState->world.tilemap.tileChunkTileDimensions            = (1 << tileChunkTileDimensionsBitShift);

    // Check that the tilemap's total possible tile dimensions are at least
    // big enough to hold the number of tile chunks and tile chunk tile dimensions
    assert(gameState->world.tilemap.tileDimensions >= (gameState->world.tilemap.tileChunkDimensions * gameState->world.tilemap.tileChunkTileDimensions))

    gameState->world.tilemap.tileHeightPx = (uint32)((uint32)pixelsPerMeter * tileDimensionsMeters);
    gameState->world.tilemap.tileWidthPx = gameState->world.tilemap.tileHeightPx;

    // Calculate how many rows/columns we can fit on a screen and add
    // margin of safety for smooth scrolling.
    gameState->world.tilemap.tilesPerScreenX = (u32RoundUpDivide(frameBuffer->widthPx, gameState->world.tilemap.tileWidthPx) + 2);
    gameState->world.tilemap.tilesPerHalfScreenX = i32RoundUpDivide((int32)gameState->world.tilemap.tilesPerScreenX, 2);

    gameState->world.tilemap.tilesPerScreenY = (u32RoundUpDivide(frameBuffer->heightPx, gameState->world.tilemap.tileHeightPx) + 2);
    gameState->world.tilemap.tilesPerHalfScreenY = i32RoundUpDivide((int32)gameState->world.tilemap.tilesPerScreenY, 2);

    // Reserve the tile chunk arrays from within the memory block
    gameState->world.tilemap.tileChunks = memoryBlockReserveArray(memoryRegion,
                                                                    memoryBlock,
                                                                    TileChunk,
                                                                    (sizet)((gameState->world.tilemap.tileChunkDimensions * gameState->world.tilemap.tileChunkDimensions) * tilemapTotalZPlanes));
}

/**
 * @brief For any given absolute pixel, this function will calculate the absolute tile
 * index, tile chunk index, chunk relative tile index and the tile relative
 * pixel position and write it into a TilemapPosition struct pointer.
 * 
 * @param tilemapPosition 
 * @param absPixelX 
 * @param absPixelY 
 * @param tilemap 
*/
void setTilemapPositionData(TilemapPosition *tilemapPosition,
                            uint32 absPixelX,
                            uint32 absPixelY,
                            uint32 zIndex,
                            Tilemap tilemap)
{
    tilemapPosition->absPixelPos.x = absPixelX;
    tilemapPosition->absPixelPos.y = absPixelY;

    // Calculate the absolute x and y tile index relative to the entire tilemap
    xyuint absTileIndex = geAbsTileIndexFromAbsPixel(absPixelX,
                                                        absPixelY,
                                                        tilemap);

    tilemapPosition->tileIndex.x = absTileIndex.x;
    tilemapPosition->tileIndex.y = absTileIndex.y;

    // Calculate the x and y tile chunk index relative to the entire tilemap
    xyzuint chunkIndex = getTileChunkIndexForAbsTile(absTileIndex.x,
                                                        absTileIndex.y,
                                                        zIndex,
                                                        tilemap);

    tilemapPosition->chunkIndex.x = chunkIndex.x;
    tilemapPosition->chunkIndex.y = chunkIndex.y;

    // Get the tile index relative to the tile chunk it's within
    xyuint chunkRelativeTileIndex = getChunkRelativeTileIndex(absTileIndex.x,
                                                                absTileIndex.y,
                                                                tilemap);

    tilemapPosition->chunkRelativeTileIndex.x = chunkRelativeTileIndex.x;
    tilemapPosition->chunkRelativeTileIndex.y = chunkRelativeTileIndex.y;

    xyuint tileRelPos = getTileRelativePixelPos(absPixelX, absPixelY, tilemap);

    tilemapPosition->tileRelativePixelPos.x = tileRelPos.x;
    tilemapPosition->tileRelativePixelPos.y = tileRelPos.y;

    // Get the TileChunk
    tilemapPosition->tileChunk = getTileChunkFromTileChunkIndex(chunkIndex, tilemap);

    // Get the active tile
    uint32 *tile = tilemapPosition->tileChunk->tiles;
    tile += (chunkRelativeTileIndex.y * tilemap.tileChunkTileDimensions) + chunkRelativeTileIndex.x;
    tilemapPosition->activeTile = tile;

    return;
}

xyuint geAbsTileIndexFromAbsPixel(uint32 pixelX, uint32 pixelY, Tilemap tilemap)
{
    xyuint tileIndex = {};
    tileIndex.x = (pixelX / tilemap.tileWidthPx);
    tileIndex.y = (pixelY / tilemap.tileHeightPx);
    return tileIndex;
}

xyzuint getTileChunkIndexForAbsTile(uint32 absTileX, uint32 absTileY, uint32 absTileZ, Tilemap tilemap)
{
    xyzuint tileChunkIndex = { 0 };

    tileChunkIndex.x = (absTileX / tilemap.tileChunkTileDimensions);
    tileChunkIndex.y = (absTileY / tilemap.tileChunkTileDimensions);
    tileChunkIndex.z = absTileZ;

    return tileChunkIndex;
}

TileChunk *getTileChunkForAbsTile(uint32 absTileX, uint32 absTileY, uint32 absTileZ, Tilemap tilemap)
{
    xyzuint tileChunkIndex = getTileChunkIndexForAbsTile(absTileX, absTileY, absTileZ, tilemap);
    return getTileChunkFromTileChunkIndex(tileChunkIndex, tilemap);
}

TileChunk *getTileChunkFromTileChunkIndex(xyzuint tileChunkIndex, Tilemap tilemap)
{
    TileChunk *tileChunk = tilemap.tileChunks;
    tileChunk = tileChunk + (tileChunkIndex.z * (tilemap.tileChunkDimensions * tilemap.tileChunkDimensions)) + (tileChunkIndex.y * tilemap.tileChunkDimensions) + tileChunkIndex.x;
    return tileChunk;
}


xyuint getChunkRelativeTileIndex(uint32 absTileX, uint32 absTileY, Tilemap tilemap)
{
    xyuint index = { 0 };

    index.x = (absTileX % tilemap.tileChunkTileDimensions);
    index.y = (absTileY % tilemap.tileChunkTileDimensions);

    return index;
}

xyuint getTileRelativePixelPos(uint32 pixelX, uint32 pixelY, Tilemap tilemap)
{
    xyuint pixelPos = {};

    pixelPos.x = (pixelX % tilemap.tileWidthPx);
    pixelPos.y = (pixelY % tilemap.tileHeightPx);

    return pixelPos;
}

void setTileValue(MemoryRegion memoryRegion,
                    GameState *gameState,
                    uint32 absTileX,
                    uint32 absTileY,
                    uint32 absTileZ,
                    uint32 value)
{
    Tilemap tilemap = gameState->world.tilemap;

    TileChunk *tileChunk = getTileChunkForAbsTile(absTileX, absTileY, absTileZ, tilemap);

    // Is this tile chunk out of the sparse storage memory bounds?
#if HANDMADE_LOCAL_BUILD
    if ((uint8 *)tileChunk > gameState->tileChunksMemoryBlock.lastAddressReserved
        || (uint8 *)tileChunk < gameState->tileChunksMemoryBlock.startingAddress){
        assert(!"Cannot set tile value for an absolute tile that sits outside of the available tile chunks")
    }
#endif

    if (!tileChunk->tiles){
        tileChunk->tiles = memoryBlockReserveArray(&memoryRegion,
                                                    &gameState->tilesMemoryBlock,
                                                    uint32,
                                                    (sizet)(tilemap.tileChunkTileDimensions * tilemap.tileChunkTileDimensions));

    }

    xyuint chunkRelTileIndex = getChunkRelativeTileIndex(absTileX, absTileY, tilemap);

    uint32 *tile = tileChunk->tiles;
    tile += (chunkRelTileIndex.y * tilemap.tileChunkTileDimensions) + chunkRelTileIndex.x;
    *tile = value;
}


bool isTilemapTileFree(GameState *gameState, Tilemap tilemap, PlayerPositionData *playerPositionData)
{

#ifdef HANDMADE_WALK_THROUGH_WALLS
    return true;
#else

    // Is this tile chunk out of the sparse storage memory bounds?
    if ( (playerPositionData->tilemapPosition.chunkIndex.x > (tilemap.tileChunkDimensions -1))
        || (playerPositionData->tilemapPosition.chunkIndex.y > (tilemap.tileChunkDimensions -1))){
        return false;
    }

    if ((uint8 *)playerPositionData->tilemapPosition.tileChunk > gameState->tileChunksMemoryBlock.lastAddressReserved
        || (uint8 *)playerPositionData->tilemapPosition.tileChunk < gameState->tileChunksMemoryBlock.startingAddress){
        return false;
    }

    // Have the tiles within this tile chunk been initialised?
    if (!playerPositionData->tilemapPosition.tileChunk->tiles) {
        return false;
    }

    if (!playerPositionData->tilemapPosition.activeTile) {
        return false;
    }

    if (*playerPositionData->tilemapPosition.activeTile == 2) {
        return false;
    }

    return true;

#endif
}

void setTileColour(Colour *tileColour, uint32 tileValue)
{
    switch (tileValue) {
    default:
        // Allocated tile memory, but no value set. (Yellow)
        *tileColour =  { (255.0f/255.0f), (255.0f/255.0f), (0.0f/255.0f) };
        break;

    case 1:
        // stone floor
        *tileColour = { (89.0f/255.0f), (89.0f/255.0f), (89.0f/255.0f) }; 
        break;

    case 2:
        // stone wall
        *tileColour = { (38.0f/255.0f), (38.0f/255.0f), (38.0f/255.0f) }; 
        break;

    case 3:
        // passageway
        *tileColour = { (77.0f/255.0f), (77.0f/255.0f), (77.0f/255.0f) }; 
        break;

    case 4:
        // earth/grass
        *tileColour = { (102.0f/255.0f), (102.0f/255.0f), (51.0f/255.0f) }; 
        break;

    case 5:
        // stairwell up
        *tileColour = { (57.0f/255.0f), (57.0f/255.0f), (57.0f/255.0f) }; 
        break;

    case 6:
        // stairwell down
        *tileColour = { (0/255.0f), (0/255.0f), (0/255.0f) }; 
        break;
    }
}

Colour getOutOfTileChunkMemoryBoundsColour()
{
    return { (1.0f/255.0f), (2.0f/255.0f), (172.0f/255.0f) }; // blue
}

Colour getUninitialisedTileChunkTilesColour()
{
    return { (204.0f/255.0f), (51.0f/255.0f), 0.0f }; // red
}