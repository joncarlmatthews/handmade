#ifndef HEADER_HH_TILEMAP
#define HEADER_HH_TILEMAP

#include "types.h"

//
// World/Tilemaps
//====================================================
typedef struct TileChunk {
    uint32 *tiles;
} TileChunk;

// How many tiles does one side of the entire World have?
// 30x30 = 900
#define WORLD_TOTAL_TILE_DIMENSIONS 30

// How many tiles does one side of a "tile chunk" have?
// 10x10 = 100
#define WORLD_TILE_CHUNK_DIMENSIONS 15

// How many meters does one side of a tile have?
// 2x2 = 4
#define TILE_DIMENSIONS_METERS 2.0f

#endif
