#ifndef HEADER_HH_GRAPHICS
#define HEADER_HH_GRAPHICS

#include "types.h"
#include "math.h"
#include "filesystem.h"

//
// Graphics
//====================================================

// Colour struct. Supports a hex colour or a colour that is comprised of a series
// of normalised RGBA values between 0.0 and 1.0.
// E.g. {0x336996} -or- {0.0f, 0.2f, 0.7f, 1.0f}
// To specify an RGBA colour, set the hex member to 0.0f
typedef struct Colour {
    float32 hex;
    float32 r; // Between 0.0f and 1.0f
    float32 g; // Between 0.0f and 1.0f
    float32 b; // Between 0.0f and 1.0f
    float32 a; // Between 0.0f and 1.0f
} Colour;

/**
* Write a rectangle into the frame buffer
*
* @param xOffset   Offset, in pixels along the x axis to start drawing from
* @param yOffset   Offset, in pixels along the y axis to start drawing from
* @param width     Width, in pixels, to draw
* @param height    Height, in pixels, to draw
* @return void
*/
typedef struct GameFrameBuffer GameFrameBuffer;
void writeRectangleInt(GameFrameBuffer *buffer,
                        int32 xOffset,
                        int32 yOffset,
                        uint32 width,
                        uint32 height,
                        Colour colour);

void writeRectangle(GameFrameBuffer *buffer,
                    float32 xOffsetf,
                    float32 yOffsetf,
                    uint32 width,
                    uint32 height,
                    Colour colour);

/**
 * Writes a bitmap into a frame buffer. Supports alpha blending.
 *
 * @param buffer        Frame back buffer
 * @param xOffset       x coordinate to start drawing the bitmap from
 * @param yOffset       y coordinate to start drawing the bitmap from
 * @param width         Width to draw the bitmap at
 * @param height        Height to draw the bitmap at
 * @param alignX        Move the bitmap N number of pixels left/right on the x axis
 * @param alignY        Move the bitmap N number of pixels up/down on the y axis
 * @param bitmapFile    The BitmapFile object
*/
void writeBitmap(GameFrameBuffer* buffer,
                    float32 xOffsetf,
                    float32 yOffsetf,
                    float32 widthf,
                    float32 heightf,
                    float32 alignXf,
                    float32 alignYf,
                    BitmapFile bitmapFile);

void drawVector(GameFrameBuffer *frameBuffer, Vec2 vector, Colour colour);

#endif