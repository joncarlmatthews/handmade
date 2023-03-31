/*
* TODO(JM) Convert all of these to platform efficient versions
* and remove math.h
*/

#include "global_macros.h"
#include "types.h"
#include "intrinsics.h"
#include <math.h> // @see https://www.cplusplus.com/reference/cmath/

float64 intrinSin(float64 radians)
{
    return sin(radians);
}

float32 intrinCeilf(float32 num)
{
    return ceilf(num);
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
bitScanResult intrinBitScanForward(uint32 mask)
{
    bitScanResult result = {0, 0};

    if (0 == mask) {
        return result;
    }

#if COMPILER_MSVC
    result.found = _BitScanForward((unsigned long *)&result.index, mask);
    return result;
#else

    for (uint32 testBit = 0; testBit <= 32; testBit++){
        uint32 testShift = (1 << testBit);
        uint32 res = (mask & testShift);
        if (res) {
            result.index = testBit;
            result.found = 1;
            break;
        }
    }

    return result;

#endif
}