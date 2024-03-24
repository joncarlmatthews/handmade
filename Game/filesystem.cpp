#include "macros.h"
#include "filesystem.h"

size_t utilKibibytesToBytes(uint32 kibibytes)
{
    return (size_t)((uint32)1024 * kibibytes);
}

size_t utilMebibytesToBytes(uint32 mebibytes)
{
    return (size_t)(((uint32)1024 * utilKibibytesToBytes(1)) * mebibytes);
}

size_t utilGibibytesToBytes(uint32 gibibytes)
{
    return (size_t)(((uint32)1024 * utilMebibytesToBytes(1)) * gibibytes);
}

size_t utilTebibyteToBytes(uint32 tebibytes)
{
    return (size_t)(((uint32)1024 * utilGibibytesToBytes(1)) * tebibytes);
}

#if HANDMADE_LOCAL_BUILD

    void DEBUGReadBMP(PlatformThreadContext *thread,
                        DEBUGPlatformReadEntireFile *playformreadFile,
                        wchar_t *abspath,
                        wchar_t *filename,
                        BitmapFile *bitmapFIle)
    {

        DEBUG_file file = playformreadFile(thread, abspath, filename);

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

        if (fileInfo->biSize != BITMAP_INFO_HEADER_V5_SIZE) {
            assert(!"Unsupported bitmap version. Can only load BITMAPV5HEADER BMP files");
        }

        bitmapInfoHeaderV5 *fileInfoV5 = (bitmapInfoHeaderV5 *)fileInfo;

        void *memory = (void *)((uint8 *)file.memory + fileHeader->bfOffBits);

        bitmapFIle->heightPx = fileInfo->biHeight;
        bitmapFIle->widthPx = fileInfo->biWidth;
        bitmapFIle->fileSize = fileInfo->biSizeImage;
        bitmapFIle->redMask = fileInfoV5->bV5RedMask;
        bitmapFIle->greenMask = fileInfoV5->bV5GreenMask;
        bitmapFIle->blueMask = fileInfoV5->bV5BlueMask;
        bitmapFIle->alphaMask = fileInfoV5->bV5AlphaMask;
        bitmapFIle->memory = memory;

        return;
    }

#endif