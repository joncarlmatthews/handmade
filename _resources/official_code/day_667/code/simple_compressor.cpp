/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/* TODO(casey):

   Hey code friends!  This is extremely rudimentary compression code that
   only does the barest minimum of work.  There are lots of fun things you
   can do with this code to try and make it better.  Here are some suggestions:

   1) Figure out an efficient to expand the LZ encoding to support > 255
      size lookback and/or runs
   2) Add an entropy backend like Huffman, Arithmetic, something from the ANS family
   3) Add a hash lookup or other acceleration structure to the LZ encoder
      so that it isn't unusably slow
   4) Add better heuristics to the LZ compressor to get closer to an optimal
      parse
   5) Add preconditioners to test whether something better can be done
      for bitmaps (like differencing, deinterleaving by 4, etc.)
   6) Add the concept of switchable compression mid-stream to allow different
      blocks to be encoded with different methods
*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

typedef char unsigned u8;

enum stat_type
{
    Stat_Literal,
    Stat_Repeat,
    Stat_Copy,
    
    Stat_Count,
};
static char *GetStatName(stat_type Type)
{
#define StringCase(a) case a: {return((char *)(#a + 5));}
    switch(Type)
    {
        StringCase(Stat_Literal);
        StringCase(Stat_Repeat);
        StringCase(Stat_Copy);
    }
    
    return("unknown");
}

struct stat
{
    size_t Count;
    size_t Total;
};

struct stat_group
{
    size_t UncompressedBytes;
    size_t CompressedBytes;
    stat Stats[Stat_Count];
};

struct file_contents
{
    size_t FileSize;
    u8 *Contents;
};

#define COMPRESS_HANDLER(name) size_t name(stat_group *Stats, size_t InSize, u8 *InBase, size_t MaxOutSize, u8 *OutBase)
#define DECOMPRESS_HANDLER(name) void name(size_t InSize, u8 *InBase, size_t OutSize, u8 *OutBase)

typedef COMPRESS_HANDLER(compress_handler);
typedef DECOMPRESS_HANDLER(decompress_handler);

struct compressor
{
    char *Name;
    compress_handler *Compress;
    decompress_handler *Decompress;
};

static file_contents
ReadEntireFileIntoMemory(char *FileName)
{
    file_contents Result = {};
    
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        // TODO(casey): 64-bit safe file size in CRT?
        Result.FileSize = ftell(File);
        fseek(File, 0, SEEK_SET);

        Result.Contents = (u8 *)malloc(Result.FileSize);
        fread(Result.Contents, Result.FileSize, 1, File);
        
        fclose(File);
    }
    else
    {
        fprintf(stderr, "Unable to read file %s\n", FileName);
    }

    return(Result);
}

static double
Percent(size_t Num, size_t Den)
{
    double Result = 0.0f;
    
    if(Den != 0)
    {
        Result = (double)Num / (double)Den;
    }
    
    return(100.0*Result);
}

static void
PrintStats(stat_group *Stats)
{
    if(Stats->UncompressedBytes > 0)
    {
        fprintf(stdout, "Compression: %.0f -> %.0f (%.02f%%)\n", 
                (double)Stats->UncompressedBytes, 
                (double)Stats->CompressedBytes,
                Percent(Stats->CompressedBytes, Stats->UncompressedBytes));
        
        for(int StatIndex = 0;
            StatIndex < Stat_Count;
            ++StatIndex)
        {
            stat *Stat = Stats->Stats + StatIndex;
            if(Stat->Count)
            {
                fprintf(stdout, "%s: %.0f %.0f\n",
                        GetStatName((stat_type)StatIndex),
                        (double)Stat->Count, (double)Stat->Total);
            }
        }
    }
}

inline void
Increment(stat_group *Stats, stat_type Type, size_t Value)
{
    ++Stats->Stats[Type].Count;
    Stats->Stats[Type].Total += Value;
}

static size_t
GetMaximumCompressedOutputSize(size_t InSize)
{
    // TODO(casey): Actually figure out the equation for _our_ compressor
    size_t OutSize = 256 + 8*InSize;
    return(OutSize);
}

static void
Copy(size_t Size, u8 *Source, u8 *Dest)
{
    while(Size--)
    {
        *Dest++ = *Source++;
    }
}

COMPRESS_HANDLER(RLECompress)
{
    u8 *Out = OutBase;
    
#define MAX_LITERAL_COUNT 255
#define MAX_RUN_COUNT 255
    int LiteralCount = 0;
    u8 Literals[MAX_LITERAL_COUNT];

    u8 *In = InBase;
    u8 *InEnd = In + InSize;
    while(In <= InEnd)
    {
        u8 StartingValue = (In == InEnd) ? 0 : In[0];
        size_t Run = 0;
        while((Run < (size_t)(InEnd - In)) &&
              (Run < MAX_RUN_COUNT) &&
              (In[Run] == StartingValue))
        {
            ++Run;
        }
        
        if((In == InEnd) || (Run > 1) || (LiteralCount == MAX_LITERAL_COUNT))
        {
            Increment(Stats, Stat_Literal, LiteralCount);
            Increment(Stats, Stat_Repeat, Run);
            
            // NOTE(casey): Output a literal/run pair
            u8 LiteralCount8 = (u8)LiteralCount;
            assert(LiteralCount8 == LiteralCount);
            *Out++ = LiteralCount8;
            
            for(int LiteralIndex = 0;
                LiteralIndex < LiteralCount;
                ++LiteralIndex)
            {
                *Out++ = Literals[LiteralIndex];
            }
            LiteralCount = 0;
            
            u8 Run8 = (u8)Run;
            assert(Run8 == Run);
            *Out++ = Run8;
            
            *Out++ = StartingValue;
            
            In += Run;
            
            if(In == InEnd)
            {
                break;
            }
        }
        else
        {
            // NOTE(casey): Buffer literals
            Literals[LiteralCount++] = StartingValue;
            
            ++In;
        }
    }
#undef MAX_LITERAL_COUNT
#undef MAX_RUN_COUNT
    
    assert(In == InEnd);
    assert(LiteralCount == 0);
    
    size_t OutSize = Out - OutBase;
    assert(OutSize <= MaxOutSize);
    
    return(OutSize);
}

DECOMPRESS_HANDLER(RLEDecompress)
{
    u8 *Out = OutBase;
    u8 *In = InBase;
    u8 *InEnd = In + InSize;
    while(In < InEnd)
    {
        int LiteralCount = *In++;
        while(LiteralCount--)
        {
            *Out++ = *In++;
        }
        
        int RepCount = *In++;
        u8 RepValue = *In++;
        while(RepCount--)
        {
            *Out++ = RepValue;
        }
    }
    
    assert(In == InEnd);
}

COMPRESS_HANDLER(LZCompress)
{
    u8 *Out = OutBase;
    u8 *In = InBase;
    
#define MAX_LOOKBACK_COUNT 255
#define MAX_LITERAL_COUNT 255
#define MAX_RUN_COUNT 255
    int LiteralCount = 0;
    u8 Literals[MAX_LITERAL_COUNT];

    u8 *InEnd = In + InSize;
    while(In <= InEnd)
    {
        size_t MaxLookback = In - InBase;
        if(MaxLookback > MAX_LOOKBACK_COUNT)
        {
            MaxLookback = MAX_LOOKBACK_COUNT;
        }
        
        size_t BestRun = 0;
        size_t BestDistance = 0;
        for(u8 *WindowStart = In - MaxLookback;
            WindowStart < In;
            ++WindowStart)
        {
            size_t WindowSize = InEnd - WindowStart;
            if(WindowSize > MAX_RUN_COUNT)
            {
                WindowSize = MAX_RUN_COUNT;
            }
            
            u8 *WindowEnd = WindowStart + WindowSize;
            u8 *TestIn = In;
            u8 *WindowIn = WindowStart;
            size_t TestRun = 0;
            while((WindowIn < WindowEnd) && (*TestIn++ == *WindowIn++))
            {
                ++TestRun;
            }
            
            if(BestRun < TestRun)
            {
                BestRun = TestRun;
                BestDistance = In - WindowStart;
            }
        }
        
        bool OutputRun = false;
        if(LiteralCount)
        {
            OutputRun = (BestRun > 4);
        }
        else
        {
            OutputRun = (BestRun > 2);
        }
        
        if((In == InEnd) || OutputRun || (LiteralCount == MAX_LITERAL_COUNT))
        {
            // NOTE(casey): Flush
            u8 LiteralCount8 = (u8)LiteralCount;
            assert(LiteralCount8 == LiteralCount);
            if(LiteralCount8)
            {
                Increment(Stats, Stat_Literal, LiteralCount);
                
                *Out++ = LiteralCount8;
                *Out++ = 0;
                
                for(int LiteralIndex = 0;
                    LiteralIndex < LiteralCount;
                    ++LiteralIndex)
                {
                    *Out++ = Literals[LiteralIndex];
                }
                LiteralCount = 0;
            }
            
            if(OutputRun)
            {
                Increment(Stats, (BestDistance >= BestRun) ? Stat_Copy : Stat_Repeat, BestRun);
                
                u8 Run8 = (u8)BestRun;
                assert(Run8 == BestRun);
                *Out++ = Run8;
                
                u8 Distance8 = (u8)BestDistance;
                assert(Distance8 == BestDistance);
                *Out++ = Distance8;
                
                In += BestRun;
            }
            
            if(In == InEnd)
            {
                break;
            }
        }
        else
        {
            // NOTE(casey): Buffer literals
            Literals[LiteralCount++] = *In++;
        }
    }
#undef MAX_LITERAL_COUNT
#undef MAX_RUN_COUNT
#undef MAX_LOOKBACK_COUNT
    
    assert(In == InEnd);
    
    size_t OutSize = Out - OutBase;
    assert(OutSize <= MaxOutSize);
    
    return(OutSize);
}

DECOMPRESS_HANDLER(LZDecompress)
{
    u8 *In = InBase;
    u8 *Out = OutBase;
    u8 *InEnd = In + InSize;
    while(In < InEnd)
    {
        int Count = *In++;
        u8 CopyDistance = *In++;
        
        u8 *Source = (Out - CopyDistance);
        if(CopyDistance == 0)
        {
            Source = In;
            In += Count;
        }
        
        while(Count--)
        {
            *Out++ = *Source++;
        }
    }
    
    assert(In == InEnd);
}

compressor Compressors[] =
{
    {"rle", RLECompress, RLEDecompress},
    {"lz", LZCompress, LZDecompress},
};

int
main(int ArgCount, char **Args)
{
    if(ArgCount == 5)
    {
        size_t FinalOutputSize = 0;
        u8 *FinalOutputBuffer = 0;
        
        char *CodecName = Args[1];
        char *Command = Args[2];
        char *InFilename = Args[3];
        char *OutFilename = Args[4];
        
        stat_group Stats = {};

        compressor *Compressor = 0;
        for(int CompressorIndex = 0;
            CompressorIndex < (sizeof(Compressors)/sizeof(Compressors[0]));
            ++CompressorIndex)
        {
            compressor *TestCompressor = Compressors + CompressorIndex;
            if(strcmp(CodecName, TestCompressor->Name) == 0)
            {
                Compressor = TestCompressor;
                break;
            }
        }
        
        if(Compressor)
        {
            file_contents InFile = ReadEntireFileIntoMemory(InFilename);
            if(strcmp(Command, "compress") == 0)
            {
                size_t OutBufferSize = GetMaximumCompressedOutputSize(InFile.FileSize);
                u8 *OutBuffer = (u8 *)malloc(OutBufferSize + 4);
                size_t CompressedSize =
                    Compressor->Compress(&Stats, InFile.FileSize, InFile.Contents, OutBufferSize, OutBuffer + 4);
                *(int unsigned *)OutBuffer = (int unsigned)InFile.FileSize;
                
                FinalOutputSize = CompressedSize + 4;
                FinalOutputBuffer = OutBuffer;
                
                Stats.UncompressedBytes = InFile.FileSize;
                Stats.CompressedBytes = CompressedSize;
            }
            else if(strcmp(Command, "decompress") == 0)
            {
                if(InFile.FileSize >= 4)
                {
                    size_t OutBufferSize = *(int unsigned *)InFile.Contents;
                    u8 *OutBuffer = (u8 *)malloc(OutBufferSize);
                    Compressor->Decompress(InFile.FileSize - 4, InFile.Contents + 4,
                               OutBufferSize, OutBuffer);
                    
                    FinalOutputSize = OutBufferSize;
                    FinalOutputBuffer = OutBuffer;
                }
                else
                {
                    fprintf(stderr, "Invalid input file\n");
                }
            }
            else if(strcmp(Command, "test") == 0)
            {
                size_t OutBufferSize = GetMaximumCompressedOutputSize(InFile.FileSize);
                u8 *OutBuffer = (u8 *)malloc(OutBufferSize);
                u8 *TestBuffer = (u8 *)malloc(InFile.FileSize);
                size_t CompressedSize =
                    Compressor->Compress(&Stats, InFile.FileSize, InFile.Contents, OutBufferSize, OutBuffer);
                Compressor->Decompress(CompressedSize, OutBuffer, InFile.FileSize, TestBuffer);
                if(memcmp(InFile.Contents, TestBuffer, InFile.FileSize) == 0)
                {
                    fprintf(stderr, "Success!\n");
                }
                else
                {
                    fprintf(stderr, "Failure :(\n");
                }
                
                Stats.UncompressedBytes = InFile.FileSize;
                Stats.CompressedBytes = CompressedSize;
            }
            else
            {
                fprintf(stderr, "Unrecognized command %s\n", Command);
            }
        }
        else
        {
            fprintf(stderr, "Unrecognized compressor %s\n", CodecName);
        }
        
        if(FinalOutputBuffer)
        {
            FILE *OutFile = fopen(OutFilename, "wb");
            if(OutFile)
            {
                fwrite(FinalOutputBuffer, 1, FinalOutputSize, OutFile);
            }
            else
            {
                fprintf(stderr, "Unable to open output file %s\n", OutFilename);
            }
        }
    
        PrintStats(&Stats);
    }
    else
    {
        fprintf(stderr, "Usage: %s [algorithm] compress [raw filename] [compressed filename]\n",
                Args[0]);
        fprintf(stderr, "       %s [algorithm] decompress [compressed filename] [raw filename]\n",
                Args[0]);
        fprintf(stderr, "       %s [algorithm] test [raw filename] [compressed filename]\n",
                Args[0]);
        
        for(int CompressorIndex = 0;
            CompressorIndex < (sizeof(Compressors)/sizeof(Compressors[0]));
            ++CompressorIndex)
        {
            compressor *Compressor = Compressors + CompressorIndex;
            fprintf(stderr, "[algorithm] = %s\n", Compressor->Name);
        }
    }
}
