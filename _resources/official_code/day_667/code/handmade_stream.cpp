/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal stream_chunk *
AppendChunk(stream *Stream, umm Size, void *Contents)
{
    stream_chunk *Chunk = PushStruct(Stream->Memory, stream_chunk);
    
    Chunk->Contents.Count = Size;
    Chunk->Contents.Data = (u8 *)Contents;
    Chunk->Next = 0;
    
    Stream->Last = ((Stream->Last ? Stream->Last->Next : Stream->First) = Chunk);
    
    return(Chunk);
}

internal void
RefillIfNecessary(stream *File)
{
    // TODO(casey): Use a free list to recycle chunks, if we ever care
    
    if((File->Contents.Count == 0) && File->First)
    {
        stream_chunk *This = File->First;
        File->Contents = This->Contents;
        File->First = This->Next;
    }
}

#define Consume(File, type) (type *)ConsumeSize(File, sizeof(type))
internal void *
ConsumeSize(stream *File, u32 Size)
{
    RefillIfNecessary(File);
    
    void *Result = Advance(&File->Contents, Size);
    if(!Result)
    {
        File->Underflowed = true;
    }
    
    Assert(!File->Underflowed);
    
    return(Result);
}

internal u32
PeekBits(stream *Buf, u32 BitCount)
{
    Assert(BitCount <= 32);
    
    u32 Result = 0;
    
    while((Buf->BitCount < BitCount) &&
          !Buf->Underflowed)
    {
        u32 Byte = *Consume(Buf, u8);
        Buf->BitBuf |= (Byte << Buf->BitCount);
        Buf->BitCount += 8;
    }
    
    Result = Buf->BitBuf & ((1 << BitCount) - 1);
    
    return(Result);
}

internal void
DiscardBits(stream *Buf, u32 BitCount)
{
    Buf->BitCount -= BitCount;
    Buf->BitBuf >>= BitCount;
}

internal u32
ConsumeBits(stream *Buf, u32 BitCount)
{
    u32 Result = PeekBits(Buf, BitCount);
    DiscardBits(Buf, BitCount);
    
    return(Result);
}

internal void
FlushByte(stream *Buf)
{
    u32 FlushCount = (Buf->BitCount % 8);
    ConsumeBits(Buf, FlushCount);
}

internal umm
OutfArgList(INTERNAL_MEMORY_PARAM stream *Dest, char *Format, va_list ArgList)
{
    umm Size = 0;
    
    if(Dest)
    {
        char Buffer[1024];
        Size = FormatStringList(sizeof(Buffer), Buffer, Format, ArgList);
        
        void *Contents = PushCopy_(INTERNAL_MEMORY_PASS Dest->Memory, Size, Buffer);
        stream_chunk *Chunk = AppendChunk(Dest, Size, Contents);
#if HANDMADE_INTERNAL
        Chunk->GUID = GUID;
#endif
    }
    
    return(Size);
}

internal umm
Outf_(INTERNAL_MEMORY_PARAM stream *Dest, char *Format, ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    umm Size = OutfArgList(INTERNAL_MEMORY_PASS Dest, Format, ArgList);
    va_end(ArgList);
    
    return(Size);
}

internal void *
OutCopy(stream *Dest, umm Count, void *Data)
{
    void *Result = PushCopy(Dest->Memory, Count, Data);
    AppendChunk(Dest, Count, Result);
    return(Result);
}

internal void *
OutSize(stream *Dest, umm Count)
{
    void *Result = PushSize(Dest->Memory, Count);
    AppendChunk(Dest, Count, Result);
    return(Result);
}

internal stream
MakeReadStream(buffer Contents, stream *Errors)
{
    stream Result = {};
    
    Result.Errors = Errors;
    Result.Contents = Contents;
    
    return(Result);
}

internal stream
OnDemandMemoryStream(memory_arena *Memory, stream *Errors)
{
    stream Result = {};
    
    Result.Memory = Memory;
    Result.Errors = Errors;
    
    return(Result);
}

internal umm
GetTotalSize(stream *Stream)
{
    umm Result = 0;
    
    for(stream_chunk *Chunk = Stream->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        Result += Chunk->Contents.Count;
    }
    
    return(Result);
}

internal void
CopyStreamToBuffer(stream *Source, buffer Dest)
{
    Assert(GetTotalSize(Source) <= Dest.Count);
    
    u64 DataOffset = 0;
    for(stream_chunk *Chunk = Source->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        Copy((u32)Chunk->Contents.Count, Chunk->Contents.Data, Dest.Data + DataOffset);
        DataOffset += Chunk->Contents.Count;
    }
}

