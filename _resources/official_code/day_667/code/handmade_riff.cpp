/* ========================================================================
   $File: C:\work\handmade\code\handmade_riff.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

internal riff_iterator
ParseChunkAt(void *At, void *Stop)
{
    riff_iterator Iter;
    
    Iter.At = (u8 *)At;
    Iter.Stop = (u8 *)Stop;
    
    return(Iter);
}

internal riff_iterator 
IterateRIFF(buffer Buffer, riff_header *Header)
{
    riff_iterator Iter = {};
    if(Buffer.Count >= sizeof(riff_header))
    {
        *Header = *(riff_header *)Buffer.Data;
        u8 *DataStart = Buffer.Data + sizeof(riff_header);
        Iter = ParseChunkAt(DataStart, DataStart + Header->Size - 4);
    }
    else
    {
        ZeroStruct(*Header);
    }
    
    return(Iter);
}

internal riff_iterator
NextChunk(riff_iterator Iter)
{
    riff_chunk_header *Chunk = (riff_chunk_header *)Iter.At;
    u32 Size = (Chunk->Size + 1) & ~1;
    Iter.At += sizeof(riff_chunk_header) + Size;
    
    return(Iter);
}

internal b32
IsValid(riff_iterator Iter)
{    
    b32 Result = (Iter.At < Iter.Stop);
    
    return(Result);
}

internal void *
GetChunkData(riff_iterator Iter)
{
    void *Result = (Iter.At + sizeof(riff_chunk_header));
    
    return(Result);
}

internal u32
GetType(riff_iterator Iter)
{
    riff_chunk_header *Chunk = (riff_chunk_header *)Iter.At;
    u32 Result = Chunk->ID;
    
    return(Result);
}

internal u32
GetChunkDataSize(riff_iterator Iter)
{
    riff_chunk_header *Chunk = (riff_chunk_header *)Iter.At;
    u32 Result = Chunk->Size;
    
    return(Result);
}
