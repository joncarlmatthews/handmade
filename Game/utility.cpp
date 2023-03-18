#include "utility.h"

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