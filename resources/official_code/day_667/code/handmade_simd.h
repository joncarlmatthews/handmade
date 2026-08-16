/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct f32_4x
{
    union
    {
        __m128 P;
        f32 E[4];
        u32 U32[4];
    };
};

struct v3_4x
{
    union
    {
        struct
        {
            union
            {
                f32_4x x;
                f32_4x r;
            };
            
            union
            {
                f32_4x y;
                f32_4x g;
            };
            
            union
            {
                f32_4x z;
                f32_4x b;
            };
        };
        
        f32_4x E[3];
    };
};

struct v4_4x
{
    union
    {
        struct
        {
            union
            {
                f32_4x x;
                f32_4x r;
            };
            
            union
            {
                f32_4x y;
                f32_4x g;
            };
            
            union
            {
                f32_4x z;
                f32_4x b;
            };
            
            union
            {
                f32_4x w;
                f32_4x a;
            };
        };
        
        f32_4x E[4];
    };
};

#define mmSquare(a) _mm_mul_ps(a, a)
// TODO(casey): We probably want to unroll the loops and make these
// macros use _mm_extract with an immediate...
#define M(a, i) ((float *)&(a))[i]
#define Mi(a, i) ((uint32 *)&(a))[i]

//
// NOTE(casey): f32_4x
//

inline f32_4x
LoadF32_4X(void *Address)
{
    f32_4x Result;
    
    Result.P = _mm_loadu_ps((f32 *)Address);
    
    return(Result);
}

inline void
StoreF32_4X(f32_4x A, void *Address)
{
    _mm_storeu_ps((f32 *)Address, A.P);
}

inline f32_4x
F32_4x(f32 EAll)
{
    f32_4x Result;
    
    Result.P = _mm_set1_ps(EAll);
    
    return(Result);
}

inline f32_4x
U32_4x(u32 EAll)
{
    f32_4x Result;
    
    Result.P = _mm_set1_ps(*(float *)&EAll);
    
    return(Result);
}

inline f32_4x
U32_4x(u32 E0, u32 E1, u32 E2, u32 E3)
{
    f32_4x Result;
    
    Result.P = _mm_setr_ps(*(float *)&E0,
                           *(float *)&E1,
                           *(float *)&E2,
                           *(float *)&E3);
    
    return(Result);
}

inline f32_4x
U32ToF32(f32_4x A)
{
    f32_4x Result;
    
    Result.P = _mm_cvtepi32_ps(_mm_castps_si128(A.P));
    
    return(Result);
}

inline f32_4x
F32_4x(f32 E0, f32 E1, f32 E2, f32 E3)
{
    f32_4x Result;
    
    Result.P = _mm_setr_ps(E0, E1, E2, E3);
    
    return(Result);
}

inline f32_4x
F32_4x(__m128 EAll)
{
    f32_4x Result;
    
    Result.P = EAll;
    
    return(Result);
}

inline f32_4x
ZeroF32_4x(void)
{
    f32_4x Result;
    
    Result.P = _mm_setzero_ps();
    
    return(Result);
}

inline f32_4x
operator+(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_add_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator-(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_sub_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator-(f32_4x A)
{
    f32_4x Result;
    
    Result = ZeroF32_4x() - A;
    
    return(Result);
};

inline f32_4x
operator*(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_mul_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator^(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_xor_ps(A.P, B.P);
    
    return(Result);
}

#define ShiftRight4X(A, Imm) F32_4x(_mm_castsi128_ps(_mm_srli_epi32( _mm_castps_si128(A.P), Imm)))
#define ShiftLeft4X(A, Imm) F32_4x(_mm_castsi128_ps(_mm_slli_epi32(_mm_castps_si128(A.P), Imm)))

inline f32_4x
operator/(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_div_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x &
operator^=(f32_4x &A, f32_4x B)
{
    A = A ^ B;
    
    return(A);
}

inline f32_4x &
operator+=(f32_4x &A, f32_4x B)
{
    A = A + B;
    
    return(A);
}

inline f32_4x &
operator-=(f32_4x &A, f32_4x B)
{
    A = A - B;
    
    return(A);
}

inline f32_4x &
operator*=(f32_4x &A, f32_4x B)
{
    A = A * B;
    
    return(A);
}

inline f32_4x &
operator/=(f32_4x &A, f32_4x B)
{
    A = A / B;
    
    return(A);
}

inline f32_4x
operator<(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_cmplt_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator<=(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_cmple_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator>(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_cmpgt_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator>=(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_cmpge_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator==(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_cmpeq_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator!=(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_cmpneq_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator&(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_and_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
operator|(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_or_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x &
operator&=(f32_4x &A, f32_4x B)
{
    A = A & B;
    
    return(A);
}

inline f32_4x &
operator|=(f32_4x &A, f32_4x B)
{
    A = A | B;
    
    return(A);
}

inline f32_4x
AbsoluteValue(f32_4x A)
{
    u32 MaskU32 = (u32)(1 << 31);
    __m128 Mask = _mm_set1_ps(*(float *)&MaskU32);
    
    f32_4x Result;
    Result.P = _mm_andnot_ps(Mask, A.P);
    
    return(Result);
}

inline f32_4x
SignBitFrom(f32_4x A)
{
    u32 MaskU32 = (u32)(1 << 31);
    __m128 Mask = _mm_set1_ps(*(float *)&MaskU32);
    
    f32_4x Result;
    Result.P = _mm_and_ps(A.P, Mask);
    
    return(Result);
}

inline f32_4x
SignOf(f32_4x Value)
{
    f32_4x Result = F32_4x(1.0f) | SignBitFrom(Value);
    
    return(Result);
}

inline f32_4x
InverseSignBitFrom(f32_4x A)
{
    u32 MaskU32 = (u32)(1 << 31);
    __m128 Mask = _mm_set1_ps(*(float *)&MaskU32);
    
    f32_4x Result;
    Result.P = _mm_xor_ps(_mm_and_ps(A.P, Mask), Mask);
    
    return(Result);
}

inline f32_4x
Min(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_min_ps(A.P, B.P);
    
    return(Result);
}

inline f32_4x
Max(f32_4x A, f32_4x B)
{
    f32_4x Result;
    
    Result.P = _mm_max_ps(A.P, B.P);
    
    return(Result);
}

internal f32_4x
Floor(f32_4x A)
{
    f32_4x Result;
    
    Result.P = _mm_floor_ps(A.P);
    
    return(Result);
}

inline b32x
AnyTrue(f32_4x Comparison)
{
    b32x Result = _mm_movemask_ps(Comparison.P);
    return(Result);
}

inline b32x
AllTrue(f32_4x Comparison)
{
    b32x Result = (_mm_movemask_ps(Comparison.P) == 15);
    return(Result);
}

inline b32x
AllFalse(f32_4x Comparison)
{
    b32x Result = (_mm_movemask_ps(Comparison.P) == 0);
    return(Result);
}

inline f32_4x
AndNot(f32_4x A, f32_4x B) // NOTE(casey): _B_ gets notted
{
    f32_4x Result;
    
    Result.P = _mm_andnot_ps(B.P, A.P);
    
    return(Result);
}

inline f32_4x
Select(f32_4x A, f32_4x Mask, f32_4x B)
{
    f32_4x Result;
    
    // TODO(casey): Use blend now that we are SSE 4.
    Result.P = _mm_or_ps(_mm_andnot_ps(Mask.P, A.P), _mm_and_ps(Mask.P, B.P));
    
    return(Result);
}

inline f32_4x
ApproxInvSquareRoot(f32_4x A)
{
    f32_4x Result;
    
    Result.P = _mm_rsqrt_ps(A.P);
    
    return(Result);
}

inline f32_4x
SquareRoot(f32_4x A)
{
    f32_4x Result;
    
    Result.P = _mm_sqrt_ps(A.P);
    
    return(Result);
}

inline f32_4x
ApproxOneOver(f32_4x A)
{
    f32_4x Result;
    
    Result.P = _mm_rcp_ps(A.P);
    
    return(Result);
}

internal f32_4x
Clamp(f32_4x MinV, f32_4x A, f32_4x MaxV)
{
    f32_4x Result;
    
    Result = Min(Max(A, MinV), MaxV);
    
    return(Result);
}

internal f32_4x
Clamp01(f32_4x A)
{
    f32_4x Result;
    
    f32_4x One = F32_4x(1);
    Result = Clamp(ZeroF32_4x(), A, One);
    
    return(Result);
}

//
// NOTE(casey): v3_4x
//


inline v3_4x
operator*(f32 As, v3_4x B)
{
    v3_4x Result;
    
    f32_4x A = F32_4x(As);
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    
    return(Result);
}

inline v3_4x
operator/(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A.x / B.x;
    Result.y = A.y / B.y;
    Result.z = A.z / B.z;
    
    return(Result);
}

inline v3_4x
Hadamard(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A.x * B.x;
    Result.y = A.y * B.y;
    Result.z = A.z * B.z;
    
    return(Result);
}

inline v3_4x
operator+(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    
    return(Result);
}

inline v3_4x
operator-(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    
    return(Result);
}

inline v3_4x
operator*(f32_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A * B.x;
    Result.y = A * B.y;
    Result.z = A * B.z;
    
    return(Result);
};

inline v3_4x
ZeroV34x(void)
{
    v3_4x Result = {};
    return(Result);
}

inline v3_4x
operator-(v3_4x A)
{
    v3_4x Result = ZeroV34x() - A;
    return(Result);
}

inline v3_4x
operator|(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A.x | B.x;
    Result.y = A.y | B.y;
    Result.z = A.z | B.z;
    
    return(Result);
}

inline v3_4x
operator&(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = A.x & B.x;
    Result.y = A.y & B.y;
    Result.z = A.z & B.z;
    
    return(Result);
}

inline v3_4x &
operator+=(v3_4x &A, v3_4x B)
{
    A.x = A.x + B.x;
    A.y = A.y + B.y;
    A.z = A.z + B.z;
    
    return(A);
}


inline v4_4x
operator*(f32 As, v4_4x B)
{
    v4_4x Result;
    
    f32_4x A = F32_4x(As);
    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;
    
    return(Result);
}

inline v4_4x
operator+(v4_4x A, v4_4x B)
{
    v4_4x Result;
    
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    
    return(Result);
}

inline v4_4x &
operator+=(v4_4x &A, v4_4x B)
{
    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    A.w += B.w;
    
    return(A);
}

inline v3_4x
operator<=(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = (A.x <= B.x);
    Result.y = (A.y <= B.y);
    Result.z = (A.z <= B.z);
    
    return(Result);
}

inline v3_4x
operator<(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = (A.x < B.x);
    Result.y = (A.y < B.y);
    Result.z = (A.z < B.z);
    
    return(Result);
}

inline v3_4x
AbsoluteValue(v3_4x A)
{
    v3_4x Result;
    
    Result.x = AbsoluteValue(A.x);
    Result.y = AbsoluteValue(A.y);
    Result.z = AbsoluteValue(A.z);
    
    return(Result);
}

inline v3_4x
Min(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = Min(A.x, B.x);
    Result.y = Min(A.y, B.y);
    Result.z = Min(A.z, B.z);
    
    return(Result);
}

inline v3_4x
Max(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = Max(A.x, B.x);
    Result.y = Max(A.y, B.y);
    Result.z = Max(A.z, B.z);
    
    return(Result);
}

inline b32x
Any3TrueInAtLeastOneLane(v3_4x Comparison)
{
    b32x Result = AnyTrue(Comparison.x | Comparison.y | Comparison.z);
    return(Result);
}

inline b32x
All3TrueInAtLeastOneLane(v3_4x Comparison)
{
    b32x Result = AnyTrue(Comparison.x & Comparison.y & Comparison.z);
    return(Result);
}

inline v3_4x
V3_4x(v3 A)
{
    v3_4x Result;
    
    Result.x = F32_4x(A.x);
    Result.y = F32_4x(A.y);
    Result.z = F32_4x(A.z);
    
    return(Result);
}

inline v3_4x
V3_4x(v3 E0, v3 E1, v3 E2, v3 E3)
{
    v3_4x Result;
    
    Result.x = F32_4x(E0.x, E1.x, E2.x, E3.x);
    Result.y = F32_4x(E0.y, E1.y, E2.y, E3.y);
    Result.z = F32_4x(E0.z, E1.z, E2.z, E3.z);
    
    return(Result);
}

inline v3_4x
V3_4x(f32 E0, f32 E1, f32 E2, f32 E3)
{
    v3_4x Result;
    
    Result.x =
        Result.y =
        Result.z = F32_4x(E0, E1, E2, E3);
    
    return(Result);
}

inline v3
GetComponent(v3_4x Combined, u32 Index)
{
    v3 Result =
    {
        Combined.x.E[Index],
        Combined.y.E[Index],
        Combined.z.E[Index],
    };
    return(Result);
}

inline v3_4x
Select(v3_4x A, f32_4x Mask, v3_4x B)
{
    v3_4x Result;
    
    Result.x = Select(A.x, Mask, B.x);
    Result.y = Select(A.y, Mask, B.y);
    Result.z = Select(A.z, Mask, B.z);
    
    return(Result);
}

inline f32_4x
Inner(v3_4x A, v3_4x B)
{
    f32_4x Result;
    
    Result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return(Result);
}

inline f32_4x
LengthSq(v3_4x A)
{
    f32_4x Result = Inner(A, A);
    
    return(Result);
}

inline v3_4x
ApproxNOZ(v3_4x A)
{
    v3_4x Result = {};
    
    f32_4x LenSq = LengthSq(A);
    v3_4x Norm = (F32_4x(1.0f) * ApproxInvSquareRoot(LenSq))*A;
    f32_4x Mask = (LenSq > F32_4x(Square(0.0001f)));
    
    Result = Select(Result, Mask, Norm);
    
    return(Result);
}

inline v3_4x
ApproxNOUp(v3_4x A)
{
    v3_4x Result = V3_4x(V3(0.0f, 0.0f, 1.0f));
    
    f32_4x LenSq = LengthSq(A);
    v3_4x Norm = (F32_4x(1.0f) * ApproxInvSquareRoot(LenSq))*A;
    f32_4x Mask = (LenSq > F32_4x(Square(0.0001f)));
    
    Result = Select(Result, Mask, Norm);
    
    return(Result);
}

inline v3_4x
ApproxOneOver(v3_4x A)
{
    v3_4x Result;
    
    Result.x = ApproxOneOver(A.x);
    Result.y = ApproxOneOver(A.y);
    Result.z = ApproxOneOver(A.z);
    
    return(Result);
}

// TODO(casey): Some places where we Shuffle4x probably should be using
// a shufd instead of a shufps because it wouldn't destroy the destination.
#define Shuffle4x(A, S0, S1, B, S2, S3) F32_4x(_mm_shuffle_ps(A.P, B.P, (S0 << 0) | (S1 << 2) | (S2 << 4) | (S3 << 6)))

#define Broadcast4x(A, Index) Shuffle4x(A, Index, Index, A, Index, Index)


inline v3_4x
Transpose(v3_4x A)
{
    v3_4x Result;
    
#define VERIFY_SHUFFLE 0
#if VERIFY_SHUFFLE
    A.x = F32_4x(0.0f, 3.0f, 6.0f, 9.0f);
    A.y = F32_4x(1.0f, 4.0f, 7.0f, 10.0f);
    A.z = F32_4x(2.0f, 5.0f, 8.0f, 11.0f);
#endif
    
    f32_4x XY = Shuffle4x(A.x, 0, 2, A.y, 0, 2);
    f32_4x YZ = Shuffle4x(A.y, 1, 3, A.z, 1, 3);
    f32_4x XZ = Shuffle4x(A.x, 1, 3, A.z, 0, 2);
    
    Result.x = Shuffle4x(XY, 0, 2, XZ, 2, 0);
    Result.y = Shuffle4x(YZ, 0, 2, XY, 1, 3);
    Result.z = Shuffle4x(XZ, 3, 1, YZ, 1, 3);
    
#if VERIFY_SHUFFLE
    f32 Order[] =
    {
        Result.x.E[0], Result.x.E[1], Result.x.E[2], Result.x.E[3],
        Result.y.E[0], Result.y.E[1], Result.y.E[2], Result.y.E[3],
        Result.z.E[0], Result.z.E[1], Result.z.E[2], Result.z.E[3],
    };
#endif
    
    return(Result);
}

internal f32_4x
PShufB(f32_4x Value, __m128i ShufflePattern)
{
    f32_4x Result;
    
    Result.P = _mm_castsi128_ps(_mm_shuffle_epi8(_mm_castps_si128(Value.P), ShufflePattern));
    
    return(Result);
}

#define ConvertF32(V, I) _mm_cvtss_f32(Shuffle4x(V, I, I, V, I, I).P)
#define ConvertS32(V, I) _mm_extract_epi32(_mm_cvtps_epi32(V.P), I)
