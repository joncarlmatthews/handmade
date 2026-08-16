/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct world_position
{
    // TODO(casey): It seems like we have to store ChunkX/Y with each
    // entity because even though the sim region gather doesn't need it
    // at first, and we could get by without it, entity references pull
    // in entities WITHOUT going through their world_chunk, and thus
    // still need to know the ChunkX/Y

    v2s Chunk;

    // NOTE(casey): These are the offsets from the chunk center
    v3 Offset_;
};

struct world_region
{
    world_position SimCenter;
    v2 SimRadius;
};

struct entity_block
{
    entity_block *Next;
    u32 UsedByteCount;
    u8 Bytes[16*sizeof(entity)];
};

struct world_chunk
{
    world_chunk *NextInHash;

    // TODO(casey): Should we just have one pointer and store whether it's packed or unpacked?
    entity_block *FirstPacked;
    entity_block *FirstUnpacked;

    /* NOTE(casey): The world is broken up into chunks where the center of each chunk is the
       point considered to correspond to that chunk. The dimension of the chunk is then the
       bounding volume around that point given by world::ChunkDimInMeters. */
    v2s ChunkP;

    // TODO(casey): Probably need to track access patterns here so we know what to repack for lack of use
};

struct world_room
{
    world_position MinPos;
    world_position MaxPos;
    v3 DebugColor;
};

struct world_room_connection
{
    u32 RoomA;
    u32 RoomB;
    v3 DebugColor;
};

struct world
{
    v3 ChunkDimInMeters;
    random_series GameEntropy; // NOTE(casey): This is entropy that DOES affect the gameplay

    u32 LastUsedEntityStorageIndex; // TODO(casey): Worry about this wrapping - free list for IDs?

    // TODO(casey): WorldChunkHash should probably switch to pointers IF
    // tile entity blocks continue to be stored en masse directly in the tile chunk!
    // NOTE(casey): A the moment, this must be a power of two!
    world_chunk *ChunkHash[4096];

    memory_arena *Arena;

    world_chunk *FirstFreeChunk;
    entity_block *FirstFreeBlock;

    entity *NullEntity;

    // TODO(casey): Temporary - eventually these will be spatially partitioned, probably?
    u32 RoomCount;
    world_room Rooms[65536];
    u32 ConnectionCount;
    world_room_connection Connections[65536];
    
    u32 UnpackedRegionCount;
    rectangle2i UnpackedRegions[16];
};

struct world_chunk_iterator
{
    rectangle2i Bounds;
    v2s P;
    world_chunk *Chunk;
};
function world_chunk_iterator IterateChunks(world *World,
                                            world_position MinChunkP,
                                            world_position MaxChunkP // NOTE(casey): MaxChunkP is INCLUDED in the range
                                            );
function b32 IsValid(world_chunk_iterator Iter);
function world_chunk_iterator Next(world *World, world_chunk_iterator Iter);

function bool32 AreInSameChunk(world *World, world_position *A, world_position *B);
function v3 Subtract(world *World, world_position *A, world_position *B);

struct entity_iterator
{

};
function entity_iterator IterateEntitiesIn(world *World, world_region Region);

struct closest_entity
{
    entity *Entity;
    v3 Delta;
    r32 DistanceSq;
};

struct tile_result
{
    b32 Valid;
    v2s Index;
};