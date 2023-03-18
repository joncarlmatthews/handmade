#ifndef HEADER_HH_FILESYSTEM
#define HEADER_HH_FILESYSTEM

#include "types.h"

#define BITMAP_FILE_ID 0x4D42
#define BITMAP_INFO_HEADER_V5_SIZE 124

#pragma pack(push, 1)
typedef struct bitmapFileHeader
{
    uint16 bfType;          // specifies the file type
    uint32 bfSize;          // specifies the size in bytes of the bitmap file
    uint16 bfReserved1;     // reserved; must be 0
    uint16 bfReserved2;     // reserved; must be 0
    uint32 bfOffBits;       // specifies the offset in bytes from the bitmapfileheader to the bitmap bits
} bitmapFileHeader;
#pragma pack(pop)

#pragma pack(push, 1)
// @see https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader
typedef struct bitmapInfoHeader
{
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

#pragma pack(push, 1)
typedef struct rgbEndpoint
{
    int32 ciexyzX;
    int32 ciexyzY;
    int32 ciexyzZ;
} rgbEndpoint;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct rgbEndpoints
{
    rgbEndpoint ciexyzRed;
    rgbEndpoint ciexyzGreen;
    rgbEndpoint ciexyzBlue;
} rgbEndpoints;
#pragma pack(pop)

#pragma pack(push, 1)
// @see https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapv5header
typedef struct bitmapInfoHeaderV5
{
    uint32          bV5Size;
    int32           bV5Width;
    int32           bV5Height;
    uint16          bV5Planes;
    uint16          bV5BitCount;
    uint32          bV5Compression;
    uint32          bV5SizeImage;
    int32           bV5XPelsPerMeter;
    int32           bV5YPelsPerMeter;
    uint32          bV5ClrUsed;
    uint32          bV5ClrImportant;
    uint32          bV5RedMask;
    uint32          bV5GreenMask;
    uint32          bV5BlueMask;
    uint32          bV5AlphaMask;
    uint32          bV5CSType;
    rgbEndpoints    bV5Endpoints;
    uint32          bV5GammaRed;
    uint32          bV5GammaGreen;
    uint32          bV5GammaBlue;
    uint32          bV5Intent;
    uint32          bV5ProfileData;
    uint32          bV5ProfileSize;
    uint32          bV5Reserved;
} bitmapInfoHeaderV5;
#pragma pack(pop)

typedef struct BitmapFile
{
    uint32 heightPx;
    uint32 widthPx;
    uint32 fileSize;
    uint32 redMask;
    uint32 greenMask;
    uint32 blueMask;
    uint32 alphaMask;
    void *memory;
} BitmapFile;

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
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) DEBUG_file name(PlatformThreadContext *thread, const char *absPath, const char *filename)
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
                    const char *absPath,
                    const char *filename,
                    BitmapFile *bitmapFIle);

#endif

#endif