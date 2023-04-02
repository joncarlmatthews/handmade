#ifndef HEADER_HH_GAME
#define HEADER_HH_GAME

//
// game.h
//====================================================
// Game setup definitions that are required for the game layer to have access to.
// Definitons that both the game layer and the platform layer need access to
// should reside in platform.h

#include "global_macros.h"
#include "types.h"
#include "global.h"
#include "intrinsics.h"
#include "utility_shared.h"
#include "utility.h"
#include "filesystem.h"
#include "random.h"
#include "memory.h"
#include "graphics.h"
#include "audio.h"
#include "world.h"
#include "tilemap.h"
#include "player.h"

#ifdef HANDMADE_DEBUG_TILE_POS

// Common input/output operations.
// Defines: sprintf_s
// @see https://www.cplusplus.com/reference/cstdio/
#include <stdio.h>

#endif

typedef struct SineWave
{
    // Hertz is the same as "cycles per second".
    // The number of cycles that occur in one second is equal to the
    // frequency of the signal in hertz (abbreviated Hz).
    // Hertz set the tone. E.g. 256 = middle C.
    // @link https://bit.ly/2PCUzdH
    // @link https://en.wikipedia.org/wiki/Scientific_pitch
    uint16 hertz;

    // The taller the wave, the louder the sound.
    uint16 sizeOfWave;

} SineWave;

/**
 * The GameState essentially sits inside (overlays) the memory's permanent storage
 * region's bytes
*/
typedef struct GameState
{
    Player player1;
    World world;

    MemoryBlock tileChunksMemoryBlock;
    MemoryBlock tilesMemoryBlock;

    // The currently active world position based off of the player's
    // absolute position
    TilemapPosition worldPosition;

    SineWave sineWave;

} GameState;

#endif