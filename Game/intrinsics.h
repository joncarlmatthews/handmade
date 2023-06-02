#ifndef HEADER_HH_INTRINSICS
#define HEADER_HH_INTRINSICS

#include "types.h"

typedef struct bitScanResult
{
    bool32 found;
    uint32 index; // aka shift
} bitScanResult;

float64 intrin_sin(float64 radians);

float32 intrin_ceilf(float32 num);

float32 intrin_sqrtf(float32 num);

int32 intrin_roundFloat32ToInt32(float32 val);

uint32 intrin_roundFloat32ToUInt32(float32 val);

int32 intrin_floorFloat32ToInt32(float32 val);

int32 intrin_truncateFloat32ToInt32(float32 val);

bitScanResult intrin_bitScanForward(uint32 mask);

#endif