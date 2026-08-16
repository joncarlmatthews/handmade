/* ========================================================================
   $File: C:\work\handmade\code\hhfont.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
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
#include "handmade_file_formats.h"
#include "handmade_stream.cpp"
#include "handmade_image.cpp"
#include "handmade_png.cpp"
#include "handmade_file_formats.cpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>
#include <windows.h>

struct glyph_result
{
    v2 AlignPercentage;
    f32 KerningChange;
    f32 CharAdvance;
    
    u32 Width;
    u32 Height;
    u32 *Pixels;
};

struct codepoint_mask
{
    u32 OnePastMaxCodepoint;
    u32 GlyphCount;
    u32 *CodepointFromGlyph;
};

internal void
DataStreamToFILE(stream *Source, FILE *Dest)
{
    for(stream_chunk *Chunk = Source->First;
        Chunk;
        Chunk = Chunk->Next)
    {
        fwrite(Chunk->Contents.Data, Chunk->Contents.Count, 1, Dest);
    }
}

internal glyph_result
LoadGlyphBitmap(HDC DeviceContext, void *FontBits, u32 CodePoint, v2u MaxGlyphDim, f32 tmDescent, void *OutMemory, u32 Scale)
{
    memset(FontBits, 0x00, MaxGlyphDim.x*MaxGlyphDim.y*sizeof(u32));
    
    wchar_t CheesePoint = (wchar_t)CodePoint;
    
    SIZE Size;
    GetTextExtentPoint32W(DeviceContext, &CheesePoint, 1, &Size);
    
    int PreStepX = 128;
    
    u32 BoundWidth = Size.cx + 2*PreStepX;
    if(BoundWidth > MaxGlyphDim.x)
    {
        BoundWidth = MaxGlyphDim.x;
    }
    
    u32 BoundHeight = Size.cy;
    if(BoundHeight > MaxGlyphDim.y)
    {
        BoundHeight = MaxGlyphDim.y;
    }
    
    TextOutW(DeviceContext, PreStepX, 0, &CheesePoint, 1);
    if(Scale > 1)
    {
        BoundWidth /= Scale;
        BoundHeight /= Scale;
        u32 *Row = (u32 *)FontBits + (MaxGlyphDim.y - 1)*MaxGlyphDim.x;
        u32 *SampleRow = (u32 *)FontBits + (MaxGlyphDim.y - 1)*MaxGlyphDim.x;
        for(u32 Y = 0;
            Y < BoundHeight;
            ++Y)
        {
            u32 *Pixel = Row;
            u32 *Sample = SampleRow;
            for(u32 X = 0;
                X < BoundWidth;
                ++X)
            {
                u32 Accum = 0;
                u32 *SampleInner = Sample;
                for(u32 YOffset = 0;
                    YOffset < Scale;
                    ++YOffset)
                {
                    for(u32 XOffset = 0;
                        XOffset < Scale;
                        ++XOffset)
                    {
                        Accum += (SampleInner[XOffset] & 0xFF);
                    }
                    
                    SampleInner -= MaxGlyphDim.x;
                }
                Accum /= (Scale*Scale); // TODO(casey): Round this value?
                
                *Pixel++ = Accum;
                Sample += Scale;
            }
            Row -= MaxGlyphDim.x;
            SampleRow -= Scale*MaxGlyphDim.x;
        }
    }
    
    u32 MinX = S32Max;
    u32 MinY = S32Max;
    u32 MaxX = 0;
    u32 MaxY = 0;
    
    u32 *Row = (u32 *)FontBits + (MaxGlyphDim.y - 1)*MaxGlyphDim.x;
    for(u32 Y = 0;
        Y < BoundHeight;
        ++Y)
    {
        u32 *Pixel = Row;
        for(u32 X = 0;
            X < BoundWidth;
            ++X)
        {
#if 0
            COLORREF RefPixel = GetPixel(GlobalFontDeviceContext, X, Y);
            Assert(RefPixel == *Pixel);
#endif
            if(*Pixel != 0)
            {
                if(MinX > X)
                {
                    MinX = X;
                }
                
                if(MinY > Y)
                {
                    MinY = Y;
                }
                
                if(MaxX < X)
                {
                    MaxX = X;
                }
                
                if(MaxY < Y)
                {
                    MaxY = Y;
                }
            }
            
            ++Pixel;
        }
        Row -= MaxGlyphDim.x;
    }
    
    glyph_result Result = {};
    
    f32 KerningChange = 0;
    v2 AlignPercentage = {0.5f, 0.5f};
    if(MinX <= MaxX)
    {
        u32 Width = (MaxX - MinX) + 1;
        u32 Height = (MaxY - MinY) + 1;
        
        u32 BytesPerPixel = 4;
        
        u32 OutWidth = Width + 2;
        u32 OutHeight = Height + 2;
        u32 OutPitch = OutWidth*BytesPerPixel;
        
        memset(OutMemory, 0, OutHeight*OutPitch);
        
        Result.Width = OutWidth;
        Result.Height = OutHeight;
        Result.Pixels = (u32 *)OutMemory;
        
        u8 *DestRow = (u8 *)OutMemory + OutPitch;
        u32 *SourceRow = (u32 *)FontBits + (MaxGlyphDim.y - 1 - MinY)*MaxGlyphDim.x;
        for(u32 Y = MinY;
            Y <= MaxY;
            ++Y)
        {
            u32 *Source = (u32 *)SourceRow + MinX;
            u32 *Dest = (u32 *)DestRow + 1;
            for(u32 X = MinX;
                X <= MaxX;
                ++X)
            {
#if 0
                COLORREF Pixel = GetPixel(GlobalFontDeviceContext, X, Y);
                Assert(Pixel == *Source);
#else
                u32 Pixel = *Source;
#endif
                u32 Gray = (u32)(Pixel & 0xFF);
                *Dest++ = ((Gray << 24) | 0x00FFFFFF);
                ++Source;
            }
            
            DestRow += OutPitch;
            SourceRow -= MaxGlyphDim.x;
        }
        
        AlignPercentage = V2((1.0f) / (f32)OutWidth,
                             (1.0f + (f32)(MaxY - ((f32)BoundHeight - (f32)tmDescent))) /
                             (f32)OutHeight);
        
        KerningChange = (f32)MinX - (f32)PreStepX;
    }
    
#if 0
    ABC ThisABC;
    GetCharABCWidthsW(DeviceContext, CodePoint, CodePoint, &ThisABC);
    f32 CharAdvance = (f32)(ThisABC.abcA + ThisABC.abcB + ThisABC.abcC);
#else
    INT ThisWidth;
    GetCharWidth32W(DeviceContext, CodePoint, CodePoint, &ThisWidth);
    f32 CharAdvance = (f32)ThisWidth;
#endif
    
    Result.AlignPercentage = AlignPercentage;
    Result.KerningChange = KerningChange;
    Result.CharAdvance = CharAdvance;
    
    return(Result);
}

internal void
Sanitize(char *Source, char *Dest)
{
    while(*Source)
    {
        int D = tolower(*Source);
        if(((D >= 'a') &&
            (D <= 'z')) ||
           ((D >= '0') &&
            (D <= '9')))
        {
            *Dest = (char)D;
        }
        else
        {
            *Dest = '_';
        }
        
        ++Dest;
        ++Source;
    }
    
    *Dest = 0;
}

internal void
ExtractFont(char *TTFFileName, char *FontName, u32 PixelHeight, codepoint_mask *CodePointMask,
            FILE *HHTOut, char *PNGDestDir)
{
    char *NameStem = (char *)malloc(strlen(FontName) + 1);
    Sanitize(FontName, NameStem);
    
    char *PNGOutName = (char *)malloc(strlen(NameStem) + strlen(PNGDestDir) + 128);
    char *PNGFileNameOnly = PNGOutName + strlen(PNGDestDir) + 1;
    
    u32 GlyphCount = CodePointMask->GlyphCount;
    u32 OnePastMaxFontCodepoint = CodePointMask->OnePastMaxCodepoint;
    u32 *GlyphCodePoint = CodePointMask->CodepointFromGlyph;
    
    //
    // NOTE(casey): Load and select the requested font
    //
    
    //
    // NOTE(casey): Windows has some secret ideas about when it's going to
    // antialias its fonts and when it won't.  Sure, they have a flag you pass
    // that says whether you want it antialiased, but it doesn't actually give
    // a crap about that flag.  That says it _may_ antialias, not it _will_.
    // Once a font gets "too large" (their words), it just won't.
    //
    // So we need to put in a value that will do our own internal antialiasing
    // when Windows' craps out, but we don't actually know where that point
    // is, because presumably it will vary from install to install, or machine
    // to machine, or even just based on the DPI setting (we have no idea).
    // On the Handmade Hero machine, the changeover point was 353->354,
    // So we know that we definitely want a number _below_ that.
    //
    // Since we know we can supersample fonts that are relatively large, we
    // choose to supersample anything over 128-high as a safe cut-off.
    // Anything below that, and we assume the TTF sampler can do a better
    // job and we _assume_ that it will actually antialias (fingers crossed).
    //
    
    u32 Scale = (PixelHeight > 128) ? 4 : 1;
    
    f32 ScaleRatio = 1.0f / (f32)Scale;
    u32 SamplePixelHeight = Scale*PixelHeight;
    DWORD iQuality = ANTIALIASED_QUALITY;
    if(Scale > 1)
    {
        iQuality = 0;
    }
    
    HDC DeviceContext = CreateCompatibleDC(GetDC(0));
    AddFontResourceExA(TTFFileName, FR_PRIVATE, 0);
    HFONT Win32Handle = CreateFontA(-(s32)SamplePixelHeight, 0, 0, 0,
                                    FW_NORMAL, // NOTE(casey): Weight
                                    FALSE, // NOTE(casey): Italic
                                    FALSE, // NOTE(casey): Underline
                                    FALSE, // NOTE(casey): Strikeout
                                    DEFAULT_CHARSET,
                                    OUT_DEFAULT_PRECIS,
                                    CLIP_DEFAULT_PRECIS,
                                    iQuality,
                                    DEFAULT_PITCH|FF_DONTCARE,
                                    FontName);
    if(Win32Handle)
    {
        SelectObject(DeviceContext, Win32Handle);
        
        TEXTMETRIC TextMetric;
        GetTextMetrics(DeviceContext, &TextMetric);
        
        // NOTE(casey): These are arbitrarily padded because I'm not sure what kind
        // of shenanigans Microsoft may pull when they report "sizes".
        v2u MaxGlyphDim =
        {
            256 + 2*((u32)TextMetric.tmMaxCharWidth + (u32)TextMetric.tmOverhang),
            256 + 2*((u32)TextMetric.tmHeight + (u32)TextMetric.tmOverhang),
        };
        
        void *OutMemory = malloc(MaxGlyphDim.x*MaxGlyphDim.y*sizeof(u32));
        
        //
        // NOTE(casey): Set up our Windows rendering buffer
        //
        
        void *FontBits = 0;
        
        BITMAPINFO Info = {};
        Info.bmiHeader.biSize = sizeof(Info.bmiHeader);
        Info.bmiHeader.biWidth = MaxGlyphDim.x;
        Info.bmiHeader.biHeight = MaxGlyphDim.y;
        Info.bmiHeader.biPlanes = 1;
        Info.bmiHeader.biBitCount = 32;
        Info.bmiHeader.biCompression = BI_RGB;
        Info.bmiHeader.biSizeImage = 0;
        Info.bmiHeader.biXPelsPerMeter = 0;
        Info.bmiHeader.biYPelsPerMeter = 0;
        Info.bmiHeader.biClrUsed = 0;
        Info.bmiHeader.biClrImportant = 0;
        
        HBITMAP Bitmap = CreateDIBSection(DeviceContext, &Info, DIB_RGB_COLORS, &FontBits, 0, 0);
        SelectObject(DeviceContext, Bitmap);
        SetBkColor(DeviceContext, RGB(0, 0, 0));
        SetTextColor(DeviceContext, RGB(255, 255, 255));
        
        u32 MinCodePoint = INT_MAX;
        u32 MaxCodePoint = 0;
        
        u32 GlyphIndexFromCodePointSize = OnePastMaxFontCodepoint*sizeof(u32);
        
        u32 *GlyphIndexFromCodePoint = (u32 *)malloc(GlyphIndexFromCodePointSize);
        memset(GlyphIndexFromCodePoint, 0, GlyphIndexFromCodePointSize);
        
        hha_font_glyph *Glyphs = (hha_font_glyph *)malloc(sizeof(hha_font_glyph)*GlyphCount);
        u32 HorizontalAdvanceCount = GlyphCount*GlyphCount;
        size_t HorizontalAdvanceSize = sizeof(r32)*GlyphCount*GlyphCount;
        r32 *HorizontalAdvance = (r32 *)malloc(HorizontalAdvanceSize);
        memset(HorizontalAdvance, 0, HorizontalAdvanceSize);
        
        u32 OnePastHighestCodepoint = 0;
        
        // NOTE(casey): Reserve space for the null glyph
        Glyphs[0].UnicodeCodePoint = 0;
        Glyphs[0].BitmapID = 0;
        
        f32 AscenderHeight = (f32)TextMetric.tmAscent;
        f32 DescenderHeight = (f32)TextMetric.tmDescent;
        f32 ExternalLeading = (f32)TextMetric.tmExternalLeading;
        
        DWORD KerningPairCount = GetKerningPairsW(DeviceContext, 0, 0);
        KERNINGPAIR *KerningPairs = (KERNINGPAIR *)malloc(KerningPairCount*sizeof(KERNINGPAIR));
        GetKerningPairsW(DeviceContext, KerningPairCount, KerningPairs);
        for(DWORD KerningPairIndex = 0;
            KerningPairIndex < KerningPairCount;
            ++KerningPairIndex)
        {
            KERNINGPAIR *Pair = KerningPairs + KerningPairIndex;
            if((Pair->wFirst < OnePastMaxFontCodepoint) &&
               (Pair->wSecond < OnePastMaxFontCodepoint))
            {
                u32 First = GlyphIndexFromCodePoint[Pair->wFirst];
                u32 Second = GlyphIndexFromCodePoint[Pair->wSecond];
                if((First != 0) && (Second != 0))
                {
                    HorizontalAdvance[First*GlyphCount + Second] += (f32)Pair->iKernAmount;
                }
            }
        }
        
        fprintf(HHTOut, "font \"%s\"\n", NameStem);
        fprintf(HHTOut, "{\n");
        fprintf(HHTOut, "    GlyphCount = %u;\n", GlyphCount);
        fprintf(HHTOut, "    AscenderHeight = %f;\n", ScaleRatio*AscenderHeight);
        fprintf(HHTOut, "    DescenderHeight = %f;\n", ScaleRatio*DescenderHeight);
        fprintf(HHTOut, "    ExternalLeading = %f;\n", ScaleRatio*ExternalLeading);
        
        f32 tmDescent = (f32)TextMetric.tmDescent;
        for(u32 GlyphIndex = 1;
            GlyphIndex < GlyphCount;
            ++GlyphIndex)
        {
            u32 CodePoint = GlyphCodePoint[GlyphIndex];
            glyph_result Glyph =
                LoadGlyphBitmap(DeviceContext, FontBits, CodePoint, MaxGlyphDim, tmDescent, OutMemory, Scale);
            
            sprintf(PNGOutName, "%s/%s_%04u.png", PNGDestDir, NameStem, CodePoint);
            
            memory_arena TempArena = {};
            stream PNGStream = OnDemandMemoryStream(&TempArena);
            WritePNG(Glyph.Width, Glyph.Height, Glyph.Pixels, &PNGStream);
            FILE *PNGOutFile = fopen(PNGOutName, "wb");
            if(PNGOutFile)
            {
                DataStreamToFILE(&PNGStream, PNGOutFile);
                fclose(PNGOutFile);
            }
            else
            {
                fprintf(stderr, "ERROR: Unable to open \"%s\" for writing.\n", PNGOutName);
            }
            Clear(&TempArena);
            
            hha_align_point AlignPoint = {};
            SetAlignPoint(&AlignPoint, HHAAlign_Default, true, 1.0f,
                          V2(Glyph.AlignPercentage.x, Glyph.AlignPercentage.y));
            fprintf(HHTOut, "    Glyph[%u] = \"%s\", %u, {%u, %u};\n", GlyphIndex, PNGFileNameOnly,
                    CodePoint, AlignPoint.PPercent[0], AlignPoint.PPercent[1]);
            
            for(u32 OtherGlyphIndex = 0;
                OtherGlyphIndex < GlyphCount;
                ++OtherGlyphIndex)
            {
                HorizontalAdvance[GlyphIndex*GlyphCount + OtherGlyphIndex] += Glyph.CharAdvance - Glyph.KerningChange;
                if(OtherGlyphIndex != 0)
                {
                    HorizontalAdvance[OtherGlyphIndex*GlyphCount + GlyphIndex] += Glyph.KerningChange;
                }
            }
        }
        
        fprintf(HHTOut, "    HorizontalAdvance = \n        ");
        for(u32 Index = 0;
            Index < HorizontalAdvanceCount;
            ++Index)
        {
            if(Index)
            {
                if((Index % 16) == 0)
                {
                    fprintf(HHTOut, ",\n        ");
                }
                else
                {
                    fprintf(HHTOut, ", ");
                }
            }
            
            fprintf(HHTOut, "%f", ScaleRatio*HorizontalAdvance[Index]);
        }
        fprintf(HHTOut, ";\n");
        fprintf(HHTOut, "};\n");
    }
    else
    {
        fprintf(stderr, "Unable to load font %s from %s\n",
                FontName,
                TTFFileName);
    }
}

#define CHAR_SET_CREATOR(name) void name(codepoint_mask *Mask)
typedef CHAR_SET_CREATOR(char_set_creator_function);

internal void
Include(codepoint_mask *Mask,
        u32 E0, u32 E1 = 0, u32 E2 = 0, u32 E3 = 0,
        u32 E4 = 0, u32 E5 = 0, u32 E6 = 0, u32 E7 = 0)
{
    u32 E[] = {E0, E1, E2, E3, E4, E5, E6, E7};
    for(u32 EIndex = 0;
        EIndex < ArrayCount(E);
        ++EIndex)
    {
        u32 CodePoint = E[EIndex];
        if(CodePoint)
        {
            if(Mask->CodepointFromGlyph)
            {
                Mask->CodepointFromGlyph[Mask->GlyphCount] = CodePoint;
            }
            
            if(Mask->OnePastMaxCodepoint <= CodePoint)
            {
                Mask->OnePastMaxCodepoint = CodePoint + 1;
            }
            
            ++Mask->GlyphCount;
        }
    }
}

internal void
IncludeRange(codepoint_mask *Mask, u32 MinCodePoint, u32 MaxCodePoint)
{
    for(u32 Character = MinCodePoint;
        Character <= MaxCodePoint;
        ++Character)
    {
        Include(Mask, Character);
    }
}

CHAR_SET_CREATOR(Test)
{
    Include(Mask, ' ');
    IncludeRange(Mask, '!', '~');
    
    // NOTE(casey): Ligatures
    IncludeRange(Mask, 0xfb00, 0xfb05);
    
    // NOTE(casey): Kanji OWL!!!!!!!
    Include(Mask, 0x5c0f, 0x8033, 0x6728, 0x514e);
}

struct char_set_creator
{
    char *Name;
    char *Description;
    char_set_creator_function *Function;
};

global char_set_creator CharSets[] =
{
    {"Test", "Basic character set for testing font creation and display.", Test},
};

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

platform_api Platform;
int
main(int ArgCount, char **Args)
{
#if 0
    unsigned long crc_table[256];
    
    unsigned long c;
    int n, k;
    
    for (n = 0; n < 256; n++) {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    
    for(int x = 0;
        x < ArrayCount(crc_table);
        ++x)
    {
        if(x && ((x % 8) == 0))
        {
            fprintf(stdout, "\n    ");
        }
        
        fprintf(stdout, "0x%08x, ", crc_table[x]);
    }
    
    fprintf(stdout, "\n");
#endif
    
    Platform.AllocateMemory = CRTAllocateMemory;
    Platform.DeallocateMemory = CRTDeallocateMemory;
    
    // "c:/Windows/Fonts/arial.ttf", "Arial", 128
    // "c:/Windows/Fonts/LiberationMono-Regular.ttf", "Liberation Mono", 20
    
    b32x PrintUsage = true;
    
    if(ArgCount == 7)
    {
        char *TTFFileName = Args[1];
        char *FontName = Args[2];
        u32 PixelHeight = atoi(Args[3]);
        char *CharSetName = Args[4];
        char *HHTFileName = Args[5];
        char *PNGDirName = Args[6];
        
        char_set_creator *CharSetCreator = 0;
        
        for(u32 CharSetIndex = 0;
            CharSetIndex < ArrayCount(CharSets);
            ++CharSetIndex)
        {
            char_set_creator *Test = CharSets + CharSetIndex;
            if(strcmp(Test->Name, CharSetName) == 0)
            {
                CharSetCreator = Test;
                break;
            }
        }
        
        if(CharSetCreator)
        {
            FILE *HHTFile = fopen(HHTFileName, "wb");
            if(HHTFile)
            {
                fprintf(HHTFile,
                        "/* ========================================================================\n"
                        "   $File: %s $\n"
                        "   $Date: $\n"
                        "   $Revision: $\n"
                        "   $Creator: %s $\n"
                        "   $Notice: Extraction of font \"%s\" $\n"
                        "   ======================================================================== */\n",
                        HHTFileName, Args[0], FontName);
                
                codepoint_mask CounterMask = {};
                CounterMask.GlyphCount = 1;
                CharSetCreator->Function(&CounterMask);
                
                codepoint_mask Mask = {};
                Mask.GlyphCount = 1;
                size_t MaskSize = CounterMask.GlyphCount*sizeof(u32);
                Mask.CodepointFromGlyph = (u32 *)malloc(MaskSize);
                memset(Mask.CodepointFromGlyph, 0, MaskSize);
                
                CharSetCreator->Function(&Mask);
                
                fprintf(stdout, "Extracting font %s - %u glyphs, codepoint range %u... ",
                        FontName, Mask.GlyphCount, Mask.OnePastMaxCodepoint);
                ExtractFont(TTFFileName, FontName, PixelHeight, &Mask, HHTFile, PNGDirName);
                PrintUsage = false;
                fprintf(stdout, "done.\n");
                
                fclose(HHTFile);
            }
            else
            {
                fprintf(stderr, "ERROR: Unable to open HHT file \"%s\" for writing.\n", HHTFileName);
            }
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized character set \"%s\".\n", CharSetName);
        }
    }
    
    if(PrintUsage)
    {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "%s <TTF filename> <font name> <pixel height> <charset> <dest hht> <dest png dir>\n", Args[0]);
        fprintf(stderr, "\n");
        fprintf(stderr, "Supported <charset> values:\n");
        for(u32 CharSetIndex = 0;
            CharSetIndex < ArrayCount(CharSets);
            ++CharSetIndex)
        {
            fprintf(stderr, "%s - %s\n", CharSets[CharSetIndex].Name, CharSets[CharSetIndex].Description);
        }
    }
}
