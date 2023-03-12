#ifndef HEADER_HH_FILESYSTEM
#define HEADER_HH_FILESYSTEM

#include "types.h"

#define BITMAP_FILE_ID 0x4D42

#pragma pack(push, 1)
typedef struct bitmapFileHeader {
    uint16 bfType;          // specifies the file type
    uint32 bfSize;          // specifies the size in bytes of the bitmap file
    uint16 bfReserved1;     // reserved; must be 0
    uint16 bfReserved2;     // reserved; must be 0
    uint32 bfOffBits;       // specifies the offset in bytes from the bitmapfileheader to the bitmap bits
} bitmapFileHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct bitmapInfoHeader {
    uint32 biSize;          // specifies the number of bytes required by the struct
    int32 biWidth;          // specifies width in pixels
    int32 biHeight;         // specifies height in pixels
    uint16 biPlanes;        // specifies the number of color planes, must be 1
    uint16 biBitCount;      // specifies the number of bits per pixel
    uint32 biCompression;   // specifies the type of compression
    uint32 biSizeImage;     // size of image in bytes
    int32 biXPelsPerMeter;  // number of pixels per meter in x axis
    int32 biYPelsPerMeter;  // number of pixels per meter in y axis
    uint32 biClrUsed;       // number of colors used by the bitmap
    uint32 biClrImportant;  // number of colors that are important
} bitmapInfoHeader;
#pragma pack(pop)

typedef struct bitmapFile
{
    uint32 heightPx;
    uint32 widthPx;
    uint32 fileSize;
    void *memory;
} bitmapFile;

typedef struct PlatformThreadContext PlatformThreadContext;
typedef struct GameMemory GameMemory;

#if HANDMADE_LOCAL_BUILD

#define DEBUG_PLATFORM_LOG(name) void name(char *buff)
typedef DEBUG_PLATFORM_LOG(DEBUGPlatformLog);

typedef struct DEBUG_file
{
    void *memory;
    uint32 sizeinBytes;

} DEBUG_file;

/*
* Read an entire file into memory
*
* @note call DEBUG_platformFreeFileMemory in a subsequent call.
*/
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DEBUG_file name(PlatformThreadContext *thread, const char *filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile);

/*
* Free file memory read from DEBUG_platformReadEntireFile
*/
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(PlatformThreadContext *thread, DEBUG_file *file)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory);

/*
* Write bytes into a new file
*/
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) bool32 name(PlatformThreadContext *thread, char *filename, void *memory, uint32 memorySizeInBytes)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile);


void DEBUGReadBMP(PlatformThreadContext *thread,
                    DEBUGPlatformReadEntireFile *playformreadFile,
                    const char *filename,
                    bitmapFile *bitmapFIle);

#endif

#endif