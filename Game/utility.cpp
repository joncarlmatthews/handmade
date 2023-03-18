#include <math.h> // For ceilf
#include "utility.h"

/**
 * 1 KiB = 1,024 bytes
 */
uint64 utilKibibytesToBytes(uint8 kibibytes)
{
    return (uint64)((uint64)1024 * (uint64)kibibytes);
}

/**
 * 1 MiB = 1,048,576 bytes
 */
uint64 utilMebibytesToBytes(uint8 mebibytes)
{
    return (uint64)(((uint64)1024 * utilKibibytesToBytes(1)) * mebibytes);
}

/**
 * 1 GiB = 1,073,741,824 bytes
 */
uint64 utilGibibytesToBytes(uint8 gibibytes)
{
    return (uint64)(((uint64)1024 * utilMebibytesToBytes(1)) * gibibytes);
}

uint64 utilTebibyteToBytes(uint8 tebibytes)
{
    return (uint64)(((uint64)1024 * utilGibibytesToBytes(1)) * tebibytes);
}

float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b <= 0) {
        return 0.0f;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}

inline uint32 truncateU8(float32 f)
{
    return (uint8)f;
}

inline uint32 truncateU32(float32 f)
{
    return (uint32)f;
}

uint32 u32RoundUpDivide(uint32 dividend, uint32 divisor)
{
    return (uint32)ceilf((float32)dividend / (float32)divisor);
}

int32 i32RoundUpDivide(int32 dividend, int32 divisor)
{
    return (int32)ceilf((float32)dividend / (float32)divisor);
}

/**
 * Returns a shift based on a 32-bit integer bitmask.
 *
 * Valid masks:
 *   0x000000ff
 *   0x0000ff00
 *   0x00ff0000
 *   0xff000000
 * 
 * @param mask uint32
 * @return shift uint8
*/
uint8 getShiftFromMask(uint32 mask)
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