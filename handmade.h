#ifndef HEADER_HANDMADE
#define HEADER_HANDMADE

/**
 * Struct for the screen buffer
 */
typedef struct GameFrameBuffer
{
    // The width in pixels of the buffer.
    uint32_t width;

    // The height in pixels of the buffer.
    uint32_t height;

    // 1 byte each for R, G & B and 1 byte for padding to match byte boundries (4)
    // Therefore our pixels are always 32-bits wide and are in Little Endian 
    // memory order (backwards) 0xBBGGRRPP
    uint16_t bytesPerPixel;

    // The number of bytes per row. (width * bytesPerPixel)
    uint32_t byteWidthPerRow;

    // Pointer to an allocated block of heap memory to hold the data of the buffer.
    void* memory;
} FrameBuffer;

internal_func void writeFrameBuffer(FrameBuffer* buffer, int redOffset, int greenOffset);
internal_func void updateAndRender(FrameBuffer* buffer, int redOffset, int greenOffset);

#endif