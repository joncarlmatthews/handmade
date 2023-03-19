#ifndef HEADER_HH_INTRINSICS
#define HEADER_HH_INTRINSICS

/*
* TODO(JM) Convert all of these to platform - efficient versions and remove math.h
*/

#include "types.h"
#include <math.h> // @see https://www.cplusplus.com/reference/cmath/

float64 intrinSin(float64 radians);

float32 intrinCeilf(float32 num);

bool32 intrinBitScanForward(uint32 *index, uint32 mask);

#endif