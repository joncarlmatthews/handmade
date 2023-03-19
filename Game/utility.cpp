#include "utility.h"

uint32 u32RoundUpDivide(uint32 dividend, uint32 divisor)
{
    return (uint32)intrinCeilf((float32)dividend / (float32)divisor);
}

int32 i32RoundUpDivide(int32 dividend, int32 divisor)
{
    return (int32)intrinCeilf((float32)dividend / (float32)divisor);
}

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
float32 percentageOfAnotherf(float32 a, float32 b)
{
    if (b <= 0) {
        return 0.0f;
    }

    float32 fract = (a / b);
    return (fract * 100.0f);
}