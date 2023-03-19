#include "graphics.h"
#include "game.h"

void writeRectangle(GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    Colour colour)
{
    // Bounds checking
    if (xOffset >= buffer->widthPx) {
        return;
    }

    if (yOffset >= buffer->heightPx) {
        return;
    }

    // Min x
    if (xOffset < 0) {
        width = (width - (xOffset * -1));
        if (width <= 0) {
            return;
        }
        xOffset = 0;
    }

    // Min y
    if (yOffset < 0) {
        height = (height - (yOffset * -1));
        if (height <= 0) {
            return;
        }
        yOffset = 0;
    }

    // Max x
    int64 maxX = (xOffset + width);

    if (maxX > buffer->widthPx) {
        maxX = (buffer->widthPx - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    int64 maxY = (yOffset + height);

    if (maxY > buffer->heightPx) {
        maxY = (buffer->heightPx - yOffset);
        if (height > maxY) {
            height = maxY;
        }
    }

    // Set the colour
    uint32 alpha    = ((uint32)(255.0f * colour.a) << 24);
    uint32 red      = ((uint32)(255.0f * colour.r) << 16);
    uint32 green    = ((uint32)(255.0f * colour.g) << 8);
    uint32 blue     = ((uint32)(255.0f * colour.b) << 0);

    uint32 hexColour = (alpha | red | green | blue);

    // Write the memory
    uint32 *row = (uint32*)buffer->memory;

    // Move to last row as starting position (bottom left of axis)
    row = (row + ((buffer->widthPx * buffer->heightPx) - buffer->widthPx));

    // Move up to starting row
    row = (row - (buffer->widthPx * yOffset));

    // Move in from left to starting absolutePosition
    row = (row + xOffset);

    // Up (rows) y
    for (int64 i = 0; i < height; i++) {

        // Accross (columns) x
        uint32 *pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            *pixel = hexColour;
            pixel = (pixel + 1);
        }

        // Move up one entire row
        row = (row - buffer->widthPx);
    }
}

// 

/**
 * Byte order: AA RR GG BB (in little endian)
 * Image rows are in bottom-up order
 * Only supports 32-bit aligned bytes
 * 
 * @param buffer 
 * @param xOffset 
 * @param yOffset 
 * @param width 
 * @param height 
 * @param bytes 
*/
void writeBitmap(GameFrameBuffer *buffer,
                int64 xOffset,
                int64 yOffset,
                int64 width,
                int64 height,
                BitmapFile bitmapFile)
{
    int64 originalXOffset = xOffset;
    int64 originalYOffset = yOffset;
    int64 originalWidth = width;

    // Bounds checking
    if (xOffset >= buffer->widthPx) {
        return;
    }

    if (yOffset >= buffer->heightPx) {
        return;
    }

    // Min x
    if (xOffset < 0) {
        width = (width - (xOffset * -1));
        if (width <= 0) {
            return;
        }
        xOffset = 0;
    }

    // Min y
    if (yOffset < 0) {
        height = (height - (yOffset * -1));
        if (height <= 0) {
            return;
        }
        yOffset = 0;
    }

    // Max x
    int64 maxX = (xOffset + width);

    if (maxX > buffer->widthPx) {
        maxX = (buffer->widthPx - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    int64 maxY = (yOffset + height);

    if (maxY > buffer->heightPx) {
        maxY = (buffer->heightPx - yOffset);
        if (height > maxY) {
            height = maxY;
        }
    }

    // Write the memory...
    uint32 *row = (uint32*)buffer->memory;

    // Move to last row as starting position (bottom left of axis)
    row = (row + ((buffer->widthPx * buffer->heightPx) - buffer->widthPx));

    // Move up to starting row
    row = (row - (buffer->widthPx * yOffset));

    // Move in from left to starting absolutePosition
    row = (row + xOffset);

    uint32 *imagePixel = (uint32 *)bitmapFile.memory;

    if (originalXOffset < 0) {
        imagePixel = (imagePixel + (originalXOffset*-1));
    }

    if (originalYOffset < 0) {
        imagePixel = (imagePixel + ((originalYOffset*-1) * originalWidth));
    }

    bool32 shiftRes;
    uint32 redShift;
    uint32 greenShift;
    uint32 blueShift;
    uint32 alphaShift;

    shiftRes = intrinBitScanForward(&redShift, bitmapFile.redMask);
    if (!shiftRes){
        assert(!"Error finding red mask bit shift");
    }

    shiftRes = intrinBitScanForward(&greenShift, bitmapFile.greenMask);
    if (!shiftRes){
        assert(!"Error finding green mask bit shift");
    }

    shiftRes = intrinBitScanForward(&blueShift, bitmapFile.blueMask);
    if (!shiftRes){
        assert(!"Error finding blue mask bit shift");
    }

    shiftRes = intrinBitScanForward(&alphaShift, bitmapFile.alphaMask);
    if (!shiftRes){
        assert(!"Error finding alpha mask bit shift");
    }

    // Up (rows) y
    for (int64 y = 0; y < height; y++) {

        // Accross (columns) x
        uint32 *pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            // Re order the bytes
            uint8 red       = (((*imagePixel & bitmapFile.redMask) >> redShift) & 0xFF);
            uint8 green     = (((*imagePixel & bitmapFile.greenMask) >> greenShift) & 0xFF);
            uint8 blue      = (((*imagePixel & bitmapFile.blueMask) >> blueShift) & 0xFF);
            uint8 alpha     = (((*imagePixel & bitmapFile.alphaMask) >> alphaShift) & 0xFF);

            uint32 packed = 0;
            packed |= blue;
            packed |= green << 8;
            packed |= red << 16;
            packed |= alpha << 24;

            *pixel++ = packed;
            imagePixel++;
        }

        // Move up one entire row
        row = (row - buffer->widthPx);

        if (originalXOffset < 0) {
            imagePixel = (imagePixel + (originalXOffset*-1));
        }else if ((originalXOffset + originalWidth) > buffer->widthPx) {
            imagePixel = (imagePixel + ((originalWidth + originalXOffset) - buffer->widthPx));
        }
    }
}