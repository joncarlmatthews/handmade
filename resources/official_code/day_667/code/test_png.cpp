/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#undef HANDMADE_INTERNAL
#define HANDMADE_INTERNAL 0

#include "handmade_platform.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_shared.h"
#include "handmade_memory.h"
#include "handmade_stream.h"
#include "handmade_image.h"
#include "handmade_png.h"
#include "handmade_stream.cpp"
#include "handmade_image.cpp"
#include "handmade_png.cpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

enum pixel_op
{
    PixelOp_SwapR = 0x1,
    PixelOp_ReplA = 0x2,
    PixelOp_MulA = 0x4,
    PixelOp_Invert = 0x8,
    PixelOp_ThresholdA = 0x10,
};

internal u32
ThreshAlpha(u32 C)
{
    u32 Alpha = (C >> 24);
    if(Alpha)
    {
        Alpha = 0xff;
    }
    
    u32 Result = ((Alpha << 24) | C);
    
    return(Result);
}

internal void
WriteImageTopDownRGBA(u32 Width, u32 Height, u8 *Pixels, char *OutputFileName,
                      u32 PixelOps, stream *Errors)
{
    u32 OutputPixelSize = 4*Width*Height;
    
    b32x ReplA = (PixelOps & PixelOp_ReplA);
    b32x SwapR = (PixelOps & PixelOp_SwapR);
    b32x MulA = (PixelOps & PixelOp_MulA);
    b32x Invert = (PixelOps & PixelOp_Invert);
    b32x ThreshA = (PixelOps & PixelOp_ThresholdA);
    
    bitmap_header Header = {};
    Header.FileType = 0x4D42;
    Header.FileSize = sizeof(Header) + OutputPixelSize;
    Header.BitmapOffset = sizeof(Header);
    Header.Size = sizeof(Header) - 14;
    Header.Width = Width;
    Header.Height = Height;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.Compression = 0;
    Header.SizeOfBitmap = OutputPixelSize;
    Header.HorzResolution = 0;
    Header.VertResolution = 0;
    Header.ColorsUsed = 0;
    Header.ColorsImportant = 0;
    
    u32 MidPointY = ((Header.Height + 1) / 2);
    u32 *Row0 = (u32 *)Pixels;
    u32 *Row1 = Row0 + ((Height - 1)*Width);
    for(u32 Y = 0;
        Y < MidPointY;
        ++Y)
    {
        u32 *Pix0 = Row0;
        u32 *Pix1 = Row1;
        for(u32 X = 0;
            X < Width;
            ++X)
        {
            u32 C0 = *Pix0;
            u32 C1 = *Pix1;
            
            if(SwapR)
            {
                C0 = SwapRAndB(C0);
                C1 = SwapRAndB(C1);
            }
            
            if(ThreshA)
            {
                C0 = ThreshAlpha(C0);
                C1 = ThreshAlpha(C1);
            }
            
            if(MulA)
            {
                C0 = MulAlpha(C0);
                C1 = MulAlpha(C1);
            }
            
            if(ReplA)
            {
                C0 = ReplAlpha(C0);
                C1 = ReplAlpha(C1);
            }
            
            if(Invert)
            {
                *Pix0++ = C1;
                *Pix1++ = C0;
            }
            else
            {
                *Pix0++ = C0;
                *Pix1++ = C1;
            }
        }
        
        Row0 += Width;
        Row1 -= Width;
    }
    
    FILE *OutFile = fopen(OutputFileName, "wb");
    if(OutFile)
    {
        fwrite(&Header, sizeof(Header), 1, OutFile);
        fwrite(Pixels, OutputPixelSize, 1, OutFile);
        fclose(OutFile);
    }
    else
    {
        Outf(Errors, "[ERROR] Unable to write output file %s.\n", OutputFileName);
    }
}

internal stream
ReadEntireFile(char *FileName, stream *Errors)
{
    buffer Buffer = {};
    
    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Buffer.Count = ftell(In);
        fseek(In, 0, SEEK_SET);
        
        Buffer.Data = (u8 *)malloc(Buffer.Count);
        fread(Buffer.Data, Buffer.Count, 1, In);
        fclose(In);
    }
    
    stream Result = MakeReadStream(Buffer, Errors);
    if(!In)
    {
        Outf(Result.Errors, "ERROR: Cannot open file %s.\n", FileName);
    }
    
    return(Result);
}

internal void
DumpStreamToCRT(stream *Source, FILE *Dest)
{
    for(stream_chunk *Chunk = Source->First;
        Chunk;
        Chunk = Chunk->Next)
    {
#if HANDMADE_INTERNAL
        fprintf(Dest, "%s: ", Chunk->GUID);
#endif
        fwrite(Chunk->Contents.Data, Chunk->Contents.Count, 1, Dest);
    }
}

PLATFORM_ALLOCATE_MEMORY(CRTAllocateMemory)
{
    umm TotalSize = sizeof(platform_memory_block) + Size;
    platform_memory_block *Block = (platform_memory_block *)malloc(TotalSize);
    memset(Block, 0, TotalSize);
    
    Block->Size = Size;
    Block->Base = (u8 *)(Block + 1);
    Block->Used = 0;
    
    return(Block);
}

PLATFORM_DEALLOCATE_MEMORY(CRTDeallocateMemory)
{
    if(Block)
    {
        free(Block);
    }
}

internal image_u32
ExtractImage(image_u32 SourceImage,
             u32 MinX, u32 MinY, u32 OnePastMaxX, u32 OnePastMaxY,
             memory_arena *TempArena)
{
    image_u32 Result = PushImage(TempArena, OnePastMaxX - MinX, OnePastMaxY - MinY);
    
    u32 *DestPixel = Result.Pixels;
    u32 *SourceRow = (SourceImage.Pixels + (OnePastMaxY - 1)*SourceImage.Width + MinX);
    for(u32 Y = 0;
        Y < Result.Height;
        ++Y)
    {
        u32 *SourcePixel = SourceRow;
        for(u32 X = 0;
            X < Result.Width;
            ++X)
        {
            u32 SourceC = *SourcePixel++;
            *DestPixel++ = SourceC;
        }
        
        SourceRow -= SourceImage.Width;
    }
    
    return(Result);
}

internal void
TestMultiTileImport(image_u32 Image, memory_arena *TempArena, stream *InfoStream)
{
    u32 BorderDim = 8;
    u32 TileDim = 1024;
    
    u32 XCountMax = 16;
    u32 YCountMax = 16;
    
    u32 XCount = Image.Width / TileDim;
    if(XCount > XCountMax)
    {
        printf("Tile column count of %u exceeds maximum of %u columns\n",
               XCount, XCountMax);
        
        XCount = XCountMax;
    }
    
    u32 YCount = Image.Height / TileDim;
    if(YCount > YCountMax)
    {
        printf("Tile row count of %u exceeds maximum of %u rows\n",
               YCount, YCountMax);
        
        YCount = YCountMax;
    }
    
    for(u32 YIndex = 0;
        YIndex < YCount;
        ++YIndex)
    {
        for(u32 XIndex = 0;
            XIndex < XCount;
            ++XIndex)
        {
            u32 MinX = U32Max;
            u32 MaxX = U32Min;
            u32 MinY = U32Max;
            u32 MaxY = U32Min;
            
            {
                u32 *SourceRow = Image.Pixels + YIndex*TileDim*Image.Width + XIndex*TileDim;
                for(u32 Y = 0;
                    Y < TileDim;
                    ++Y)
                {
                    u32 *SourcePixel = SourceRow;
                    for(u32 X = 0;
                        X < TileDim;
                        ++X)
                    {
                        u32 SourceC = *SourcePixel++;
                        if(SourceC & 0xFF000000)
                        {
                            MinX = Minimum(MinX, X);
                            MaxX = Maximum(MaxX, X);
                            MinY = Minimum(MinY, Y);
                            MaxY = Maximum(MaxY, Y);
                        }
                    }
                    
                    SourceRow += Image.Width;
                }
            }
            
            if(MinX <= MaxX)
            {
                // NOTE(casey): There was something in this tile
                
                if(MinX >= BorderDim)
                {
                    MinX -= BorderDim;
                }
                else
                {
                    MinX = 0;
                    
                    printf("Tile %u,%u extends into left %u-pixel border\n",
                           XIndex, YIndex, BorderDim);
                }
                
                if(MaxX < (TileDim - BorderDim))
                {
                    MaxX += BorderDim;
                }
                else
                {
                    MaxX = TileDim - 1;
                    
                    printf("Tile %u,%u extends into right %u-pixel border\n",
                           XIndex, YIndex, BorderDim);
                }
                
                if(MinY >= BorderDim)
                {
                    MinY -= BorderDim;
                }
                else
                {
                    MinY = 0;
                    
                    printf("Tile %u,%u extends into top %u-pixel border\n",
                           XIndex, YIndex, BorderDim);
                }
                
                if(MaxY < (TileDim - BorderDim))
                {
                    MaxY += BorderDim;
                }
                else
                {
                    MaxY = TileDim - 1;
                    
                    printf("Tile %u,%u extends into bottom %u-pixel border\n",
                           XIndex, YIndex, BorderDim);
                }
                
                rectangle2i Extract;
                Extract.MinX = XIndex*TileDim + MinX;
                Extract.MinY = YIndex*TileDim + MinY;
                Extract.MaxX = XIndex*TileDim + MaxX + 1;
                Extract.MaxY = YIndex*TileDim + MaxY + 1;
                
                image_u32 Extracted = ExtractImage(Image, Extract.MinX, Extract.MinY, Extract.MaxX, Extract.MaxY, TempArena);
                
                printf("EXTRACTION[%u,%u]: %u, %u -> %u, %u (%ux%u)\n",
                       XIndex, YIndex,
                       Extract.MinX, Extract.MinY, Extract.MaxX, Extract.MaxY,
                       Extracted.Width, Extracted.Height);
                
                char OutRGB[256];
                char OutA[256];
                sprintf(OutRGB, "extract%u%u_rgb.bmp", XIndex, YIndex);
                sprintf(OutA, "extract%u%u_alpha.bmp", XIndex, YIndex);
                
                WriteImageTopDownRGBA(Extracted.Width, Extracted.Height, (u8 *)Extracted.Pixels, OutRGB,
                                      PixelOp_SwapR|PixelOp_Invert,
                                      InfoStream);
                WriteImageTopDownRGBA(Extracted.Width, Extracted.Height, (u8 *)Extracted.Pixels, OutA,
                                      PixelOp_ReplA|PixelOp_ThresholdA,
                                      InfoStream);
            }
        }
    }
}

platform_api Platform;
int
main(int ArgCount, char **Args)
{
    Platform.AllocateMemory = CRTAllocateMemory;
    Platform.DeallocateMemory = CRTDeallocateMemory;
    
    memory_arena Memory = {};
    stream ErrorStream = OnDemandMemoryStream(&Memory);
    stream InfoStream = OnDemandMemoryStream(&Memory, &ErrorStream);
    
    if(ArgCount == 4)
    {
        char *InFileName = Args[1];
        char *OutRGB = Args[2];
        char *OutA = Args[3];
        
        Outf(&InfoStream, "Loading PNG %s...\n", InFileName);
        stream File = ReadEntireFile(InFileName, &ErrorStream);
        image_u32 Image = ParsePNG(&Memory, File, &InfoStream);
        
#if 0
        TestMultiTileImport(Image, &Memory, &InfoStream);
#endif
        
        Outf(&InfoStream, "Writing BMP %s...\n", OutRGB);
        WriteImageTopDownRGBA(Image.Width, Image.Height, (u8 *)Image.Pixels, OutRGB,
                              PixelOp_SwapR|PixelOp_Invert,
                              &InfoStream);
        Outf(&InfoStream, "Writing BMP %s...\n", OutA);
        WriteImageTopDownRGBA(Image.Width, Image.Height, (u8 *)Image.Pixels, OutA,
                              PixelOp_ReplA,
                              &InfoStream);
    }
    else
    {
        Outf(&ErrorStream, "Usage: %s (png file to load) (rgb bmp file to write) (alpha bmp file to write)\n", Args[0]);
    }
    
    fprintf(stdout, "Info:\n");
    DumpStreamToCRT(&InfoStream, stdout);
    fprintf(stdout, "\nErrors:\n");
    DumpStreamToCRT(&ErrorStream, stderr);
    
    return(0);
}
