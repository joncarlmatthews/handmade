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

// Only supports 32-bit aligned bytes
void writeBytes(GameFrameBuffer *buffer,
                int64 xOffset,
                int64 yOffset,
                int64 width,
                int64 height,
                uint32 *bytes)
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

    // Write the memory...
    uint32 *row = (uint32*)buffer->memory;

    // Move to last row as starting position (bottom left of axis)
    row = (row + ((buffer->widthPx * buffer->heightPx) - buffer->widthPx));

    // Move up to starting row
    row = (row - (buffer->widthPx * yOffset));

    // Move in from left to starting absolutePosition
    row = (row + xOffset);

    //uint32 *imagePixel = (bytes + (width * height));
    uint32 *imagePixel = bytes;

    // Up (rows) y
    for (int64 y = 0; y < height; y++) {

        // Accross (columns) x
        uint32 *pixel = (uint32*)row;
        for (int64 x = 0; x < width; x++) {

            uint8 alpha    = (*imagePixel >> 24) & 0xff;
            uint8 red      = (*imagePixel >> 16) & 0xff;
            uint8 green    = (*imagePixel >> 8) & 0xff;
            uint8 blue     = (*imagePixel & 0xff);

            uint32 packed = 0;
            packed |= blue;           // pack a into the least significant byte
            packed |= green << 8;      // pack b into the second least significant byte
            packed |= red << 16;     // pack c into the third least significant byte
            packed |= alpha << 24;     // pack d into the most significant byte

            *++pixel = packed;
            ++imagePixel;
        }

        // Move up one entire row
        row = (row - buffer->widthPx);
    }
}