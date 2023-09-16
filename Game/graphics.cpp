#include "graphics.h"
#include "game.h"

void writeRectangle(GameFrameBuffer *buffer,
                        float32 xOffsetf,
                        float32 yOffsetf,
                        uint32 width,
                        uint32 height,
                        Colour colour)
{
    assert(width >= 0.0f);
    assert(height >= 0.0f);

    int32 xOffset   = intrin_roundF32ToI32(xOffsetf);
    int32 yOffset   = intrin_roundF32ToI32(yOffsetf);

    // Bounds checking
    if (xOffset >= (int32)buffer->widthPx) {
        return;
    }

    if (yOffset >= (int32)buffer->heightPx) {
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
    uint32 maxX = (xOffset + width);

    if (maxX > buffer->widthPx) {
        maxX = (buffer->widthPx - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    uint32 maxY = (yOffset + height);

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
    for (uint32 i = 0; i < height; i++) {

        // Accross (columns) x
        uint32 *pixel = (uint32*)row;
        for (uint32 x = 0; x < width; x++) {

            *pixel = hexColour;
            pixel = (pixel + 1);
        }

        // Move up one entire row
        row = (row - buffer->widthPx);
    }
}

void writeRectangleInt(GameFrameBuffer* buffer,
                        int32 xOffset,
                        int32 yOffset,
                        uint32 width,
                        uint32 height,
                        Colour colour)
{
    writeRectangle(buffer, (float32)xOffset, (float32)yOffset, width, height, colour);
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
                    float32 xOffsetf,
                    float32 yOffsetf,
                    float32 widthf,
                    float32 heightf,
                    float32 alignXf,
                    float32 alignYf,
                    BitmapFile bitmapFile)
{
    assert(widthf >= 1.0f);
    assert(heightf >= 1.0f);

    int32 xOffset   = intrin_roundF32ToI32(xOffsetf);
    int32 yOffset   = intrin_roundF32ToI32(yOffsetf);
    int32 width     = intrin_roundF32ToI32(widthf);
    int32 height    = intrin_roundF32ToI32(heightf);
    int32 alignX    = intrin_roundF32ToI32(alignXf);
    int32 alignY    = intrin_roundF32ToI32(alignYf);
    
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

    int32 originalXOffset = xOffset;
    int32 originalYOffset = yOffset;
    int32 originalWidth = width;

    // Bounds checking
    if (xOffset >= (int32)buffer->widthPx) {
        return;
    }

    if (yOffset >= (int32)buffer->heightPx) {
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
    int32 maxX = (xOffset + width);

    if (maxX > (int32)buffer->widthPx) {
        maxX = ((int32)buffer->widthPx - xOffset);
        if (width > maxX) {
            width = maxX;
        }
    }

    // Max y
    int32 maxY = (yOffset + height);

    if (maxY > (int32)buffer->heightPx) {
        maxY = ((int32)buffer->heightPx - yOffset);
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
    bitScanResult redShift = intrin_bitScanForward(bitmapFile.redMask);
    if (!redShift.found){
        assert(!"Error finding red mask bit shift");
    }

    bitScanResult greenShift = intrin_bitScanForward(bitmapFile.greenMask);
    if (!greenShift.found){
        assert(!"Error finding green mask bit shift");
    }

    bitScanResult blueShift = intrin_bitScanForward(bitmapFile.blueMask);
    if (!blueShift.found){
        assert(!"Error finding blue mask bit shift");
    }

    bitScanResult alphaShift = intrin_bitScanForward(bitmapFile.alphaMask);
    if (!alphaShift.found){
        assert(!"Error finding alpha mask bit shift");
    }

    // Up (rows) y
    for (int32 y = 0; y < height; y++) {

        // Accross (columns) x
        uint32 *pixel = (uint32*)row;
        for (int32 x = 0; x < width; x++) {

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
        }else if ((originalXOffset + originalWidth) > (int32)buffer->widthPx) {
            imagePixel = (imagePixel + ((originalWidth + originalXOffset) - buffer->widthPx));
        }
    }
}

void drawVector(GameFrameBuffer *frameBuffer, Vector2 vector, Colour colour)
{
    // Vector origins
    uint32 vox = (FRAME_BUFFER_PIXEL_WIDTH / 2);
    uint32 voy = (FRAME_BUFFER_PIXEL_HEIGHT / 2);

    // Pixels per point
    float32 pixelsPerPoint = 10.0f;

    float32 v1mag = getVectorMagnitude(vector);

    float32 xfract = (vector.x / v1mag);
    float32 yfract = (vector.y / v1mag);

    for(size_t i = 0; i < ((size_t)((float64)v1mag * (float64)pixelsPerPoint)); i++){
        float32 x = ((float32)vox + ((float32)i * xfract));
        float32 y = ((float32)voy + ((float32)i * yfract));
        writeRectangle(frameBuffer,
            x,
            y,
            1,
            1,
            colour);
    }
}