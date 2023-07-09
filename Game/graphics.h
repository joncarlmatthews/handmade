#ifndef HEADER_HH_GRAPHICS
#define HEADER_HH_GRAPHICS

#include "types.h"
#include "filesystem.h"

//
// Graphics
//====================================================

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

#endif