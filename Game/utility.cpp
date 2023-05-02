#include "utility.h"

uint32 u32RoundUpDivide(uint32 dividend, uint32 divisor)
{
    return (uint32)intrinCeilf((float32)dividend / (float32)divisor);
}

int32 i32RoundUpDivide(int32 dividend, int32 divisor)
{
    return (int32)intrinCeilf((float32)dividend / (float32)divisor);
}