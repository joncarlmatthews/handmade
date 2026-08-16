#if !defined(LANE_WIDTH)
#define LANE_WIDTH 8
#endif

// NOTE(casey): V3 is for STORAGE ONLY
struct v3
{
    f32 x, y, z;
};

//
// NOTE(casey): 8-wide SIMD
//
#if (LANE_WIDTH==8)

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SEE/NEON optimizations are not available for this compiler yet!!!!
#endif

#include "ray_lane_8x.h"

//
// NOTE(casey): 4-wide SIMD
//
#elif (LANE_WIDTH==4)

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SEE/NEON optimizations are not available for this compiler yet!!!!
#endif

#include "ray_lane_4x.h"

//
// NOTE(casey): 1-wide float
//
#elif (LANE_WIDTH==1)

typedef f32 lane_f32;
typedef u32 lane_u32;
typedef v3 lane_v3;

internal void
ConditionalAssign(lane_u32 *Dest, lane_u32 Mask, lane_u32 Source)
{
    Mask = (Mask ? 0xFFFFFFFF : 0);
    *Dest = ((~Mask & *Dest) | (Mask & Source));
}

internal void
ConditionalAssign(lane_f32 *Dest, lane_u32 Mask, lane_f32 Source)
{
    ConditionalAssign((lane_u32 *)Dest, Mask, *(lane_u32 *)&Source);
}

internal lane_f32
Max(lane_f32 A, lane_f32 B)
{
    lane_f32 Result = ((A > B) ? A : B);
    return(Result);
}

internal lane_f32
GatherF32_(void *BasePtr, u32 Stride, lane_u32 Index)
{
    lane_f32 Result = (*(f32 *)((u8 *)BasePtr + Index*Stride));
    return(Result);
}

internal b32x
MaskIsZeroed(lane_u32 LaneMask)
{
    b32x Result = (LaneMask == 0);
    return(Result);
}

internal u32
HorizontalAdd(lane_u32 A)
{
    u32 Result = A;
    return(Result);
}

internal f32
HorizontalAdd(lane_f32 A)
{
    f32 Result = A;
    return(Result);
}

internal lane_u32
LaneU32FromU32(lane_u32 A)
{
    lane_u32 Result = (lane_u32)A;
    return(Result);
}

internal lane_u32
LaneU32FromU32(u32 R0, u32 R1, u32 R2, u32 R3)
{
    lane_u32 Result;
    
    // TODO(casey): Is it really OK to just do it this way?
    Result = R0;
    
    return(Result);
}

internal lane_f32
LaneF32FromU32(lane_u32 A)
{
    lane_f32 Result = (lane_f32)A;
    return(Result);
}

internal lane_f32
LaneF32FromF32(f32 A)
{
    lane_f32 Result = A;
    return(Result);
}

internal lane_v3
operator&(lane_u32 A, lane_v3 B)
{
    lane_v3 Result;
    
    // TODO(casey): Maybe make this a specific function, because it is kind of misleading?
    
    A = (A ? 0xFFFFFFFF : 0);
    
    u32 x = A & *(u32 *)&B.x;
    u32 y = A & *(u32 *)&B.y;
    u32 z = A & *(u32 *)&B.z;
    
    Result.x = *(f32 *)&x;
    Result.y = *(f32 *)&y;
    Result.z = *(f32 *)&z;
    
    return(Result);
}

#else
#error Lane width must be set to 1, 4, or 8!
#endif

#if (LANE_WIDTH != 1)

// TODO(casey): Just do this outside, always
struct lane_v3
{
    lane_f32 x;
    lane_f32 y;
    lane_f32 z;
};

internal lane_f32 
operator+(lane_f32 A, f32 B)
{
    lane_f32 Result;
    
    Result = A + LaneF32FromF32(B);
    return(Result);
}

internal lane_f32 
operator+(f32 A, lane_f32 B)
{
    lane_f32 Result = LaneF32FromF32(A) + B;
    return(Result);
}

internal lane_f32 
operator-(lane_f32 A, f32 B)
{
    lane_f32 Result = A - LaneF32FromF32(B);
    return(Result);
}

internal lane_f32 
operator-(f32 A, lane_f32 B)
{
    lane_f32 Result = LaneF32FromF32(A) - B;
    return(Result);
}

internal lane_f32 
operator*(lane_f32 A, f32 B)
{
    lane_f32 Result = A * LaneF32FromF32(B);
    return(Result);
}

internal lane_f32 
operator*(f32 A, lane_f32 B)
{
    lane_f32 Result = LaneF32FromF32(A) * B;
    return(Result);
}

internal lane_f32 
operator/(lane_f32 A, f32 B)
{
    lane_f32 Result = A / LaneF32FromF32(B);
    return(Result);
}

internal lane_f32 
operator/(f32 A, lane_f32 B)
{
    lane_f32 Result = LaneF32FromF32(A) / B;
    return(Result);
}

internal lane_f32
operator+=(lane_f32 &A, lane_f32 B)
{
    A = A + B;
    return(A);
}

internal lane_u32
operator+=(lane_u32 &A, lane_u32 B)
{
    A = A + B;
    return(A);
}

internal lane_f32
operator-=(lane_f32 &A, lane_f32 B)
{
    A = A - B;
    return(A);
}

internal lane_f32
operator*=(lane_f32 &A, lane_f32 B)
{
    A = A * B;
    return(A);
}

internal lane_f32
operator/=(lane_f32 &A, lane_f32 B)
{
    A = A / B;
    return(A);
}

internal lane_u32
operator&=(lane_u32 &A, lane_u32 B)
{
    A = A & B;
    return(A);
}

internal lane_u32
operator^=(lane_u32 &A, lane_u32 B)
{
    A = A ^ B;
    return(A);
}

internal lane_u32
operator|=(lane_u32 &A, lane_u32 B)
{
    A = A | B;
    return(A);
}

lane_u32 &lane_u32::
operator=(u32 B)
{
    *this = LaneU32FromU32(B);
    return(*this);
}

lane_f32 &lane_f32::
operator=(f32 B)
{
    *this = LaneF32FromF32(B);
    return(*this);
}

internal lane_f32 
operator-(lane_f32 A)
{
    lane_f32 Result = LaneF32FromF32(0) - A;
    return(Result);
}

internal lane_v3
operator*(lane_v3 A, lane_f32 B)
{
    lane_v3 Result;
    
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    
    return(Result);
}

internal lane_v3
operator*(lane_f32 A, lane_v3 B)
{
    lane_v3 Result = B*A;
    return(Result);
}

internal lane_u32
operator>(lane_f32 A, f32 B)
{
    lane_u32 Result = (A > LaneF32FromF32(B));
    return(Result);
}

internal lane_u32
operator>(f32 A, lane_f32 B)
{
    lane_u32 Result = (LaneF32FromF32(A) > B);
    return(Result);
}

internal lane_u32
operator<(lane_f32 A, f32 B)
{
    lane_u32 Result = (A < LaneF32FromF32(B));
    return(Result);
}

internal lane_u32
operator<(f32 A, lane_f32 B)
{
    lane_u32 Result = (LaneF32FromF32(A) < B);
    return(Result);
}

internal lane_v3
operator&(lane_u32 A, lane_v3 B)
{
    lane_v3 Result;
    
    Result.x = A & B.x;
    Result.y = A & B.y;
    Result.z = A & B.z;
    
    return(Result);
}

internal void
ConditionalAssign(lane_u32 *Dest, lane_u32 Mask, lane_u32 Source)
{
    *Dest = (AndNot(Mask, *Dest) | (Mask & Source));
}

internal lane_f32
Clamp01(lane_f32 Value)
{
    lane_f32 Result = Min(Max(Value, LaneF32FromF32(0.0f)), LaneF32FromF32(1.0f));
    return(Result);
}

#endif

#define GatherF32(BasePtr, Index, Member) GatherF32_(&(BasePtr)->Member, sizeof(*(BasePtr)), Index)
#define GatherV3(BasePtr, Index, Member) GatherV3_(&(BasePtr)->Member, sizeof(*(BasePtr)), Index)

internal void
ConditionalAssign(lane_v3 *Dest, lane_u32 Mask, lane_v3 Source)
{
    ConditionalAssign(&Dest->x, Mask, Source.x);
    ConditionalAssign(&Dest->y, Mask, Source.y);
    ConditionalAssign(&Dest->z, Mask, Source.z);
}

internal v3
HorizontalAdd(lane_v3 A)
{
    v3 Result;
    
    Result.x = HorizontalAdd(A.x);
    Result.y = HorizontalAdd(A.y);
    Result.z = HorizontalAdd(A.z);
    
    return(Result);
}

internal lane_v3 
LaneV3FromV3(v3 A)
{
    lane_v3 Result;
    
    Result.x = LaneF32FromF32(A.x);
    Result.y = LaneF32FromF32(A.y);
    Result.z = LaneF32FromF32(A.z);
    
    return(Result);
}

internal v3
Extract0(lane_v3 A)
{
    v3 Result;
    
    Result.x = *(f32 *)&A.x;
    Result.y = *(f32 *)&A.y;
    Result.z = *(f32 *)&A.z;
    
    return(Result);
}

#define ExtractF32(A, I) ((f32 *)&A)[I]

internal lane_v3 
GatherV3_(void *BasePtr, u32 Stride, lane_u32 Indices)
{
    lane_v3 Result;
    
    // TODO(casey): If we care about the speed of this operation eventually,
    // we should write a custom V3 gather for each lane width!
    Result.x = GatherF32_((f32 *)BasePtr + 0, Stride, Indices);
    Result.y = GatherF32_((f32 *)BasePtr + 1, Stride, Indices);
    Result.z = GatherF32_((f32 *)BasePtr + 2, Stride, Indices);
    
    return(Result);
}
