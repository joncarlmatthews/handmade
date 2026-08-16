/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

//
// TODO(casey): Convert all of these to platform-efficient versions
// and remove math.h
//
// cosf
// sinf
// atan2f
// ceilf
// floorf
//
//

#include "math.h"

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

inline int32
SignOf(int32 Value)
{
    int32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline f32
SignOf(f32 Value)
{
    // TODO(casey): Look at octahedral code and figure out
    // if we're actually using SignOf for SignOrZero in places?
    // Because I don't see a SignOrZero and I feel like there
    // should be one!

    u32 MaskU32 = (u32)(1 << 31);
    __m128 Mask = _mm_set_ss(*(float *)&MaskU32);

    __m128 One = _mm_set_ss(1.0f);
    __m128 SignBit = _mm_and_ps(_mm_set_ss(Value), Mask);
    __m128 Combined = _mm_or_ps(One, SignBit);

    f32 Result = _mm_cvtss_f32(Combined);

    return(Result);
}

inline f32
SquareRoot(f32 Real32)
{
    f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(Real32)));
    return(Result);
}

inline f32
ReciprocalSquareRoot(f32 Real32)
{
    f32 Result = (1.0f / SquareRoot(Real32));
    return(Result);
}

inline real32
AbsoluteValue(real32 Real32)
{
    real32 Result = fabsf(Real32);
    return(Result);
}

inline uint32
RotateLeft(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotl(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return(Result);
}

inline u64
RotateLeft(u64 Value, s32 Amount)
{
#if COMPILER_MSVC
    u64 Result = _rotl64(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 63;
    u64 Result = ((Value << Amount) | (Value >> (64 - Amount)));
#endif

    return(Result);
}

inline uint32
RotateRight(uint32 Value, int32 Amount)
{
#if COMPILER_MSVC
    uint32 Result = _rotr(Value, Amount);
#else
    // TODO(casey): Actually port this to other compiler platforms!
    Amount &= 31;
    uint32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}

inline s32
RoundReal32ToInt32(f32 Real32)
{
    s32 Result = _mm_cvtss_si32(_mm_set_ss(Real32));
    return(Result);
}

inline u32
RoundReal32ToUInt32(real32 Real32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(Real32));
    return(Result);
}

inline int32
FloorReal32ToInt32(real32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    int32 Result = _mm_cvtss_si32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline f32
Round(f32 Real32)
{
    f32 Result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(Real32),
                               (_MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC)));
    return(Result);
}

inline f32
Floor(f32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    f32 Result = _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline int32
CeilReal32ToInt32(real32 Real32)
{
    // TODO(casey): Do we want to forgo the use of SSE 4.1?
    int32 Result = _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(Real32)));
    return(Result);
}

inline u32
Hash32(u64 A, u64 B = 0)
{
    u8 Seed[16] =
    {
        0xaa, 0x9b, 0xbd, 0xb8,
        0xa1, 0x98, 0xac, 0x3f,
        0x1f, 0x94, 0x07, 0xb3,
        0x8c, 0x27, 0x93, 0x69,
    };
    __m128i Hash = _mm_set_epi64x(A, B);
    Hash = _mm_aesdec_si128(Hash, _mm_loadu_si128((__m128i *)Seed));
    Hash = _mm_aesdec_si128(Hash, _mm_loadu_si128((__m128i *)Seed));
    u32 Result = _mm_extract_epi32(Hash, 0);
    return(Result);
}

inline int32
TruncateReal32ToInt32(real32 Real32)
{
    int32 Result = (int32)Real32;
    return(Result);
}

inline real32
Sin(real32 Angle)
{
    real32 Result = sinf(Angle);
    return(Result);
}

inline real32
Cos(real32 Angle)
{
    real32 Result = cosf(Angle);
    return(Result);
}

inline real32
ATan2(real32 Y, real32 X)
{
    real32 Result = atan2f(Y, X);
    return(Result);
}

struct bit_scan_result
{
    bool32 Found;
    uint32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(s32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}

inline bit_scan_result
FindMostSignificantSetBit(uint32 Value)
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found = _BitScanReverse((unsigned long *)&Result.Index, Value);
#else
    for(s32 Test = 32;
        Test > 0;
        --Test)
    {
        if(Value & (1 << (Test - 1)))
        {
            Result.Index = Test - 1;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}

internal void
SetDefaultFPBehavior(void)
{
#define FLUSH_TO_ZERO_BIT (1 << 15)
#define ROUNDING_CONTROL_BITS (3 << 13)
#define PRECISION_MASK (1 << 12)
#define UNDERFLOW_MASK (1 << 11)
#define OVERFLOW_MASK (1 << 10)
#define DBZ_MASK (1 << 9)
#define DENORMAL_OP_MASK (1 << 8)
#define INVALID_OP_MASK (1 << 7)
#define DENORMALS_ARE_ZERO (1 << 6)

    unsigned int FPControlMask = (FLUSH_TO_ZERO_BIT |
                                  // ROUNDING_CONTROL_BITS |
                                  PRECISION_MASK |
                                  UNDERFLOW_MASK |
                                  OVERFLOW_MASK |
                                  DBZ_MASK |
                                  DENORMAL_OP_MASK |
                                  INVALID_OP_MASK |
                                  DENORMALS_ARE_ZERO);
    unsigned int DesiredBits = FPControlMask;
    unsigned int OldControlBits = _mm_getcsr();
    unsigned int NewControlBits = (OldControlBits & ~FPControlMask) | DesiredBits;
    _mm_setcsr(NewControlBits);
}
