union v2
{
    struct
    {
        f32 x, y;
    };
    struct
    {
        f32 u, v;
    };
    f32 E[2];
};

inline f32 
SquareRoot(f32 A)
{
    // TODO(casey): Replace with SSE
    f32 Result = (f32)sqrt(A);
    return(Result);
}

inline f32
Pow(f32 A, f32 B)
{
    f32 Result = (f32)pow(A, B);
    return(Result);
}

inline u32
RoundReal32ToUInt32(f32 F)
{
    // TODO(casey): Replace with SSE
    u32 Result = (u32)(F + 0.5f);
    return(Result);
}

inline v2
V2i(s32 X, s32 Y)
{
    v2 Result = {(f32)X, (f32)Y};

    return(Result);
}

inline v2
V2i(u32 X, u32 Y)
{
    v2 Result = {(f32)X, (f32)Y};

    return(Result);
}

inline v2
V2(f32 X, f32 Y)
{
    v2 Result;

    Result.x = X;
    Result.y = Y;

    return(Result);
}

inline lane_v3
V3(f32 X, f32 Y, f32 Z)
{
    lane_v3 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;

    return(Result);
}

inline lane_v3
V3(v2 XY, f32 Z)
{
    lane_v3 Result;

    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;

    return(Result);
}

//
// NOTE(casey): Scalar operations
//

inline f32
Square(f32 A)
{
    f32 Result = A*A;

    return(Result);
}

inline f32
Triangle01(f32 t)
{
    f32 Result = 2.0f*t;
    if(Result > 1.0f)
    {
        Result = 2.0f - Result;
    }
    
    return(Result);
}

inline f32
Lerp(f32 A, f32 t, f32 B)
{
    f32 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline s32
Clamp(s32 Min, s32 Value, s32 Max)
{
    s32 Result = Value;
    
    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }
    
    return(Result);
}

inline f32
Clamp(f32 Min, f32 Value, f32 Max)
{
    f32 Result = Value;

    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline f32
Clamp01(f32 Value)
{
    f32 Result = Clamp(0.0f, Value, 1.0f);

    return(Result);
}

inline f32
Clamp01MapToRange(f32 Min, f32 t, f32 Max)
{
    f32 Result = 0.0f;
    
    f32 Range = Max - Min;
    if(Range != 0.0f)
    {
        Result = Clamp01((t - Min) / Range);
    }

    return(Result);
}

inline f32
ClampAboveZero(f32 Value)
{
    f32 Result = (Value < 0) ? 0.0f : Value;
    return(Result);
}

inline f32
SafeRatioN(f32 Numerator, f32 Divisor, f32 N)
{
    f32 Result = N;

    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline f32
SafeRatio0(f32 Numerator, f32 Divisor)
{
    f32 Result = SafeRatioN(Numerator, Divisor, 0.0f);

    return(Result);
}

inline f32
SafeRatio1(f32 Numerator, f32 Divisor)
{
    f32 Result = SafeRatioN(Numerator, Divisor, 1.0f);

    return(Result);
}

//
// NOTE(casey): v2 operations
//

inline v2
Perp(v2 A)
{
    v2 Result = {-A.y, A.x};
    return(Result);
}

inline v2
operator*(f32 A, v2 B)
{
    v2 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    
    return(Result);
}

inline v2
operator*(v2 B, f32 A)
{
    v2 Result = A*B;

    return(Result);
}

inline v2 &
operator*=(v2 &B, f32 A)
{
    B = A * B;

    return(B);
}

inline v2
operator-(v2 A)
{
    v2 Result;

    Result.x = -A.x;
    Result.y = -A.y;

    return(Result);
}

inline v2
operator+(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return(Result);
}

inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;

    return(A);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

    return(Result);
}

inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;

    return(A);
}

inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

inline f32
Inner(v2 A, v2 B)
{
    f32 Result = A.x*B.x + A.y*B.y;

    return(Result);
}

inline f32
LengthSq(v2 A)
{
    f32 Result = Inner(A, A);

    return(Result);
}

inline f32
Length(v2 A)
{
    f32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);

    return(Result);
}

//
// NOTE(casey): lane_v3 operations
//

inline lane_v3
operator*(f32 A, lane_v3 B)
{
    lane_v3 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    
    return(Result);
}

inline lane_v3
operator*(lane_v3 B, f32 A)
{
    lane_v3 Result = A*B;

    return(Result);
}

inline lane_v3 &
operator*=(lane_v3 &B, f32 A)
{
    B = A * B;

    return(B);
}

inline lane_v3
operator/(lane_v3 B, f32 A)
{
    lane_v3 Result = (1.0f/A)*B;
    
    return(Result);
}

inline lane_v3 &
operator/=(lane_v3 &B, f32 A)
{
    B = B / A;
    
    return(B);
}

inline lane_v3
operator-(lane_v3 A)
{
    lane_v3 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;

    return(Result);
}

inline lane_v3
operator+(lane_v3 A, lane_v3 B)
{
    lane_v3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return(Result);
}

inline lane_v3 &
operator+=(lane_v3 &A, lane_v3 B)
{
    A = A + B;

    return(A);
}

inline lane_v3
operator-(lane_v3 A, lane_v3 B)
{
    lane_v3 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return(Result);
}

inline lane_v3 &
operator-=(lane_v3 &A, lane_v3 B)
{
    A = A - B;

    return(A);
}

inline lane_v3
Hadamard(lane_v3 A, lane_v3 B)
{
    lane_v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

inline lane_f32
Inner(lane_v3 A, lane_v3 B)
{
    lane_f32 Result = A.x*B.x + A.y*B.y + A.z*B.z;
    return(Result);
}

inline lane_v3
Cross(lane_v3 A, lane_v3 B)
{
    lane_v3 Result;
    
    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;
    
    return(Result);
}

inline lane_f32
LengthSq(lane_v3 A)
{
    lane_f32 Result = Inner(A, A);

    return(Result);
}

inline lane_f32
Length(lane_v3 A)
{
    lane_f32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline lane_v3
Normalize(lane_v3 A)
{
    lane_v3 Result = A * (1.0f / Length(A));

    return(Result);
}

inline lane_v3
NOZ(lane_v3 A)
{
    lane_v3 Result = {};
    
    lane_f32 LenSq = LengthSq(A);
    lane_u32 Mask = (LenSq > Square(0.0001f));
    ConditionalAssign(&Result, Mask, A * (1.0f / SquareRoot(LenSq)));
                      
    return(Result);
}

inline lane_v3
Clamp01(lane_v3 Value)
{
    lane_v3 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);

    return(Result);
}

inline lane_v3
Lerp(lane_v3 A, f32 t, lane_v3 B)
{
    lane_v3 Result = (1.0f - t)*A + t*B;

    return(Result);
}

internal v2
RayIntersect2(v2 Pa, v2 ra, v2 Pb, v2 rb)
{
    v2 Result = {};
    
    /* NOTE(casey):
    
       Pa.x + ta*ra.x = Pb.x + tb*rb.x
       Pa.y + ta*ra.y = Pb.y + tb*rb.y
    */
    
    f32 d = (rb.x*ra.y - rb.y*ra.x);
    if(d != 0.0f)
    {
        f32 ta = ((Pa.x - Pb.x)*rb.y + (Pb.y - Pa.y)*rb.x) / d;
        f32 tb = ((Pa.x - Pb.x)*ra.y + (Pb.y - Pa.y)*ra.x) / d;
        
        Result = V2(ta, tb);
    }
    
    return(Result);
}

inline b32x
IsInRange(f32 Min, f32 Value, f32 Max)
{
    b32x Result = ((Min <= Value) &&
                   (Value <= Max));
    
    return(Result);
}

inline lane_v3
LaneV3(lane_f32 X, lane_f32 Y, lane_f32 Z)
{
    lane_v3 Result;
    
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    
    return(Result);
}

// TODO(casey): Clean up our header order, etc.
#if (LANE_WIDTH != 1)
inline lane_v3
Lerp(lane_v3 A, lane_f32 t, lane_v3 B)
{
    lane_v3 Result = (1.0f - t)*A + t*B;
    
    return(Result);
}
#endif
