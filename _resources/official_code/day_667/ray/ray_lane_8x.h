
struct lane_f32
{
    __m256 V;
    
    lane_f32 &operator=(f32 A);
};

struct lane_u32
{
    __m256i V;
    lane_u32 &operator=(u32 A);
};

internal lane_u32
operator<(lane_f32 A, lane_f32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_LT_OQ));
    
    return(Result);
}

internal lane_u32
operator<=(lane_f32 A, lane_f32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_LE_OQ));
    
    return(Result);
}

internal lane_u32
operator>(lane_f32 A, lane_f32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GT_OQ));
    
    return(Result);
}

internal lane_u32
operator>=(lane_f32 A, lane_f32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_GE_OQ));
    
    return(Result);
}

internal lane_u32
operator==(lane_f32 A, lane_f32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_EQ_OQ));
    
    return(Result);
}

internal lane_u32
operator!=(lane_f32 A, lane_f32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_castps_si256(_mm256_cmp_ps(A.V, B.V, _CMP_NEQ_OQ));
    
    return(Result);
}

internal lane_u32
operator!=(lane_u32 A, lane_u32 B)
{
    lane_u32 Result;
    
    // TODO(casey): What's the most efficient way to invert this comparison?
    Result.V = _mm256_xor_si256(_mm256_cmpeq_epi32(A.V, B.V), _mm256_set1_epi32(0xFFFFFFFF));
    
    return(Result);
}

internal lane_u32
operator^(lane_u32 A, lane_u32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_xor_si256(A.V, B.V);
    
    return(Result);
}

internal lane_u32
operator&(lane_u32 A, lane_u32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_and_si256(A.V, B.V);
    
    return(Result);
}

internal lane_f32
operator&(lane_u32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_and_ps(_mm256_castsi256_ps(A.V), B.V);
    
    return(Result);
}

internal lane_u32
AndNot(lane_u32 A, lane_u32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_andnot_si256(A.V, B.V);
    
    return(Result);
}

internal lane_u32
operator|(lane_u32 A, lane_u32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_or_si256(A.V, B.V);
    
    return(Result);
}

internal lane_u32
operator<<(lane_u32 A, u32 Shift)
{
    lane_u32 Result;
    
    Result.V = _mm256_slli_epi32(A.V, Shift);
    
    return(Result);
}

internal lane_u32
operator>>(lane_u32 A, u32 Shift)
{
    lane_u32 Result;
    
    Result.V = _mm256_srli_epi32(A.V, Shift);
    
    return(Result);
}

internal lane_f32
operator+(lane_f32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_add_ps(A.V, B.V);
    
    return(Result);
}

internal lane_u32
operator+(lane_u32 A, lane_u32 B)
{
    lane_u32 Result;
    
    Result.V = _mm256_add_epi32(A.V, B.V);
    
    return(Result);
}

internal lane_f32
operator-(lane_f32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_sub_ps(A.V, B.V);
    
    return(Result);
}

internal lane_f32
operator*(lane_f32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_mul_ps(A.V, B.V);
    
    return(Result);
}

internal lane_f32
operator/(lane_f32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_div_ps(A.V, B.V);
    
    return(Result);
}

internal lane_f32
LaneF32FromU32(lane_u32 A)
{
    lane_f32 Result;
    
    Result.V = _mm256_cvtepi32_ps(A.V);
    
    return(Result);
}

internal lane_u32
LaneU32FromU32(u32 R0, u32 R1, u32 R2, u32 R3, u32 R4, u32 R5, u32 R6, u32 R7)
{
    lane_u32 Result;
    
    Result.V = _mm256_setr_epi32(R0, R1, R2, R3, R4, R5, R6, R7);
    
    return(Result);
}

internal lane_u32
LaneU32FromU32(u32 Repl)
{
    lane_u32 Result;
    
    Result.V = _mm256_set1_epi32(Repl);
    
    return(Result);
}

internal lane_f32
LaneF32FromU32(u32 Repl)
{
    lane_f32 Result;
    
    Result.V = _mm256_set1_ps((f32)Repl);
    
    return(Result);
}

internal lane_f32
LaneF32FromF32(f32 Repl)
{
    lane_f32 Result;
    
    Result.V = _mm256_set1_ps(Repl);
    
    return(Result);
}

internal lane_f32
SquareRoot(lane_f32 A)
{
    lane_f32 Result;
    
    // TODO(casey): We may want to allow rsqrts as well, need to see if we
    // care about the performance!
    Result.V = _mm256_sqrt_ps(A.V);
    
    return(Result);
}

internal void
ConditionalAssign(lane_f32 *Dest, lane_u32 Mask, lane_f32 Source)
{
    __m256 MaskPS = _mm256_castsi256_ps(Mask.V);
    Dest->V = _mm256_or_ps(_mm256_andnot_ps(MaskPS, Dest->V), 
                           _mm256_and_ps(MaskPS, Source.V));
}

inline lane_f32
Min(lane_f32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_min_ps(A.V, B.V);
    
    return(Result);
}

inline lane_f32
Max(lane_f32 A, lane_f32 B)
{
    lane_f32 Result;
    
    Result.V = _mm256_max_ps(A.V, B.V);
    
    return(Result);
}

internal lane_f32
GatherF32_(void *BasePtr, u32 Stride, lane_u32 Indices)
{
    lane_f32 Result;
    u32 *V = (u32 *)&Indices.V;
    
    Result.V = _mm256_setr_ps(*(f32 *)((u8 *)BasePtr + V[0]*Stride), 
                              *(f32 *)((u8 *)BasePtr + V[1]*Stride), 
                              *(f32 *)((u8 *)BasePtr + V[2]*Stride), 
                              *(f32 *)((u8 *)BasePtr + V[3]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[4]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[5]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[6]*Stride),
                              *(f32 *)((u8 *)BasePtr + V[7]*Stride));
    
    return(Result);
}

internal b32x
MaskIsZeroed(lane_u32 A)
{
    int Mask = _mm256_movemask_epi8(A.V);
    return(Mask == 0);
}

internal u64
HorizontalAdd(lane_u32 A)
{
    u32 *V = (u32 *)&A.V;
    u64 Result = (u64)V[0] + (u64)V[1] + (u64)V[2] + (u64)V[3] + (u64)V[4] + (u64)V[5] + (u64)V[6] + (u64)V[7];
    return(Result);
}

internal f32
HorizontalAdd(lane_f32 A)
{
    f32 *V = (f32 *)&A.V;
    f32 Result = V[0] + V[1] + V[2] + V[3] + V[4] + V[5] + V[6] + V[7];
    return(Result);
}
