#include "filesystem.h"

#if HANDMADE_LOCAL_BUILD

void DEBUGReadBMP(PlatformThreadContext *thread,
                    DEBUGPlatformReadEntireFile *playformreadFile,
                    const char *filename,
                    bitmapFile *bitmapFIle)
{

    DEBUG_file file = playformreadFile(thread, filename);

    if (file.sizeinBytes <= 0) {
        assert(!"Cannot read BMP");
        return;
    }

    bitmapFileHeader *fileHeader = (bitmapFileHeader *)file.memory;

    if (fileHeader->bfType != BITMAP_FILE_ID) {
        assert(!"Invalid BMP file format");
        return;
    }

    bitmapInfoHeader *fileInfo = (bitmapInfoHeader *)((uint8 *)file.memory + sizeof(bitmapFileHeader));

    void *memory = (void *)((uint8 *)file.memory + fileHeader->bfOffBits);

    bitmapFIle->heightPx = fileInfo->biHeight;
    bitmapFIle->widthPx = fileInfo->biWidth;
    bitmapFIle->fileSize = fileInfo->biSizeImage;
    bitmapFIle->memory = memory;

    return;
}

#endif