/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

function b32 IsContainedInChunkVolume(world_position MinChunk, world_position TestChunk, world_position MaxChunk)
{
    b32x Result = ((TestChunk.Chunk.x >= MinChunk.Chunk.x) &&
                   (TestChunk.Chunk.y >= MinChunk.Chunk.y) &&
                   (TestChunk.Chunk.x <= MaxChunk.Chunk.x) &&
                   (TestChunk.Chunk.y <= MaxChunk.Chunk.y));
    
    return(Result);
}

function bool32
IsCanonical(real32 ChunkDim, real32 TileRel)
{
    // TODO(casey): Fix floating point math so this can be exact?
    real32 Epsilon = 0.01f;
    bool32 Result = ((TileRel >= -(0.5f*ChunkDim + Epsilon)) &&
                     (TileRel <= (0.5f*ChunkDim + Epsilon)));
    
    return(Result);
}

function bool32
IsCanonical(world *World, v3 Offset)
{
    bool32 Result = (IsCanonical(World->ChunkDimInMeters.x, Offset.x) &&
                     IsCanonical(World->ChunkDimInMeters.y, Offset.y) &&
                     IsCanonical(World->ChunkDimInMeters.z, Offset.z));
    
    return(Result);
}

function bool32
AreInSameChunk(world *World, world_position *A, world_position *B)
{
    Assert(IsCanonical(World, A->Offset_));
    Assert(IsCanonical(World, B->Offset_));
    
    bool32 Result = ((A->Chunk.x == B->Chunk.x) &&
                     (A->Chunk.y == B->Chunk.y));
    
    return(Result);
}

function world_chunk **
GetWorldChunkInternal(world *World, v2s ChunkP)
{
    // TODO(casey): Think about what the real safe margin is!
#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX - 1024)
    Assert(ChunkP.x > -TILE_CHUNK_SAFE_MARGIN);
    Assert(ChunkP.y > -TILE_CHUNK_SAFE_MARGIN);
    Assert(ChunkP.x < TILE_CHUNK_SAFE_MARGIN);
    Assert(ChunkP.y < TILE_CHUNK_SAFE_MARGIN);
    
    // TODO(casey): BETTER HASH FUNCTION!!!!
    u32 HashValue = 19*ChunkP.x + 7*ChunkP.y;
    u32 HashSlot = HashValue & (ArrayCount(World->ChunkHash) - 1);
    Assert(HashSlot < ArrayCount(World->ChunkHash));
    
    world_chunk **Chunk = &World->ChunkHash[HashSlot];
    while(*Chunk &&
          !((ChunkP.x == (*Chunk)->ChunkP.x) &&
            (ChunkP.y == (*Chunk)->ChunkP.y)))
    {
        Chunk = &(*Chunk)->NextInHash;
    }
    
    return(Chunk);
}

function world_chunk *GetWorldChunk(world *World, v2s ChunkP, memory_arena *Arena = 0)
{
    world_chunk **ChunkPtr = GetWorldChunkInternal(World, ChunkP);
    world_chunk *Result = *ChunkPtr;
    if(!Result && Arena)
    {
        if(!World->FirstFreeChunk)
        {
            u32 WorldBlockSize = 4096*64;
            u32 ChunkCountPerBlock = WorldBlockSize / sizeof(world_chunk);
            world_chunk *ChunkArray = PushArray(Arena, ChunkCountPerBlock, world_chunk, NoClear());
            for(u32 ChunkIndex = 0;
                ChunkIndex < ChunkCountPerBlock;
                ++ChunkIndex)
            {
                world_chunk *NewChunk = ChunkArray + ChunkIndex;
                NewChunk->NextInHash = World->FirstFreeChunk;
                World->FirstFreeChunk = NewChunk;
            }
        }
        
        Result = World->FirstFreeChunk;
        World->FirstFreeChunk = Result->NextInHash;
        
        Result->FirstPacked = 0;
        Result->FirstUnpacked = 0;
        Result->ChunkP = ChunkP;
                
        Result->NextInHash = *ChunkPtr;
        *ChunkPtr = Result;
    }
    
    return Result;
}

function world *CreateWorld(v3 ChunkDimInMeters, memory_arena *ParentArena, u32 InitialSeed)
{
    world *World = PushStruct(ParentArena, world);
    
    World->ChunkDimInMeters = ChunkDimInMeters;
    World->Arena = ParentArena;
    World->GameEntropy = RandomSeed(InitialSeed);
    World->LastUsedEntityStorageIndex = ReservedBrainID_FirstFree;
    
    World->NullEntity = PushStruct(World->Arena, entity);
    
    return World;
}

function void
RecanonicalizeCoord(real32 ChunkDim, int32 *Tile, real32 *TileRel)
{
    // TODO(casey): Need to do something that doesn't use the divide/multiply method
    // for recanonicalizing because this can end up rounding back on to the tile
    // you just came from.
    
    // NOTE(casey): Wrapping IS NOT ALLOWED, so all coordinates are assumed to be
    // within the safe margin!
    // TODO(casey): Assert that we are nowhere near the edges of the world.
    
    int32 Offset = RoundReal32ToInt32(*TileRel / ChunkDim);
    *Tile += Offset;
    *TileRel -= (r32)Offset*ChunkDim;
    
    Assert(IsCanonical(ChunkDim, *TileRel));
}

function world_position MapIntoChunkSpace(world *World, world_position BasePos, v3 Offset)
{
    world_position Result = BasePos;
    
    Result.Offset_ += Offset;
    
    RecanonicalizeCoord(World->ChunkDimInMeters.x, &Result.Chunk.x, &Result.Offset_.x);
    RecanonicalizeCoord(World->ChunkDimInMeters.y, &Result.Chunk.y, &Result.Offset_.y);
    
    return(Result);
}

function v3 Subtract(world *World, world_position *A, world_position *B)
{
    v3 dTile = V3(V2(A->Chunk - B->Chunk), 0.0f);
    v3 Result = World->ChunkDimInMeters*dTile + (A->Offset_ - B->Offset_);
    
    return Result;
}

function b32 HasRoomFor(entity_block *Block, u32 Size)
{
    b32 Result = ((Block->UsedByteCount + Size) <= sizeof(Block->Bytes));
    return Result;
}

function void EnsureFreeBlockExists(world *World)
{
    if(!World->FirstFreeBlock)
    {
        World->FirstFreeBlock = PushStruct(World->Arena, entity_block);
        World->FirstFreeBlock->Next = 0;
    }
}

function void AddToFreeList(world *World, entity_block *OldBlock)
{
    Assert(OldBlock);
    OldBlock->Next = World->FirstFreeBlock;
    World->FirstFreeBlock = OldBlock;
}

function rectangle3 GetWorldChunkBounds(world *World, s32 ChunkX, s32 ChunkY, s32 ChunkZ)
{
    v3 ChunkCenter = World->ChunkDimInMeters*V3((f32)ChunkX, (f32)ChunkY, (f32)ChunkZ);
    rectangle3 Result = RectCenterDim(ChunkCenter, World->ChunkDimInMeters);
    
    return(Result);
}

function world_room *AddWorldRoom(world *World, 
                                  world_position MinPos,
                                  world_position MaxPos,
                                  v3 DebugColor)
{
    Assert(World->RoomCount < ArrayCount(World->Rooms));
    world_room *Room = World->Rooms + World->RoomCount++;
    Room->MinPos = MinPos;
    Room->MaxPos = MaxPos;
    Room->DebugColor = DebugColor;
    
    return(Room);
}

function world_room_connection *AddWorldConnection(world *World, u32 RoomA, u32 RoomB, v3 DebugColor)
{
    Assert(World->ConnectionCount < ArrayCount(World->Connections));
    world_room_connection *Con = World->Connections + World->ConnectionCount++;
    Con->RoomA = RoomA;
    Con->RoomB = RoomB;
    Con->DebugColor = DebugColor;
    
    return Con;
}

function brain_id AddBrain(world *World)
{
    brain_id ID = {++World->LastUsedEntityStorageIndex};
    return(ID);
}

function entity_id AllocateEntityID(world *World)
{
    entity_id Result = {++World->LastUsedEntityStorageIndex};
    return(Result);
}

function void MoveEntityToNewPosition(world *World, world_chunk *Chunk, u32 EntityIndex, 
                                      world_position NewPos)
{
    NotImplemented;
}

function b32 ChunkIsValid(world_chunk *Chunk)
{
    b32 Result = !(Chunk->FirstPacked && Chunk->FirstUnpacked);
    return Result;
}

function u8 *UseBlockSpace(world *World, entity_block **BlockPtr, u32 Size)
{
    if(!(*BlockPtr) || !HasRoomFor(*BlockPtr, Size))
    {
        EnsureFreeBlockExists(World);
        entity_block *NewPacked = World->FirstFreeBlock;
        World->FirstFreeBlock = NewPacked->Next;
        NewPacked->UsedByteCount = 0;
        NewPacked->Next = *BlockPtr;
        *BlockPtr = NewPacked;
    }
    
    entity_block *Packed = *BlockPtr;
    
    Assert(HasRoomFor(Packed, Size));
    u8 *Result = Packed->Bytes + Packed->UsedByteCount;
    Packed->UsedByteCount += Size;
    
    return Result;
}

function void EnsureChunkIsUnpacked(world *World, world_chunk *Chunk)
{
    Assert(ChunkIsValid(Chunk));
    
    u16 PackedEntitySize = OffsetOf(entity, DiscardEverythingAfter);
    while(Chunk->FirstPacked)
    {
        entity_block *Packed = Chunk->FirstPacked;

        for(u16 At = 0; At < Packed->UsedByteCount;)
        {
            entity *Dest = (entity *)UseBlockSpace(World, &Chunk->FirstUnpacked, sizeof(entity));
            
            Copy(PackedEntitySize, Packed->Bytes + At, Dest);
            ZeroSize(sizeof(entity) - PackedEntitySize, (u8 *)Dest + PackedEntitySize);
            At += PackedEntitySize;
            Dest->Flags &= ~EntityFlag_Unpacked;                
        }
        
        Chunk->FirstPacked = Packed->Next;
        AddToFreeList(World, Packed);    
    }
    
    Assert(ChunkIsValid(Chunk));
}

function void EnsureChunkIsPacked(world *World, world_chunk *Chunk)
{
    Assert(ChunkIsValid(Chunk));
    
    u16 PackedEntitySize = OffsetOf(entity, DiscardEverythingAfter);
    while(Chunk->FirstUnpacked)
    {
        entity_block *Unpacked = Chunk->FirstUnpacked;
        u32 EntityCount = Unpacked->UsedByteCount / sizeof(entity);
        for(u32 EntityIndex = 0; EntityIndex < EntityCount; ++EntityIndex)
        {
            entity *Entity = (entity *)Unpacked->Bytes + EntityIndex;
            if(!(Entity->Flags & EntityFlag_Deleted))
            {
                u8 *Dest = UseBlockSpace(World, &Chunk->FirstPacked, PackedEntitySize);
                Copy(PackedEntitySize, Entity, Dest);
            }
        }
        
        Chunk->FirstUnpacked = Unpacked->Next;
        AddToFreeList(World, Unpacked);    
    }

    Assert(ChunkIsValid(Chunk));
}

function entity *CreateEntity(world *World, entity_id ID, world_position AtP)
{
    world_chunk *Chunk = GetWorldChunk(World, AtP.Chunk, World->Arena);
    EnsureChunkIsUnpacked(World, Chunk);
    
    entity *Result = (entity *)UseBlockSpace(World, &Chunk->FirstUnpacked, sizeof(entity));
    ZeroStruct(*Result);
    
    Result->ID = ID;
    Result->XAxis = V2(1, 0);
    Result->YAxis = V2(0, 1);
    
    return Result;
}

function entity *CreateEntity(world *World, world_position AtP)
{
    entity *Entity = CreateEntity(World, AllocateEntityID(World), AtP);
    return(Entity);
}

function b32 IsValid(world_chunk_iterator Iter)
{
    b32 Result = (Iter.P.y <= Iter.Bounds.Max.y);
    return Result;
}

function void EnsureValidChunk(world *World, world_chunk_iterator *Result)
{
    while(!Result->Chunk)
    {
        ++Result->P.x;
        if(Result->P.x > Result->Bounds.Max.x)
        {
            ++Result->P.y;
            Result->P.x = Result->Bounds.Min.x;
            if(!IsValid(*Result))
            {
                break;
            }
        }
        
        Result->Chunk = *GetWorldChunkInternal(World, Result->P);
    }
}

function world_chunk_iterator IterateChunks(world *World, 
                                            world_position MinChunkP, world_position MaxChunkP)
{
    world_chunk_iterator Result = {};
    Result.Bounds = RectMinMax(MinChunkP.Chunk.x, MinChunkP.Chunk.y, MaxChunkP.Chunk.x, MaxChunkP.Chunk.y);
    Result.P = Result.Bounds.Min;
    Result.Chunk = *GetWorldChunkInternal(World, Result.P);
    EnsureValidChunk(World, &Result);
}

function world_chunk_iterator Next(world *World, world_chunk_iterator Iter)
{
    world_chunk_iterator Result = Iter;
    
    Result.Chunk = 0;
    EnsureValidChunk(World, &Result);
}

function u32 OrAllFlagsOnTile(world *World, v2s TileIndex)
{
    u32 Result = 0;

    for(entity_iterator EntIter = IterateEntitiesIn(GetRegionForTileIndex(World, TileIndex));
        IsValid(EntIter);
        EntIter = Next(World, EntIter))
    {
        entity *Entity = EntIter.Entity;
        if(AreEqual(GetTileIndexOf(Entity), TileIndex))
        {
            Result |= Entity->Flags;
            break;
        }
    }
        
    return Result;
}

function b32 TileCanBeOccupied(world *World, v2s TileIndex)
{
    u32 TileFlags = OrAllFlagsOnTile(World,  TileIndex);
    b32 Result = (TileFlags & EntityFlag_SupportsOccupation) && !(TileFlags & EntityFlag_OccupiesTile);
    
    return Result;
}

function b32 TransactionalOccupy(world *World, entity_id EntityID, v2s *MoverRef, v2s DesiredRef)
{
    b32 Result = false;
    
    if(TileCanBeOccupied(World, DesiredRef))
    {
        *MoverRef = DesiredRef;
        Result = true;
    }
    
    return(Result);
}

function closest_entity GetClosestEntityWithBrain(world *World, world_position P,
                                                  brain_type Type, r32 MaxRadius = 20.0f)
{
    closest_entity Result = {};
    Result.DistanceSq = Square(MaxRadius);
    
    for(entity_iterator EntIter = IterateEntitiesIn(GetRegionForRadius(World, P, MaxRadius));
        IsValid(EntIter);
        EntIter = Next(World, EntIter))
    {
        entity *TestEntity = EntIter.Entity;
        if(IsType(TestEntity->BrainSlot, Type))
        {
            world_position TestEntityP = GetWorldPositionOf(EntIter);
            v3 TestDelta = Subtract(World, &TestEntityP, &P);
            real32 TestDSq = LengthSq(TestDelta);
            if(Result.DistanceSq > TestDSq)
            {
                Result.Entity = TestEntity;
                Result.DistanceSq = TestDSq;
                Result.Delta = TestDelta;
            }
        }
    }
    
    return(Result);
}

function tile_result GetClosestEmptyTileTo(world *World, v2s ToTileIndex)
{
    tile_result Result = {};
    
    NotImplemented;
    
    return Result;
}

function tile_result FindRandomOpenTile(world *World, v2 StartUV = V2(0.5f, 0.5f))
{
    tile_result Result = {};

    NotImplemented;
    
#if 0
    f32 BestDistSq = F32Max;
    
    v2 StartP = Hadamard(StartUV, V3(Grid->TileCount).xy);
    
    for(edit_tile Tile = IterateAsPlanarTiles(Grid);
        IsValid(Tile);
        Advance(&Tile))
    {
        if(TraversableIsOpen(Tile))
        {
            f32 DistSq = LengthSq(V3(Tile.RelIndex).xy - StartP);
            if(BestDistSq > DistSq)
            {
                BestDistSq = DistSq;
                Result = Tile;
            }
        }
    }
#endif
    
    return(Result);
}

function tile_result FindAdjacentOpenTile(world *World, v2s BaseTile, u32 DirMask)
{
    // TODO(casey): This should probably use a stochastic selection
    // for the direction, or we pass in an array of ordering, or
    // something, so it's not always biased towards one direction.
    tile_result Result = {};
    
    for(u32 Dir = 0;
        Dir < BoxIndex_Count;
        ++Dir)
    {
        box_surface_index DirIndex = (box_surface_index)Dir;
        u32 Mask = GetSurfaceMask(DirIndex);
        if(DirMask & Mask)
        {
            v2s dTileP = GetDirection(DirIndex);
            v2s CheckTile = BaseTile + dTileP;
            if(TileCanBeOccupied(World, CheckTile))
            {
                Result.Valid = true;
                Result.Index = CheckTile;
                break;
            }
        }
    }
    
    return Result;
}
