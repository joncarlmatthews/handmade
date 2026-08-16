/* ========================================================================
   $File: C:\work\handmade\code\handmade_riff.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

struct riff_iterator
{
    u8 *At;
    u8 *Stop;
};

#pragma pack(push, 1)

#define RIFF_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

struct riff_chunk_header
{
    u32 ID;
    u32 Size;
};

struct riff_header
{
    u32 RIFFID;
    u32 Size;
    u32 FileTypeID;
};

enum
{
    RIFFID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
};

#pragma pack(pop)

internal riff_iterator ParseChunkAt(void *At, void *Stop);
internal riff_iterator IterateRIFF(buffer Buffer, riff_header *Header);
internal riff_iterator NextChunk(riff_iterator Iter);
internal b32 IsValid(riff_iterator Iter);
internal void *GetChunkData(riff_iterator Iter);
internal u32 GetType(riff_iterator Iter);
internal u32 GetChunkDataSize(riff_iterator Iter);
