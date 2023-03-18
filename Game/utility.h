#ifndef HEADER_HH_UTILITIES
#define HEADER_HH_UTILITIES

#include "types.h"

/**
 * Simple function to calculate one number as a percentage of another.
 *
 * @param float32 a What is (a) as a percentage of...
 * @param float32 b ?
 * @return float32
 */
float32 percentageOfAnotherf(float32 a, float32 b);

uint32 u32RoundUpDivide(uint32 dividend, uint32 divisor);

int32 i32RoundUpDivide(int32 dividend, int32 divisor);

#endif