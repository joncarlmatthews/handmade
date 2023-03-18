#include "intrinsics.h"

float64 intrinSin(float64 radians)
{
    return sin(radians);
}

float32 intrinCeilf(float32 num)
{
    return ceilf(num);
}

uint32 u32RoundUpDivide(uint32 dividend, uint32 divisor)
{
    return (uint32)intrinCeilf((float32)dividend / (float32)divisor);
}

int32 i32RoundUpDivide(int32 dividend, int32 divisor)
{
    return (int32)intrinCeilf((float32)dividend / (float32)divisor);
}

/**
* Returns a bit shift based on a 32-bit integer bitmask. Function will
* find the least significant set bit.
*
* E.g:
*   0x000000ff = 0
*   0x0000ff00 = 8
*   0x00ff0000 = 16
*   0xff000000 = 24
* 
* @param mask uint32
* @return shift uint8
*/
uint8 intrinBitScanForward(uint32 mask)
{
    uint8 shift = 0;
    switch (mask) {
    case 0xff000000:
        shift = 24;
        break;
    case 0x00ff0000:
        shift = 16;
        break;
    case 0x0000ff00:
        shift = 8;
        break;
    case 0x000000ff:
        shift = 0;
        break;
    }
    return shift;
}