#ifndef HEADER_HH_INTRINSICS
#define HEADER_HH_INTRINSICS

#include "game_types.h"

typedef struct bitScanResult
{
    bool32 found;
    uint32 index; // aka shift
} bitScanResult;

float64 intrin_sin(float64 radians);

float32 intrin_ceilf(float32 num);

float32 intrin_sqrtf(float32 num);

int32 intrin_roundF32ToI32(float32 val);

uint32 intrin_roundF32ToUI32(float32 val);

int32 intrin_floorF32ToI32(float32 val);

int32 intrin_truncateF32ToI32(float32 val);

bitScanResult intrin_bitScanForward(uint32 mask);

#endif