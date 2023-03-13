#ifndef HEADER_HH_GRAPHICS
#define HEADER_HH_GRAPHICS

#include "types.h"
#include "filesystem.h"

//
// Graphics
//====================================================

// FHD
#define FRAME_BUFFER_PIXEL_WIDTH  1920
#define FRAME_BUFFER_PIXEL_HEIGHT 1080

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
void writeRectangle(GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    Colour colour);

void writeBitmap(GameFrameBuffer *buffer,
                    int64 xOffset,
                    int64 yOffset,
                    int64 width,
                    int64 height,
                    BitmapFile bitmapFile);

#endif