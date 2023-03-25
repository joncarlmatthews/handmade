#ifndef HEADER_HH_INTRINSICS
#define HEADER_HH_INTRINSICS

#include "types.h"

typedef struct bitScanResult
{
    bool32 found;
    uint32 index; // aka shift
} bitScanResult;

float64 intrinSin(float64 radians);

float32 intrinCeilf(float32 num);

bitScanResult intrinBitScanForward(uint32 mask);

#endif