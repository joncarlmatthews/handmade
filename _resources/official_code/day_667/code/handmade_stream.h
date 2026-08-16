/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct memory_arena;

struct stream_chunk
{
    buffer Contents;
    stream_chunk *Next;
    
#if HANDMADE_INTERNAL
    char *GUID;
#endif
};

struct stream
{
    memory_arena *Memory;
    stream *Errors;
    
    buffer Contents;
    
    u32 BitCount;
    u32 BitBuf;
    
    b32 Underflowed;
    
    stream_chunk *First;
    stream_chunk *Last;
};

#if HANDMADE_INTERNAL
#define Outf(...) Outf_(DEBUG_NAME("Outf"), ## __VA_ARGS__)
#else
#define Outf(...) Outf_(__VA_ARGS__)
#endif

internal umm OutfArgList(INTERNAL_MEMORY_PARAM stream *Dest, char *Format, va_list ArgList);
internal umm Outf_(INTERNAL_MEMORY_PARAM stream *Dest, char *Format, ...);
internal stream OnDemandMemoryStream(memory_arena *Memory, stream *Errors = 0);

#define OutStruct(Stream, type) (type *)OutSize(Stream, sizeof(type));
#define OutStructCopy(Stream, Instance) OutCopy(Stream, sizeof(Instance), &(Instance))

