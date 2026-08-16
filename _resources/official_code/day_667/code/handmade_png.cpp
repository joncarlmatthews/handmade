/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

global u32 GlobalCRCTable[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

internal void
EndianSwap(u16 *Value)
{
    u16 V = (*Value);
#if 1
    *Value = ((V << 8) | (V >> 8));
#else
    *Value = _byteswap_ushort(V);
#endif
}

internal void
EndianSwap(u32 *Value)
{
    u32 V = (*Value);
#if 1
    *Value = ((V << 24) |
              ((V & 0xFF00) << 8) |
              ((V >> 8) & 0xFF00) |
              (V >> 24));
#else
    *Value = _byteswap_ulong(V);
#endif
}

internal u32
EndBigCRC(umm SkipN, stream_chunk *StartChunk, stream_chunk *OnePastLastChunk = 0)
{
    // NOTE(casey): This code is just a modified version of the reference
    // code for the CRC given in Annex D of the PNG specification.
    u32 Result = 0xffffffffL;
    
    for(stream_chunk *Chunk = StartChunk;
        Chunk != OnePastLastChunk;
        Chunk = Chunk->Next)
    {
        Assert(SkipN <= Chunk->Contents.Count);
        for(umm n = SkipN; n < Chunk->Contents.Count; ++n) 
        {
            Result = GlobalCRCTable[(Result ^ Chunk->Contents.Data[n]) & 0xff] ^ (Result >> 8);
        }
        SkipN = 0;
    }
    
    Result = (Result ^ 0xffffffffL);
    EndianSwap(&Result);
    return(Result);
}

internal u32
GetTotalImageSize(image_u32 Image)
{
    u32 Result = Image.Width*Image.Height*4;
    return(Result);
}
internal image_u32
PushImage(memory_arena *Arena, u32 Width, u32 Height)
{
    image_u32 Result;
    
    Result.Width = Width;
    Result.Height = Height;
    Result.Pixels = (u32 *)PushSize(Arena, GetTotalImageSize(Result));
    return(Result);
}

internal u32
ReverseBits(u32 V, u32 BitCount)
{
    u32 Result = 0;
    
    for(u32 BitIndex = 0;
        BitIndex <= (BitCount / 2);
        ++BitIndex)
    {
        u32 Inv = (BitCount - (BitIndex + 1));
        Result |= ((V >> BitIndex) & 0x1) << Inv;
        Result |= ((V >> Inv) & 0x1) << BitIndex;
    }
    
    return(Result);
}

internal void *
AllocatePixels(memory_arena *Memory, u32 Width, u32 Height, u32 BPP, u32x ExtraBytes = 0)
{
    void *Result = PushSize(Memory, Width*Height*BPP + (ExtraBytes*Height));
    
    return(Result);
}

internal png_huffman
AllocateHuffman(memory_arena *Memory, u32 MaxCodeLengthInBits)
{
    Assert(MaxCodeLengthInBits <= PNG_HUFFMAN_MAX_BIT_COUNT);
    
    png_huffman Result = {};
    
    Result.MaxCodeLengthInBits = MaxCodeLengthInBits;
    Result.EntryCount = (1 << MaxCodeLengthInBits);
    Result.Entries = PushArray(Memory, Result.EntryCount, png_huffman_entry);
    
    return(Result);
}

internal void
ComputeHuffman(u32 SymbolCount, u32 *SymbolCodeLength, png_huffman *Result, u32 SymbolAddend = 0)
{
    u32 CodeLengthHist[PNG_HUFFMAN_MAX_BIT_COUNT] = {};
    for(u32 SymbolIndex = 0;
        SymbolIndex < SymbolCount;
        ++SymbolIndex)
    {
        u32 Count = SymbolCodeLength[SymbolIndex];
        Assert(Count <= ArrayCount(CodeLengthHist));
        ++CodeLengthHist[Count];
    }
    
    u32 NextUnusedCode[PNG_HUFFMAN_MAX_BIT_COUNT];
    NextUnusedCode[0] = 0;
    CodeLengthHist[0] = 0;
    for(u32 BitIndex = 1;
        BitIndex < ArrayCount(NextUnusedCode);
        ++BitIndex)
    {
        NextUnusedCode[BitIndex] = ((NextUnusedCode[BitIndex - 1] +
                                     CodeLengthHist[BitIndex - 1]) << 1);
    }
    
    for(u32 SymbolIndex = 0;
        SymbolIndex < SymbolCount;
        ++SymbolIndex)
    {
        u32 CodeLengthInBits = SymbolCodeLength[SymbolIndex];
        if(CodeLengthInBits)
        {
            Assert(CodeLengthInBits < ArrayCount(NextUnusedCode));
            u32 Code = NextUnusedCode[CodeLengthInBits]++;
            
            u32 ArbitraryBits = Result->MaxCodeLengthInBits - CodeLengthInBits;
            u32 EntryCount = (1 << ArbitraryBits);
            
            for(u32 EntryIndex = 0;
                EntryIndex < EntryCount;
                ++EntryIndex)
            {
                u32 BaseIndex = (Code << ArbitraryBits) | EntryIndex;
                u32 Index = ReverseBits(BaseIndex, Result->MaxCodeLengthInBits);
                
                png_huffman_entry *Entry = Result->Entries + Index;
                
                u32 Symbol = (SymbolIndex + SymbolAddend);
                Entry->BitsUsed = (u16)CodeLengthInBits;
                Entry->Symbol = (u16)Symbol;
                
                Assert(Entry->BitsUsed == CodeLengthInBits);
                Assert(Entry->Symbol == Symbol);
            }
        }
    }
}

internal u32
HuffmanDecode(png_huffman *Huffman, stream *Input)
{
    u32 EntryIndex = PeekBits(Input, Huffman->MaxCodeLengthInBits);
    Assert(EntryIndex < Huffman->EntryCount);
    
    png_huffman_entry Entry = Huffman->Entries[EntryIndex];
    
    u32 Result = Entry.Symbol;
    DiscardBits(Input, Entry.BitsUsed);
    Assert(Entry.BitsUsed);
    
    return(Result);
}

global png_huffman_entry PNGLengthExtra[] =
{
    {3, 0}, // NOTE(casey): 257
    {4, 0}, // NOTE(casey): 258
    {5, 0}, // NOTE(casey): 259
    {6, 0}, // NOTE(casey): 260
    {7, 0}, // NOTE(casey): 261
    {8, 0}, // NOTE(casey): 262
    {9, 0}, // NOTE(casey): 263
    {10, 0}, // NOTE(casey): 264
    {11, 1}, // NOTE(casey): 265
    {13, 1}, // NOTE(casey): 266
    {15, 1}, // NOTE(casey): 267
    {17, 1}, // NOTE(casey): 268
    {19, 2}, // NOTE(casey): 269
    {23, 2}, // NOTE(casey): 270
    {27, 2}, // NOTE(casey): 271
    {31, 2}, // NOTE(casey): 272
    {35, 3}, // NOTE(casey): 273
    {43, 3}, // NOTE(casey): 274
    {51, 3}, // NOTE(casey): 275
    {59, 3}, // NOTE(casey): 276
    {67, 4}, // NOTE(casey): 277
    {83, 4}, // NOTE(casey): 278
    {99, 4}, // NOTE(casey): 279
    {115, 4}, // NOTE(casey): 280
    {131, 5}, // NOTE(casey): 281
    {163, 5}, // NOTE(casey): 282
    {195, 5}, // NOTE(casey): 283
    {227, 5}, // NOTE(casey): 284
    {258, 0}, // NOTE(casey): 285
};

global png_huffman_entry PNGDistExtra[] =
{
    {1, 0}, // NOTE(casey): 0
    {2, 0}, // NOTE(casey): 1
    {3, 0}, // NOTE(casey): 2
    {4, 0}, // NOTE(casey): 3
    {5, 1}, // NOTE(casey): 4
    {7, 1}, // NOTE(casey): 5
    {9, 2}, // NOTE(casey): 6
    {13, 2}, // NOTE(casey): 7
    {17, 3}, // NOTE(casey): 8
    {25, 3}, // NOTE(casey): 9
    {33, 4}, // NOTE(casey): 10
    {49, 4}, // NOTE(casey): 11
    {65, 5}, // NOTE(casey): 12
    {97, 5}, // NOTE(casey): 13
    {129, 6}, // NOTE(casey): 14
    {193, 6}, // NOTE(casey): 15
    {257, 7}, // NOTE(casey): 16
    {385, 7}, // NOTE(casey): 17
    {513, 8}, // NOTE(casey): 18
    {769, 8}, // NOTE(casey): 19
    {1025, 9}, // NOTE(casey): 20
    {1537, 9}, // NOTE(casey): 21
    {2049, 10}, // NOTE(casey): 22
    {3073, 10}, // NOTE(casey): 23
    {4097, 11}, // NOTE(casey): 24
    {6145, 11}, // NOTE(casey): 25
    {8193, 12}, // NOTE(casey): 26
    {12289, 12}, // NOTE(casey): 27
    {16385, 13}, // NOTE(casey): 28
    {24577, 13}, // NOTE(casey): 29
};

internal u8
PNGFilter1And2(u8 *x, u8 *a, u32 Channel)
{
    u8 Result = (u8)x[Channel] + (u8)a[Channel];
    return(Result);
}

internal u8
PNGFilter3(u8 *x, u8 *a, u8 *b, u32 Channel)
{
    u32 Average = ((u32)a[Channel] + (u32)b[Channel]) / 2;
    u8 Result = (u8)x[Channel] + (u8)Average;
    return(Result);
}

internal u8
PNGFilter4(u8 *x, u8 *aFull, u8 *bFull, u8 *cFull, u32 Channel)
{
    s32 a = (s32)aFull[Channel];
    s32 b = (s32)bFull[Channel];
    s32 c = (s32)cFull[Channel];
    s32 p = a + b - c;
    
    s32 pa = p - a;
    if(pa < 0) {pa = -pa;}
    
    s32 pb = p - b;
    if(pb < 0) {pb = -pb;}
    
    s32 pc = p - c;
    if(pc < 0) {pc = -pc;}
    
    s32 Paeth;
    if((pa <= pb) && (pa <= pc))
    {
        Paeth = a;
    }
    else if(pb <= pc)
    {
        Paeth = b;
    }
    else
    {
        Paeth = c;
    }
    
    u8 Result = (u8)x[Channel] + (u8)Paeth;
    return(Result);
}

internal void
PNGFilterReconstruct(u32x Width, u32x Height, u8 *DecompressedPixels, u8 *FinalPixels,
                     stream *Errors)
{
    // NOTE(casey): If you cared about speed, this filter process
    // seems tailor-made for SIMD - you could go 4-wide trivially.
    u32 Zero = 0;
    u8 *PriorRow = (u8 *)&Zero;
    s32 PriorRowAdvance = 0;
    u8 *Source = DecompressedPixels;
    u8 *Dest = FinalPixels;
    
    for(u32 Y = 0;
        Y < Height;
        ++Y)
    {
        u8 Filter = *Source++;
        u8 *CurrentRow = Dest;
        
        switch(Filter)
        {
            case 0:
            {
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    *(u32 *)Dest = *(u32 *)Source;
                    Dest += 4;
                    Source += 4;
                }
            } break;
            
            case 1:
            {
                u32 APixel = 0;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter1And2(Source, (u8 *)&APixel, 0);
                    Dest[1] = PNGFilter1And2(Source, (u8 *)&APixel, 1);
                    Dest[2] = PNGFilter1And2(Source, (u8 *)&APixel, 2);
                    Dest[3] = PNGFilter1And2(Source, (u8 *)&APixel, 3);
                    
                    APixel = *(u32 *)Dest;
                    
                    Dest += 4;
                    Source += 4;
                }
            } break;
            
            case 2:
            {
                u8 *BPixel = PriorRow;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter1And2(Source, BPixel, 0);
                    Dest[1] = PNGFilter1And2(Source, BPixel, 1);
                    Dest[2] = PNGFilter1And2(Source, BPixel, 2);
                    Dest[3] = PNGFilter1And2(Source, BPixel, 3);
                    
                    BPixel += PriorRowAdvance;
                    Dest += 4;
                    Source += 4;
                }
            } break;
            
            case 3:
            {
                // TODO(casey): This has not been tested!
                
                u32 APixel = 0;
                u8 *BPixel = PriorRow;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 0);
                    Dest[1] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 1);
                    Dest[2] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 2);
                    Dest[3] = PNGFilter3(Source, (u8 *)&APixel, BPixel, 3);
                    
                    APixel = *(u32 *)Dest;
                    
                    BPixel += PriorRowAdvance;
                    Dest += 4;
                    Source += 4;
                }
            } break;
            
            case 4:
            {
                u32 APixel = 0;
                u32 CPixel = 0;
                u8 *BPixel = PriorRow;
                for(u32 X = 0;
                    X < Width;
                    ++X)
                {
                    Dest[0] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 0);
                    Dest[1] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 1);
                    Dest[2] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 2);
                    Dest[3] = PNGFilter4(Source, (u8 *)&APixel, BPixel, (u8 *)&CPixel, 3);
                    
                    CPixel = *(u32 *)BPixel;
                    APixel = *(u32 *)Dest;
                    
                    BPixel += PriorRowAdvance;
                    Dest += 4;
                    Source += 4;
                }
            } break;
            
            default:
            {
                Outf(Errors, "ERROR: Unrecognized row filter %u.\n", Filter);
            } break;
            
        }
        
        PriorRow = CurrentRow;
        PriorRowAdvance = 4;
    }
}

internal image_u32
ParsePNG(memory_arena *Memory, stream File, stream *Info)
{
    // NOTE(casey): This is NOT MEANT TO BE FAULT TOLERANT.  It only loads specifically
    // what we expect, and is happy to crash otherwise.
    
    stream *At = &File;
    
    b32x Supported = false;
    
    u32 Width = 0;
    u32 Height = 0;
    u8 *FinalPixels = 0;
    
    png_header *FileHeader = Consume(At, png_header);
    if(FileHeader)
    {
        stream CompData = OnDemandMemoryStream(Memory, File.Errors);
        
        while(At->Contents.Count > 0)
        {
            png_chunk_header *ChunkHeader = Consume(At, png_chunk_header);
            if(ChunkHeader)
            {
                EndianSwap(&ChunkHeader->Length);
                EndianSwap(&ChunkHeader->TypeU32);
                
                void *ChunkData = ConsumeSize(At, ChunkHeader->Length);
                png_chunk_footer *ChunkFooter = Consume(At, png_chunk_footer);
                EndianSwap(&ChunkFooter->CRC);
                
                if(ChunkHeader->TypeU32 == 'IHDR')
                {
                    Outf(Info, "IHDR\n");
                    
                    png_ihdr *IHDR = (png_ihdr *)ChunkData;
                    
                    EndianSwap(&IHDR->Width);
                    EndianSwap(&IHDR->Height);
                    
                    Outf(Info, "    Width: %u\n", IHDR->Width);
                    Outf(Info, "    Height: %u\n", IHDR->Height);
                    Outf(Info, "    BitDepth: %u\n", IHDR->BitDepth);
                    Outf(Info, "    ColorType: %u\n", IHDR->ColorType);
                    Outf(Info, "    CompressionMethod: %u\n", IHDR->CompressionMethod);
                    Outf(Info, "    FilterMethod: %u\n", IHDR->FilterMethod);
                    Outf(Info, "    InterlaceMethod: %u\n", IHDR->InterlaceMethod);
                    
                    if((IHDR->BitDepth == 8) &&
                       (IHDR->ColorType == 6) &&
                       (IHDR->CompressionMethod == 0) &&
                       (IHDR->FilterMethod == 0) &&
                       (IHDR->InterlaceMethod == 0))
                    {
                        Width = IHDR->Width;
                        Height = IHDR->Height;
                        Supported = true;
                    }
                }
                else if(ChunkHeader->TypeU32 == 'IDAT')
                {
                    Outf(Info, "IDAT (%u)\n", ChunkHeader->Length);
                    AppendChunk(&CompData, ChunkHeader->Length, ChunkData);
                }
            }
        }
        
        if(Supported)
        {
            Outf(Info, "Examining ZLIB headers...\n");
            png_idat_header *IDATHead = Consume(&CompData, png_idat_header);
            
            u8 CM = (IDATHead->ZLibMethodFlags & 0xF);
            u8 CINFO = (IDATHead->ZLibMethodFlags >> 4);
            u8 FCHECK = (IDATHead->AdditionalFlags & 0x1F);
            u8 FDICT = (IDATHead->AdditionalFlags >> 5) & 0x1;
            u8 FLEVEL = (IDATHead->AdditionalFlags >> 6);
            
            Outf(Info, "    CM: %u\n", CM);
            Outf(Info, "    CINFO: %u\n", CINFO);
            Outf(Info, "    FCHECK: %u\n", FCHECK);
            Outf(Info, "    FDICT: %u\n", FDICT);
            Outf(Info, "    FLEVEL: %u\n", FLEVEL);
            
            Supported = ((CM == 8) && (FDICT == 0));
            
            if(Supported)
            {
                FinalPixels = (u8 *)AllocatePixels(Memory, Width, Height, 4);
                u8 *DecompressedPixels = (u8 *)AllocatePixels(Memory, Width, Height, 4, 1);
                u8 *DecompressedPixelsEnd = DecompressedPixels + (Height*((Width*4) + 1));
                u8 *Dest = DecompressedPixels;
                Outf(Info, "Decompressing...\n");
                
                u32 BFINAL = 0;
                while(BFINAL == 0)
                {
                    Assert(Dest <= DecompressedPixelsEnd);
                    BFINAL = ConsumeBits(&CompData, 1);
                    u32 BTYPE = ConsumeBits(&CompData, 2);
                    
                    if(BTYPE == 0)
                    {
                        FlushByte(&CompData);
                        
                        u16 LEN = (u16)ConsumeBits(&CompData, 16);
                        u16 NLEN = (u16)ConsumeBits(&CompData, 16);
                        if((u16)LEN != (u16)~NLEN)
                        {
                            Outf(CompData.Errors, "ERROR: LEN/NLEN mismatch.\n");
                        }
                        
                        while(LEN)
                        {
                            RefillIfNecessary(&CompData);
                            
                            u16 UseLEN = LEN;
                            if(UseLEN > CompData.Contents.Count)
                            {
                                UseLEN = (u16)CompData.Contents.Count;
                            }
                            
                            u8 *Source = (u8 *)ConsumeSize(&CompData, UseLEN);
                            if(Source)
                            {
                                u16 CopyCount = UseLEN;
                                while(CopyCount--)
                                {
                                    *Dest++ = *Source++;
                                }
                            }
                            
                            LEN -= UseLEN;
                        }
                    }
                    else if(BTYPE == 3)
                    {
                        Outf(CompData.Errors, "ERROR: BTYPE of %u encountered.\n", BTYPE);
                    }
                    else
                    {
                        u32 LitLenDistTable[512];
                        png_huffman LitLenHuffman = AllocateHuffman(Memory, 15);
                        png_huffman DistHuffman = AllocateHuffman(Memory, 15);
                        
                        u32 HLIT = 0;
                        u32 HDIST = 0;
                        if(BTYPE == 2)
                        {
                            HLIT = ConsumeBits(&CompData, 5);
                            HDIST = ConsumeBits(&CompData, 5);
                            u32 HCLEN = ConsumeBits(&CompData, 4);
                            
                            HLIT += 257;
                            HDIST += 1;
                            HCLEN += 4;
                            
                            u32 HCLENSwizzle[] =
                            {
                                16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15,
                            };
                            Assert(HCLEN <= ArrayCount(HCLENSwizzle));
                            u32 HCLENTable[ArrayCount(HCLENSwizzle)] = {};
                            
                            for(u32 Index = 0;
                                Index < HCLEN;
                                ++Index)
                            {
                                HCLENTable[HCLENSwizzle[Index]] = ConsumeBits(&CompData, 3);
                            }
                            
                            png_huffman DictHuffman = AllocateHuffman(Memory, 7);
                            ComputeHuffman(ArrayCount(HCLENSwizzle), HCLENTable, &DictHuffman);
                            
                            u32 LitLenCount = 0;
                            u32 LenCount = HLIT + HDIST;
                            Assert(LenCount <= ArrayCount(LitLenDistTable));
                            while(LitLenCount < LenCount)
                            {
                                u32 RepCount = 1;
                                u32 RepVal = 0;
                                u32 EncodedLen = HuffmanDecode(&DictHuffman, &CompData);
                                if(EncodedLen <= 15)
                                {
                                    RepVal = EncodedLen;
                                }
                                else if(EncodedLen == 16)
                                {
                                    RepCount = 3 + ConsumeBits(&CompData, 2);
                                    
                                    Assert(LitLenCount > 0);
                                    RepVal = LitLenDistTable[LitLenCount - 1];
                                }
                                else if(EncodedLen == 17)
                                {
                                    RepCount = 3 + ConsumeBits(&CompData, 3);
                                }
                                else if(EncodedLen == 18)
                                {
                                    RepCount = 11 + ConsumeBits(&CompData, 7);
                                }
                                else
                                {
                                    Outf(CompData.Errors, "ERROR: EncodedLen of %u encountered.\n", EncodedLen);
                                }
                                
                                while(RepCount--)
                                {
                                    LitLenDistTable[LitLenCount++] = RepVal;
                                }
                            }
                            Assert(LitLenCount == LenCount);
                            
                        }
                        else if(BTYPE == 1)
                        {
                            HLIT = 288;
                            HDIST = 32;
                            u32 BitCounts[][2] =
                            {
                                {143, 8},
                                {255, 9},
                                {279, 7},
                                {287, 8},
                                {319, 5},
                            };
                            
                            u32 BitCountIndex = 0;
                            for(u32 RangeIndex = 0;
                                RangeIndex < ArrayCount(BitCounts);
                                ++RangeIndex)
                            {
                                u32 BitCount = BitCounts[RangeIndex][1];
                                u32 LastValue = BitCounts[RangeIndex][0];
                                while(BitCountIndex <= LastValue)
                                {
                                    LitLenDistTable[BitCountIndex++] = BitCount;
                                }
                            }
                        }
                        else
                        {
                            Outf(CompData.Errors, "ERROR: BTYPE of %u encountered.\n", BTYPE);
                        }
                        
                        ComputeHuffman(HLIT, LitLenDistTable, &LitLenHuffman);
                        ComputeHuffman(HDIST, LitLenDistTable + HLIT, &DistHuffman);
                        
                        for(;;)
                        {
                            u32 LitLen = HuffmanDecode(&LitLenHuffman, &CompData);
                            if(LitLen <= 255)
                            {
                                u32 Out = (LitLen & 0xFF);
                                *Dest++ = (u8)Out;
                            }
                            else if(LitLen >= 257)
                            {
                                u32 LenTabIndex = (LitLen - 257);
                                png_huffman_entry LenTab = PNGLengthExtra[LenTabIndex];
                                u32 Len = LenTab.Symbol;
                                if(LenTab.BitsUsed)
                                {
                                    u32 ExtraBits = ConsumeBits(&CompData, LenTab.BitsUsed);
                                    Len += ExtraBits;
                                }
                                
                                u32 DistTabIndex = HuffmanDecode(&DistHuffman, &CompData);
                                png_huffman_entry DistTab = PNGDistExtra[DistTabIndex];
                                u32 Distance = DistTab.Symbol;
                                if(DistTab.BitsUsed)
                                {
                                    u32 ExtraBits = ConsumeBits(&CompData, DistTab.BitsUsed);
                                    Distance += ExtraBits;
                                }
                                
                                u8 *Source = Dest - Distance;
                                Assert((Source + Len) <= DecompressedPixelsEnd);
                                Assert((Dest + Len) <= DecompressedPixelsEnd);
                                Assert(Source >= DecompressedPixels);
                                
                                while(Len--)
                                {
                                    *Dest++ = *Source++;
                                }
                            }
                            else
                            {
                                break;
                            }
                        }
                    }
                }
                
                Assert(Dest == DecompressedPixelsEnd);
                PNGFilterReconstruct(Width, Height, DecompressedPixels, FinalPixels,
                                     CompData.Errors);
            }
        }
    }
    
    Outf(Info, "Supported: %s\n", Supported ? "TRUE" : "FALSE");
    
    image_u32 Result = {};
    Result.Width = Width;
    Result.Height = Height;
    Result.Pixels = (u32 *)FinalPixels;
    return(Result);
}

internal void
WritePNG(u32 Width, u32 Height, u32 *Pixels, stream *Out)
{
    stream_chunk *StartCRC = 0;
    u32 OutputPixelSize = 4*Width*Height;
    
    OutCopy(Out, sizeof(PNGSignature), PNGSignature);
    
    png_ihdr IHDR;
    IHDR.Width = Width;
    IHDR.Height = Height;
    IHDR.BitDepth = 8;
    IHDR.ColorType = 6;
    IHDR.CompressionMethod = 0;
    IHDR.FilterMethod = 0;
    IHDR.InterlaceMethod = 0;
    EndianSwap(&IHDR.Width);
    EndianSwap(&IHDR.Height);
    
    png_chunk_header ChunkHeader;
    png_chunk_footer ChunkFooter;
    
    ChunkHeader.Length = sizeof(IHDR);
    ChunkHeader.TypeU32 = 'IHDR';
    EndianSwap(&ChunkHeader.Length);
    EndianSwap(&ChunkHeader.TypeU32);
    
    OutStructCopy(Out, ChunkHeader);
    StartCRC = Out->Last;
    OutStructCopy(Out, IHDR);
    ChunkFooter.CRC = EndBigCRC(4, StartCRC);
    OutStructCopy(Out, ChunkFooter);
    
    png_idat_header IDAT;
    IDAT.ZLibMethodFlags = 8;
    IDAT.AdditionalFlags = 29;
    // NOTE(casey): The FCHECK value has to make the IDAT be a multiple of 31, 
    // so 29 corrects for the fact that we would be remainder 2 when we set
    // ZLibMethodFlags to 8.
    
    u32 MaxChunkSize = 65535;
    
    u32 TotalLength = ((Width*4 + 1)*Height);
    u32 ChunkCount = (TotalLength + MaxChunkSize - 1) / MaxChunkSize;
    if(ChunkCount == 0)
    {
        ChunkCount = 1;
    }
    
    u32 ChunkOverhead = (sizeof(u8) + sizeof(u16) + sizeof(u16));
    ChunkHeader.Length = (sizeof(IDAT) + (ChunkCount*ChunkOverhead) + TotalLength + sizeof(u32));
    ChunkHeader.TypeU32 = 'IDAT';
    EndianSwap(&ChunkHeader.Length);
    EndianSwap(&ChunkHeader.TypeU32);
    
    OutStructCopy(Out, ChunkHeader);
    StartCRC = Out->Last;
    OutStructCopy(Out, IDAT);
    
    adler_32 Adler = BeginAdler32();
    
    u32 B = 0;
    u32 Y = 0;
    u32 LengthRemaining = TotalLength;
    for(u32 ChunkIndex = 0;
        ChunkIndex < ChunkCount;
        ++ChunkIndex)
    {
        u16 Len = (u16)MaxChunkSize;
        if(Len > LengthRemaining)
        {
            Len = (u16)LengthRemaining;
        }
        u16 NLen = (u16)~Len;
        LengthRemaining -= Len;
        
        u32 TotalRowLen = (4*Width + 1);
        u8 BFinalType = ((ChunkIndex + 1) == ChunkCount) ? 0x1 : 0x0;
        OutStructCopy(Out, BFinalType);
        OutStructCopy(Out, Len);
        OutStructCopy(Out, NLen);
        while(Len)
        {
            u8 NoFilter = 0;
            u32 RowLen = 1;
            void *Row = &NoFilter;
            if(B > 0)
            {
                RowLen = TotalRowLen - B;
                if(RowLen > Len)
                {
                    RowLen = Len;
                }
                Row = (u8 *)(Pixels + Y*Width) + B - 1;
            }
            
            OutCopy(Out, RowLen, Row); // TODO(casey): For speed, this could just append a direct pointer!
            Adler32Append(&Adler, RowLen, Row);
            B += RowLen;
            Len -= (u16)RowLen;
            if(B == TotalRowLen)
            {
                B = 0;
                ++Y;
            }
        }
    }
    
    u32 Adler32 = EndAdler32(&Adler);
    EndianSwap(&Adler32);
    OutStructCopy(Out, Adler32);
    ChunkFooter.CRC = EndBigCRC(4, StartCRC);
    OutStructCopy(Out, ChunkFooter);
    
    ChunkHeader.Length = 0;
    ChunkHeader.TypeU32 = 'IEND';
    EndianSwap(&ChunkHeader.Length);
    EndianSwap(&ChunkHeader.TypeU32);
    OutStructCopy(Out, ChunkHeader);
    StartCRC = Out->Last;
    ChunkFooter.CRC = EndBigCRC(4, StartCRC);
    OutStructCopy(Out, ChunkFooter);
}
