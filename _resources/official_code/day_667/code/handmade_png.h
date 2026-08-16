/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#pragma pack(push, 1)
struct png_header
{
    u8 Signature[8];
};
global u8 PNGSignature[] = {137, 80, 78, 71, 13, 10, 26, 10};

struct png_chunk_header
{
    u32 Length;
    union
    {
        u32 TypeU32;
        char Type[4];
    };
};

struct png_chunk_footer
{
    u32 CRC;
};

struct png_ihdr
{
    u32 Width;
    u32 Height;
    u8 BitDepth;
    u8 ColorType;
    u8 CompressionMethod;
    u8 FilterMethod;
    u8 InterlaceMethod;
};

struct png_idat_header
{
    u8 ZLibMethodFlags;
    u8 AdditionalFlags;
};

struct png_idat_footer
{
    u32 CheckValue;
};

struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
};

#pragma pack(pop)

struct png_huffman_entry
{
    u16 Symbol;
    u16 BitsUsed;
};

#define PNG_HUFFMAN_MAX_BIT_COUNT 16
struct png_huffman
{
    u32 MaxCodeLengthInBits;
    u32 EntryCount;
    png_huffman_entry *Entries;
};

internal image_u32 ParsePNG(memory_arena *Memory, stream File, stream *Info = 0);
internal void WritePNG(u32 Width, u32 Height, u32 *Pixels, stream *Out);
