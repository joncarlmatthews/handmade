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

/**
 * Writes a bitmap into a frame buffer. Implements linear alpha blending by
 * blending with pixel data that exists directly below where the bitmap is
 * writting to.
 * 
 * - Image rows are in bottom-up order
 *  -Only supports 32-bit aligned bytes
 * 
 * @param buffer        Frame back buffer
 * @param xOffset       x coordinate to start drawing the bitmap from
 * @param yOffset       y coordinate to start drawing the bitmap from
 * @param width         Width to draw the bitmap at
 * @param height        Height to draw the bitmap at
 * @param alignX        Adjust the bitmap N number of pixels left/right on the x axis
 * @param alignY        Adjust the bitmap N number of pixels up/down on the y axis
 * @param bitmapFile    The BitmapFile object
*/
void writeBitmap(GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    int64 alignX,
                    int64 alignY,
                    BitmapFile bitmapFile)
{
    if (alignX < 0) {
        xOffset = (xOffset - (alignX * -1));
    }
    else if (alignX > 0) {
        xOffset = (xOffset + alignX);
    }

    if (alignY < 0) {
        yOffset = (yOffset - (alignY * -1));
    }
    else if (alignY > 0) {
        yOffset = (yOffset + alignY);
    }

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

    // Fetch the RGBA shifts from the bitmap's masks...
    bitScanResult redShift = intrinBitScanForward(bitmapFile.redMask);
    if (!redShift.found){
        assert(!"Error finding red mask bit shift");
    }

    bitScanResult greenShift = intrinBitScanForward(bitmapFile.greenMask);
    if (!greenShift.found){
        assert(!"Error finding green mask bit shift");
    }

    bitScanResult blueShift = intrinBitScanForward(bitmapFile.blueMask);
    if (!blueShift.found){
        assert(!"Error finding blue mask bit shift");
    }

    bitScanResult alphaShift = intrinBitScanForward(bitmapFile.alphaMask);
    if (!alphaShift.found){
        assert(!"Error finding alpha mask bit shift");
    }

    // Up (rows) y
    for (int64 y = 0; y < height; y++) {

        // Accross (columns) x
        uint32 *pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            // Extract RGBA values from bitmap.
            uint8 red       = (((*imagePixel & bitmapFile.redMask) >> redShift.index) & 0xFF);
            uint8 green     = (((*imagePixel & bitmapFile.greenMask) >> greenShift.index) & 0xFF);
            uint8 blue      = (((*imagePixel & bitmapFile.blueMask) >> blueShift.index) & 0xFF);
            uint8 alpha     = (((*imagePixel & bitmapFile.alphaMask) >> alphaShift.index) & 0xFF);

            // Extract RGB values from data that already exists at this location
            uint8 origRed = ((*pixel >> 16) & 0xFF);
            uint8 origGreen = ((*pixel >> 8) & 0xFF);
            uint8 origBlue = ((*pixel >> 0) & 0xFF);

            // Do linear alpha blend
            float32 alphaf = ((float32)alpha / 255.0f);

            float32 blendedR = ((float32)origRed - ((float32)origRed * alphaf) +
             ((float32)red * alphaf));
            float32 blendedG = ((float32)origGreen - ((float32)origGreen * alphaf) +
            ((float32)green * alphaf));
            float32 blendedB = ((float32)origBlue - ((float32)origBlue * alphaf) +
             ((float32)blue * alphaf));

            // Re order the bytes into order we need
            uint32 packed = 0;
            packed |= (uint32)blendedB;
            packed |= (uint32)blendedG << 8;
            packed |= (uint32)blendedR << 16;
            //packed |= alpha << 24; // We're not actually using the alpha channel yet when we blit to screen

            *pixel = packed;

            pixel++;
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