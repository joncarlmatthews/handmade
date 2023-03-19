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
* @param index *uint32
* @param mask uint32
* @return res bool32
*/
bool32 intrinBitScanForward(uint32 *index, uint32 mask)
{
    bool32 found = 0;

    if (0 == mask) {
        return found;
    }

    for (uint32 testBit = 0; testBit <= 32; testBit++){
        uint32 testShift = (1 << testBit);
        uint32 res = (mask & testShift);
        if (res) {
            *index = testBit;
            found = 1;
            break;
        }
    }

    return found;
}