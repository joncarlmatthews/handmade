/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define IGNORED_TIMED_FUNCTION(...)
#define IGNORED_TIMED_BLOCK(...)

#if 0
#include <iacaMarks.h>
#else
#define IACA_VC64_START
#define IACA_VC64_END
#endif

internal void
DrawRectangle(software_texture *Buffer, v2 vMin, v2 vMax, v4 Color, rectangle2i ClipRect)
{
    IGNORED_TIMED_FUNCTION();
    
    rectangle2i FillRect;
    FillRect.MinX = RoundReal32ToInt32(vMin.x);
    FillRect.MinY = RoundReal32ToInt32(vMin.y);
    FillRect.MaxX = RoundReal32ToInt32(vMax.x);
    FillRect.MaxY = RoundReal32ToInt32(vMax.y);

    FillRect = Intersect(ClipRect, FillRect);

#if 0
    uint32 Color32 = ((RoundReal32ToUInt32(A * 255.0f) << 24) |
                      (RoundReal32ToUInt32(R * 255.0f) << 16) |
                      (RoundReal32ToUInt32(G * 255.0f) << 8) |
                      (RoundReal32ToUInt32(B * 255.0f) << 0));

    uint8 *Row = ((uint8 *)Buffer->Memory +
                  FillRect.MinX*BITMAP_BYTES_PER_PIXEL +
                  FillRect.MinY*Buffer->Pitch);
    for(int Y = FillRect.MinY;
        Y < FillRect.MaxY;
        ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = FillRect.MinX;
            X < FillRect.MaxX;
            ++X)
        {
            *Pixel++ = Color32;
        }
        
        Row += Buffer->Pitch;
    }
#else
    Color *= 255.0f;
    Color.rgb *= 255.0f;
    
    if(HasArea(FillRect))
    {
        __m128i StartClipMask = _mm_set1_epi8(-1);
        __m128i EndClipMask = _mm_set1_epi8(-1);

        __m128i StartClipMasks[] =
        {
            _mm_slli_si128(StartClipMask, 0*4),
            _mm_slli_si128(StartClipMask, 1*4),
            _mm_slli_si128(StartClipMask, 2*4),
            _mm_slli_si128(StartClipMask, 3*4),
        };

        __m128i EndClipMasks[] =
        {
            _mm_srli_si128(EndClipMask, 0*4),
            _mm_srli_si128(EndClipMask, 3*4),
            _mm_srli_si128(EndClipMask, 2*4),
            _mm_srli_si128(EndClipMask, 1*4),
        };
        
        if(FillRect.MinX & 3)
        {
            StartClipMask = StartClipMasks[FillRect.MinX & 3];
            FillRect.MinX = FillRect.MinX & ~3;
        }

        if(FillRect.MaxX & 3)
        {
            EndClipMask = EndClipMasks[FillRect.MaxX & 3];
            FillRect.MaxX = (FillRect.MaxX & ~3) + 4;
        }
        
        if((FillRect.MaxX - FillRect.MinX) == 4)
        {
            StartClipMask = _mm_and_si128(StartClipMask, EndClipMask);
        }
        
        real32 Inv255 = 1.0f / 255.0f;
        __m128 Inv255_4x = _mm_set1_ps(Inv255);
        real32 One255 = 255.0f;

        __m128 One = _mm_set1_ps(1.0f);
        __m128 Half = _mm_set1_ps(0.5f);
        __m128 Four_4x = _mm_set1_ps(4.0f);
        __m128 One255_4x = _mm_set1_ps(255.0f);
        __m128 Zero = _mm_set1_ps(0.0f);
        __m128i MaskFF = _mm_set1_epi32(0xFF);
        __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
        __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
        __m128 Colorr_4x = _mm_set1_ps(Color.r);
        __m128 Colorg_4x = _mm_set1_ps(Color.g);
        __m128 Colorb_4x = _mm_set1_ps(Color.b);
        __m128 Colora_4x = _mm_set1_ps(Color.a);
        __m128 MaxColorValue = _mm_set1_ps(255.0f*255.0f);
        
        uint8 *Row = ((uint8 *)Buffer->Memory +
                      FillRect.MinX*BITMAP_BYTES_PER_PIXEL +
                      FillRect.MinY*Buffer->Pitch);
        int32 RowAdvance = Buffer->Pitch;
        
        int MinY = FillRect.MinY;
        int MaxY = FillRect.MaxY;
        int MinX = FillRect.MinX;
        int MaxX = FillRect.MaxX;
        
        IGNORED_TIMED_BLOCK("Pixel Fill", GetClampedRectArea(FillRect) / 2);
        for(int Y = MinY;
            Y < MaxY;
            ++Y)
        {
            __m128i ClipMask = StartClipMask;

            uint32 *Pixel = (uint32 *)Row;
            for(int XI = MinX;
                XI < MaxX;
                XI += 4)
            {
                
                IACA_VC64_START;
                __m128i WriteMask = ClipMask;
            
// TODO(casey): Later, re-check if this helps
//            if(_mm_movemask_epi8(WriteMask))
                {
                    __m128i OriginalDest = _mm_load_si128((__m128i *)Pixel);
                    
                    // NOTE(casey): Load destination
                    __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest, MaskFF));
                    __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), MaskFF));
                    __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), MaskFF));
                    __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));
                    
                    // NOTE(casey): Modulate by incoming color
                    __m128 Texelr = Colorr_4x;
                    __m128 Texelg = Colorg_4x;
                    __m128 Texelb = Colorb_4x;
                    __m128 Texela = Colora_4x;

                    Texelr = _mm_min_ps(_mm_max_ps(Texelr, Zero), MaxColorValue);
                    Texelg = _mm_min_ps(_mm_max_ps(Texelg, Zero), MaxColorValue);
                    Texelb = _mm_min_ps(_mm_max_ps(Texelb, Zero), MaxColorValue);
                    
                    // NOTE(casey): Go from sRGB to "linear" brightness space
                    Destr = mmSquare(Destr);
                    Destg = mmSquare(Destg);
                    Destb = mmSquare(Destb);

                    // NOTE(casey): Destination blend
                    __m128 InvTexelA = _mm_sub_ps(One, _mm_mul_ps(Inv255_4x, Texela));
                    __m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvTexelA, Destr), Texelr);
                    __m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvTexelA, Destg), Texelg);
                    __m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvTexelA, Destb), Texelb);
                    __m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvTexelA, Desta), Texela);
        
                    // NOTE(casey): Go from "linear" 0-1 brightness space to sRGB 0-255
#if 1
                    Blendedr = _mm_mul_ps(Blendedr, _mm_rsqrt_ps(Blendedr));
                    Blendedg = _mm_mul_ps(Blendedg, _mm_rsqrt_ps(Blendedg));
                    Blendedb = _mm_mul_ps(Blendedb, _mm_rsqrt_ps(Blendedb));
#else
                    Blendedr = _mm_sqrt_ps(Blendedr);
                    Blendedg = _mm_sqrt_ps(Blendedg);
                    Blendedb = _mm_sqrt_ps(Blendedb);
#endif
                    Blendeda = Blendeda;
            
                    __m128i Intr = _mm_cvtps_epi32(Blendedr);
                    __m128i Intg = _mm_cvtps_epi32(Blendedg);
                    __m128i Intb = _mm_cvtps_epi32(Blendedb);
                    __m128i Inta = _mm_cvtps_epi32(Blendeda);

                    __m128i Sr = _mm_slli_epi32(Intr, 16);
                    __m128i Sg = _mm_slli_epi32(Intg, 8);
                    __m128i Sb = Intb;
                    __m128i Sa = _mm_slli_epi32(Inta, 24);

                    __m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));

                    __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out),
                                                     _mm_andnot_si128(WriteMask, OriginalDest));
                    _mm_store_si128((__m128i *)Pixel, MaskedOut);
                }
            
                Pixel += 4;

                if((XI + 8) < MaxX)
                {
                    ClipMask = _mm_set1_epi8(-1);
                }
                else
                {
                    ClipMask = EndClipMask;
                }

                IACA_VC64_END;
            }
        
            Row += RowAdvance;
        }
    }
#endif
}

struct bilinear_sample
{
    uint32 A, B, C, D;
};
inline bilinear_sample
BilinearSample(software_texture *Texture, int32 X, int32 Y)
{
    bilinear_sample Result;
    
    uint8 *TexelPtr = ((uint8 *)Texture->Memory) + Y*Texture->Pitch + X*sizeof(uint32);
    Result.A = *(uint32 *)(TexelPtr);
    Result.B = *(uint32 *)(TexelPtr + sizeof(uint32));
    Result.C = *(uint32 *)(TexelPtr + Texture->Pitch);
    Result.D = *(uint32 *)(TexelPtr + Texture->Pitch + sizeof(uint32));

    return(Result);
}

inline v4
SRGBBilinearBlend(bilinear_sample TexelSample, real32 fX, real32 fY)
{
    v4 TexelA = BGRAUnpack4x8(TexelSample.A);
    v4 TexelB = BGRAUnpack4x8(TexelSample.B);
    v4 TexelC = BGRAUnpack4x8(TexelSample.C);
    v4 TexelD = BGRAUnpack4x8(TexelSample.D);

    // NOTE(casey): Go from sRGB to "linear" brightness space
    TexelA = SRGB255ToLinear1(TexelA);
    TexelB = SRGB255ToLinear1(TexelB);
    TexelC = SRGB255ToLinear1(TexelC);
    TexelD = SRGB255ToLinear1(TexelD);

    v4 Result = Lerp(Lerp(TexelA, fX, TexelB),
                     fY,
                     Lerp(TexelC, fX, TexelD));

    return(Result);
}

#if 0
inline v3
SampleEnvironmentMap(v2 ScreenSpaceUV, v3 SampleDirection, real32 Roughness, environment_map *Map,
                     real32 DistanceFromMapInZ)
{
    /* NOTE(casey):

       ScreenSpaceUV tells us where the ray is being cast _from_ in
       normalized screen coordinates.

       SampleDirection tells us what direction the cast is going -
       it does not have to be normalized.

       Roughness says which LODs of Map we sample from.

       DistanceFromMapInZ says how far the map is from the sample point in Z, given
       in meters.
    */

    // NOTE(casey): Pick which LOD to sample from
    uint32 LODIndex = (uint32)(Roughness*(real32)(ArrayCount(Map->LOD) - 1) + 0.5f);
    Assert(LODIndex < ArrayCount(Map->LOD));

    software_texture *LOD = &Map->LOD[LODIndex];

    // NOTE(casey): Compute the distance to the map and the scaling
    // factor for meters-to-UVs
    real32 UVsPerMeter = 0.1f; // TODO(casey): Parameterize this, and should be different for X and Y based on map!
    real32 C = (UVsPerMeter*DistanceFromMapInZ) / SampleDirection.y;
    v2 Offset = C * V2(SampleDirection.x, SampleDirection.z);

    // NOTE(casey): Find the intersection point
    v2 UV = ScreenSpaceUV + Offset;

    // NOTE(casey): Clamp to the valid range
    UV.x = Clamp01(UV.x);
    UV.y = Clamp01(UV.y);

    // NOTE(casey): Bilinear sample
    // TODO(casey): Formalize texture boundaries!!!
    real32 tX = ((UV.x*(real32)(LOD->Width - 2)));
    real32 tY = ((UV.y*(real32)(LOD->Height - 2)));
    
    int32 X = (int32)tX;
    int32 Y = (int32)tY;

    real32 fX = tX - (real32)X;
    real32 fY = tY - (real32)Y;

    Assert((X >= 0) && (X < LOD->Width));
    Assert((Y >= 0) && (Y < LOD->Height));

    if(Global_Renderer_ShowLightingSamples)
    {
        // NOTE(casey): Turn this on to see where in the map you're sampling!
        uint8 *TexelPtr = ((uint8 *)LOD->Memory) + Y*LOD->Pitch + X*sizeof(uint32);
        *(uint32 *)TexelPtr = 0xFFFFFFFF;
    }
    
    bilinear_sample Sample = BilinearSample(LOD, X, Y);
    v3 Result = SRGBBilinearBlend(Sample, fX, fY).xyz;

    return(Result);
}

internal void
DrawRectangleSlowly(software_texture *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                    software_texture *Texture, software_texture *NormalMap,
                    environment_map *Top,
                    environment_map *Middle,
                    environment_map *Bottom,
                    real32 PixelsToMeters)
{
    IGNORED_TIMED_FUNCTION();
    
    real32 XAxisLength = Length(XAxis);
    real32 YAxisLength = Length(YAxis);
    
    v2 NxAxis = (YAxisLength / XAxisLength) * XAxis;
    v2 NyAxis = (XAxisLength / YAxisLength) * YAxis;

    // NOTE(casey): NzScale could be a parameter if we want people to
    // have control over the amount of scaling in the Z direction
    // that the normals appear to have.
    real32 NzScale = 0.5f*(XAxisLength + YAxisLength);
    
    real32 InvXAxisLengthSq = 1.0f / LengthSq(XAxis);
    real32 InvYAxisLengthSq = 1.0f / LengthSq(YAxis);

    uint32 Color32 = ((RoundReal32ToUInt32(Color.a * 255.0f) << 24) |
                      (RoundReal32ToUInt32(Color.r * 255.0f) << 16) |
                      (RoundReal32ToUInt32(Color.g * 255.0f) << 8) |
                      (RoundReal32ToUInt32(Color.b * 255.0f) << 0));

    int WidthMax = (Buffer->Width - 1);
    int HeightMax = (Buffer->Height - 1);
    
    real32 InvWidthMax = 1.0f / (real32)WidthMax;
    real32 InvHeightMax = 1.0f / (real32)HeightMax;

    // TODO(casey): This will need to be specified separately!!!
    real32 OriginZ = 0.0f;
    real32 OriginY = (Origin + 0.5f*XAxis + 0.5f*YAxis).y;
    real32 FixedCastY = InvHeightMax*OriginY;
    
    int XMin = WidthMax;
    int XMax = 0;
    int YMin = HeightMax;
    int YMax = 0;

    v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};
    for(int PIndex = 0;
        PIndex < ArrayCount(P);
        ++PIndex)
    {
        v2 TestP = P[PIndex];
        int FloorX = FloorReal32ToInt32(TestP.x);
        int CeilX = CeilReal32ToInt32(TestP.x);
        int FloorY = FloorReal32ToInt32(TestP.y);
        int CeilY = CeilReal32ToInt32(TestP.y);

        if(XMin > FloorX) {XMin = FloorX;}
        if(YMin > FloorY) {YMin = FloorY;}
        if(XMax < CeilX) {XMax = CeilX;}
        if(YMax < CeilY) {YMax = CeilY;}
    }

    if(XMin < 0) {XMin = 0;}
    if(YMin < 0) {YMin = 0;}
    if(XMax > WidthMax) {XMax = WidthMax;}
    if(YMax > HeightMax) {YMax = HeightMax;}
    
    uint8 *Row = ((uint8 *)Buffer->Memory +
                  XMin*BITMAP_BYTES_PER_PIXEL +
                  YMin*Buffer->Pitch);

    IGNORED_TIMED_BLOCK("Pixel Fill", (XMax - XMin + 1)*(YMax - YMin + 1));
    for(int StepY = YMin;
        StepY <= YMax;
        ++StepY)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int StepX = XMin;
            StepX <= XMax;
            ++StepX)
        {
#if 1
            v2 PixelP = V2i(StepX, StepY);
            v2 d = PixelP - Origin;
            
            // TODO(casey): PerpInner
            // TODO(casey): Simpler origin
            real32 Edge0 = Inner(d, -Perp(XAxis));
            real32 Edge1 = Inner(d - XAxis, -Perp(YAxis));
            real32 Edge2 = Inner(d - XAxis - YAxis, Perp(XAxis));
            real32 Edge3 = Inner(d - YAxis, Perp(YAxis));
            
            if((Edge0 < 0) &&
               (Edge1 < 0) &&
               (Edge2 < 0) &&
               (Edge3 < 0))
            {
#if 1
                v2 ScreenSpaceUV = {InvWidthMax*(real32)StepX, FixedCastY};
                real32 ZDiff = PixelsToMeters*((real32)StepY - OriginY);
#else
                v2 ScreenSpaceUV = {InvWidthMax*(real32)StepX, InvHeightMax*(real32)StepY};
                real32 ZDiff = 0.0f;
#endif

                
                real32 U = InvXAxisLengthSq*Inner(d, XAxis);
                real32 V = InvYAxisLengthSq*Inner(d, YAxis);

#if 0
                // TODO(casey): SSE clamping.
                Assert((U >= 0.0f) && (U <= 1.0f));
                Assert((V >= 0.0f) && (V <= 1.0f));
#endif
                
                // TODO(casey): Formalize texture boundaries!!!
                real32 tX = ((U*(real32)(Texture->Width - 2)));
                real32 tY = ((V*(real32)(Texture->Height - 2)));
                
                int32 tXi = (int32)tX;
                int32 tYi = (int32)tY;

                real32 fX = tX - (real32)tXi;
                real32 fY = tY - (real32)tYi;

                Assert((tXi >= 0) && (tXi < Texture->Width));
                Assert((tYi >= 0) && (tYi < Texture->Height));

                bilinear_sample TexelSample = BilinearSample(Texture, tXi, tYi);
                v4 Texel = SRGBBilinearBlend(TexelSample, fX, fY);

#if 0
                if(NormalMap)
                {
                    bilinear_sample NormalSample = BilinearSample(NormalMap, X, Y);

                    v4 NormalA = Unpack4x8(NormalSample.A);
                    v4 NormalB = Unpack4x8(NormalSample.B);
                    v4 NormalC = Unpack4x8(NormalSample.C);
                    v4 NormalD = Unpack4x8(NormalSample.D);

                    v4 Normal = Lerp(Lerp(NormalA, fX, NormalB),
                                     fY,
                                     Lerp(NormalC, fX, NormalD));

                    Normal = UnscaleAndBiasNormal(Normal);
                    // TODO(casey): Do we really need to do this?

                    Normal.xy = Normal.x*NxAxis + Normal.y*NyAxis;
                    Normal.z *= NzScale;
                    Normal.xyz = Normalize(Normal.xyz);
                    
                    // NOTE(casey): The eye vector is always assumed to be [0, 0, 1]
                    // This is just the simplified version of the reflection -e + 2e^T N N
                    v3 BounceDirection = 2.0f*Normal.z*Normal.xyz;
                    BounceDirection.z -= 1.0f;

                    // TODO(casey): Eventually we need to support two mappings,
                    // one for top-down view (which we don't do now) and one
                    // for sideways, which is what's happening here.
                    BounceDirection.z = -BounceDirection.z;

                    environment_map *FarMap = 0;
                    real32 Pz = OriginZ + ZDiff;
                    real32 MapZ = 2.0f;
                    real32 tEnvMap = BounceDirection.y;
                    real32 tFarMap = 0.0f;
                    if(tEnvMap < -0.5f)
                    {
                        // TODO(casey): This path seems PARTICULARLY broken!
                        FarMap = Bottom;
                        tFarMap = -1.0f - 2.0f*tEnvMap;
                    }
                    else if(tEnvMap > 0.5f)
                    {
                        FarMap = Top;
                        tFarMap = 2.0f*(tEnvMap - 0.5f);
                    }

                    tFarMap *= tFarMap;
                    tFarMap *= tFarMap;

                    v3 LightColor = {0, 0, 0}; // TODO(casey): How do we sample from the middle map???
                    if(FarMap)
                    {
                        real32 DistanceFromMapInZ = FarMap->Pz - Pz;
                        v3 FarMapColor = SampleEnvironmentMap(ScreenSpaceUV, BounceDirection, Normal.w, FarMap,
                                                              DistanceFromMapInZ);
                        LightColor = Lerp(LightColor, tFarMap, FarMapColor);
                    }
                    
                    // TODO(casey): ? Actually do a lighting model computation here
                    Texel.rgb = Texel.rgb + Texel.a*LightColor;

#if 0
                    // NOTE(casey): Draws the bounce direction
                    Texel.rgb = V3(0.5f, 0.5f, 0.5f) + 0.5f*BounceDirection;
                    Texel.rgb *= Texel.a;
#endif
                }
#endif
                
                Texel = Hadamard(Texel, Color);
                Texel.r = Clamp01(Texel.r);
                Texel.g = Clamp01(Texel.g);
                Texel.b = Clamp01(Texel.b);
                
                v4 Dest = {(real32)((*Pixel >> 16) & 0xFF),
                           (real32)((*Pixel >> 8) & 0xFF),
                           (real32)((*Pixel >> 0) & 0xFF),
                           (real32)((*Pixel >> 24) & 0xFF)};
                
                // NOTE(casey): Go from sRGB to "linear" brightness space
                Dest = SRGB255ToLinear1(Dest);
                
                v4 Blended = (1.0f-Texel.a)*Dest + Texel;

                // NOTE(casey): Go from "linear" brightness space to sRGB
                v4 Blended255 = Linear1ToSRGB255(Blended);

                *Pixel = (((uint32)(Blended255.a + 0.5f) << 24) |
                          ((uint32)(Blended255.r + 0.5f) << 16) |
                          ((uint32)(Blended255.g + 0.5f) << 8) |
                          ((uint32)(Blended255.b + 0.5f) << 0));
            }
#else
            *Pixel = Color32;
#endif
            
            ++Pixel;
        }
        
        Row += Buffer->Pitch;
    }
}
#endif

internal void
DrawBitmap(software_texture *Buffer, software_texture *Bitmap,
           real32 RealX, real32 RealY, real32 CAlpha = 1.0f)
{
    IGNORED_TIMED_FUNCTION();

    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;

    int32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }

    int32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }

    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint8 *SourceRow = (uint8 *)Bitmap->Memory + SourceOffsetY*Bitmap->Pitch + BITMAP_BYTES_PER_PIXEL*SourceOffsetX;
    uint8 *DestRow = ((uint8 *)Buffer->Memory +
                      MinX*BITMAP_BYTES_PER_PIXEL +
                      MinY*Buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        uint32 *Dest = (uint32 *)DestRow;
        uint32 *Source = (uint32 *)SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            v4 Texel = {(real32)((*Source >> 16) & 0xFF),
                        (real32)((*Source >> 8) & 0xFF),
                        (real32)((*Source >> 0) & 0xFF),
                        (real32)((*Source >> 24) & 0xFF)};

            Texel = SRGB255ToLinear1(Texel);

            Texel *= CAlpha;

            v4 D = {(real32)((*Dest >> 16) & 0xFF),
                    (real32)((*Dest >> 8) & 0xFF),
                    (real32)((*Dest >> 0) & 0xFF),
                    (real32)((*Dest >> 24) & 0xFF)};

            D = SRGB255ToLinear1(D);
            
            v4 Result = (1.0f-Texel.a)*D + Texel;

            Result = Linear1ToSRGB255(Result);

            *Dest = (((uint32)(Result.a + 0.5f) << 24) |
                     ((uint32)(Result.r + 0.5f) << 16) |
                     ((uint32)(Result.g + 0.5f) << 8) |
                     ((uint32)(Result.b + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }

        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Pitch;
    }
}

internal void
ChangeSaturation(software_texture *Buffer, real32 Level)
{
    IGNORED_TIMED_FUNCTION();

    uint8 *DestRow = (uint8 *)Buffer->Memory;
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        uint32 *Dest = (uint32 *)DestRow;
        for(int X = 0;
            X < Buffer->Width;
            ++X)
        {
            v4 D = {(real32)((*Dest >> 16) & 0xFF),
                    (real32)((*Dest >> 8) & 0xFF),
                    (real32)((*Dest >> 0) & 0xFF),
                    (real32)((*Dest >> 24) & 0xFF)};

            D = SRGB255ToLinear1(D);

            real32 Avg = (1.0f / 3.0f) * (D.r + D.g + D.b);
            v3 Delta = V3(D.r - Avg, D.g - Avg, D.b - Avg);
            
            v4 Result = V4(V3(Avg, Avg, Avg) + Level*Delta, D.a);

            Result = Linear1ToSRGB255(Result);

            *Dest = (((uint32)(Result.a + 0.5f) << 24) |
                     ((uint32)(Result.r + 0.5f) << 16) |
                     ((uint32)(Result.g + 0.5f) << 8) |
                     ((uint32)(Result.b + 0.5f) << 0));
            
            ++Dest;
        }

        DestRow += Buffer->Pitch;
    }
}

internal void
DrawMatte(software_texture *Buffer, software_texture *Bitmap,
          real32 RealX, real32 RealY, real32 CAlpha = 1.0f)
{
    IGNORED_TIMED_FUNCTION();

    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;

    int32 SourceOffsetX = 0;
    if(MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }

    int32 SourceOffsetY = 0;
    if(MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }

    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint8 *SourceRow = (uint8 *)Bitmap->Memory + SourceOffsetY*Bitmap->Pitch + BITMAP_BYTES_PER_PIXEL*SourceOffsetX;
    uint8 *DestRow = ((uint8 *)Buffer->Memory +
                      MinX*BITMAP_BYTES_PER_PIXEL +
                      MinY*Buffer->Pitch);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        uint32 *Dest = (uint32 *)DestRow;
        uint32 *Source = (uint32 *)SourceRow;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            real32 SA = (real32)((*Source >> 24) & 0xFF);
            real32 RSA = (SA / 255.0f) * CAlpha;
            real32 SR = CAlpha*(real32)((*Source >> 16) & 0xFF);
            real32 SG = CAlpha*(real32)((*Source >> 8) & 0xFF);
            real32 SB = CAlpha*(real32)((*Source >> 0) & 0xFF);

            real32 DA = (real32)((*Dest >> 24) & 0xFF);
            real32 DR = (real32)((*Dest >> 16) & 0xFF);
            real32 DG = (real32)((*Dest >> 8) & 0xFF);
            real32 DB = (real32)((*Dest >> 0) & 0xFF);
            real32 RDA = (DA / 255.0f);
            
            real32 InvRSA = (1.0f-RSA);
            // TODO(casey): Check this for math errors
//            real32 A = 255.0f*(RSA + RDA - RSA*RDA);
            real32 A = InvRSA*DA;
            real32 R = InvRSA*DR;
            real32 G = InvRSA*DG;
            real32 B = InvRSA*DB;

            *Dest = (((uint32)(A + 0.5f) << 24) |
                     ((uint32)(R + 0.5f) << 16) |
                     ((uint32)(G + 0.5f) << 8) |
                     ((uint32)(B + 0.5f) << 0));
            
            ++Dest;
            ++Source;
        }

        DestRow += Buffer->Pitch;
        SourceRow += Bitmap->Pitch;
    }
}

void
DrawRectangleQuickly(software_texture *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                     software_texture *Texture, real32 PixelsToMeters,
                     rectangle2i ClipRect)
{
    IGNORED_TIMED_FUNCTION();
    
    // TODO(casey): We have a bug in our texel lookup here, where we're not properly
    // computing the texel or fill values (not sure which) when we're skewing!
    
    rectangle2i FillRect = InvertedInfinityRectangle2i();

    v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};
    for(int PIndex = 0;
        PIndex < ArrayCount(P);
        ++PIndex)
    {
        v2 TestP = P[PIndex];
        int FloorX = FloorReal32ToInt32(TestP.x);
        int CeilX = CeilReal32ToInt32(TestP.x) + 1;
        int FloorY = FloorReal32ToInt32(TestP.y);
        int CeilY = CeilReal32ToInt32(TestP.y) + 1;

        if(FillRect.MinX > FloorX) {FillRect.MinX = FloorX;}
        if(FillRect.MinY > FloorY) {FillRect.MinY = FloorY;}
        if(FillRect.MaxX < CeilX) {FillRect.MaxX = CeilX;}
        if(FillRect.MaxY < CeilY) {FillRect.MaxY = CeilY;}
    }

//    rectangle2i ClipRect = {0, 0, WidthMax, HeightMax};
//    rectangle2i ClipRect = {128, 128, 256, 256};
    FillRect = Intersect(ClipRect, FillRect);

    if(HasArea(FillRect))
    {
        __m128i StartClipMask = _mm_set1_epi8(-1);
        __m128i EndClipMask = _mm_set1_epi8(-1);

        __m128i StartClipMasks[] =
        {
            _mm_slli_si128(StartClipMask, 0*4),
            _mm_slli_si128(StartClipMask, 1*4),
            _mm_slli_si128(StartClipMask, 2*4),
            _mm_slli_si128(StartClipMask, 3*4),
        };

        __m128i EndClipMasks[] =
        {
            _mm_srli_si128(EndClipMask, 0*4),
            _mm_srli_si128(EndClipMask, 3*4),
            _mm_srli_si128(EndClipMask, 2*4),
            _mm_srli_si128(EndClipMask, 1*4),
        };
        
        if(FillRect.MinX & 3)
        {
            StartClipMask = StartClipMasks[FillRect.MinX & 3];
            FillRect.MinX = FillRect.MinX & ~3;
        }

        if(FillRect.MaxX & 3)
        {
            EndClipMask = EndClipMasks[FillRect.MaxX & 3];
            FillRect.MaxX = (FillRect.MaxX & ~3) + 4;
        }
            
        // TODO(casey): Just don't draw if the Det is close to 0?
        r32 Det = XAxis.x*YAxis.y - XAxis.y*YAxis.x;
        if(Det == 0.0f)
        {
            Det = 1.0f;
        }
        
        v2 nXAxis = { YAxis.y/Det, -YAxis.x/Det};
        v2 nYAxis = {-XAxis.y/Det,  XAxis.x/Det};

        real32 Inv255 = 1.0f / 255.0f;
        __m128 Inv255_4x = _mm_set1_ps(Inv255);
        real32 One255 = 255.0f;

        __m128 One = _mm_set1_ps(1.0f);
        __m128 Half = _mm_set1_ps(0.5f);
        __m128 Four_4x = _mm_set1_ps(4.0f);
        __m128 One255_4x = _mm_set1_ps(255.0f);
        __m128 Zero = _mm_set1_ps(0.0f);
        __m128i MaskFF = _mm_set1_epi32(0xFF);
        __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
        __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
        __m128 Colorr_4x = _mm_set1_ps(Color.r);
        __m128 Colorg_4x = _mm_set1_ps(Color.g);
        __m128 Colorb_4x = _mm_set1_ps(Color.b);
        __m128 Colora_4x = _mm_set1_ps(Color.a);
        __m128 nXAxisx_4x = _mm_set1_ps(nXAxis.x);
        __m128 nXAxisy_4x = _mm_set1_ps(nXAxis.y);
        __m128 nYAxisx_4x = _mm_set1_ps(nYAxis.x);
        __m128 nYAxisy_4x = _mm_set1_ps(nYAxis.y);
        __m128 Originx_4x = _mm_set1_ps(Origin.x);
        __m128 Originy_4x = _mm_set1_ps(Origin.y);
        __m128 MaxColorValue = _mm_set1_ps(255.0f*255.0f);
        __m128i TexturePitch_4x = _mm_set1_epi32(Texture->Pitch);

        __m128 WidthM2 = _mm_set1_ps((real32)(Texture->Width - 2));
        __m128 HeightM2 = _mm_set1_ps((real32)(Texture->Height - 2));
    
        uint8 *Row = ((uint8 *)Buffer->Memory +
                      FillRect.MinX*BITMAP_BYTES_PER_PIXEL +
                      FillRect.MinY*Buffer->Pitch);
        int32 RowAdvance = Buffer->Pitch;
    
        void *TextureMemory = Texture->Memory;
        int32 TexturePitch = Texture->Pitch;

        int MinY = FillRect.MinY;
        int MaxY = FillRect.MaxY;
        int MinX = FillRect.MinX;
        int MaxX = FillRect.MaxX;
        
        IGNORED_TIMED_BLOCK("Pixel Fill", GetClampedRectArea(FillRect) / 2);
        for(int Y = MinY;
            Y < MaxY;
            ++Y)
        {
            __m128 PixelPy = _mm_set1_ps((real32)Y);
            PixelPy = _mm_sub_ps(PixelPy, Originy_4x);
            __m128 PynX = _mm_mul_ps(PixelPy, nXAxisy_4x);
            __m128 PynY = _mm_mul_ps(PixelPy, nYAxisy_4x);
        
            __m128 PixelPx = _mm_set_ps((real32)(MinX + 3),
                                        (real32)(MinX + 2),
                                        (real32)(MinX + 1),
                                        (real32)(MinX + 0));
            PixelPx = _mm_sub_ps(PixelPx, Originx_4x);

            __m128i ClipMask = StartClipMask;

            uint32 *Pixel = (uint32 *)Row;
            for(int XI = MinX;
                XI < MaxX;
                XI += 4)
            {
                IACA_VC64_START;
                __m128 U = _mm_add_ps(_mm_mul_ps(PixelPx, nXAxisx_4x), PynX);
                __m128 V = _mm_add_ps(_mm_mul_ps(PixelPx, nYAxisx_4x), PynY);

                __m128i WriteMask = _mm_castps_si128(_mm_and_ps(_mm_and_ps(_mm_cmpge_ps(U, Zero),
                                                                           _mm_cmple_ps(U, One)),
                                                                _mm_and_ps(_mm_cmpge_ps(V, Zero),
                                                                           _mm_cmple_ps(V, One))));
                WriteMask = _mm_and_si128(WriteMask, ClipMask);
            
// TODO(casey): Later, re-check if this helps
//            if(_mm_movemask_epi8(WriteMask))
                {
                    __m128i OriginalDest = _mm_load_si128((__m128i *)Pixel);

                    U = _mm_min_ps(_mm_max_ps(U, Zero), One);
                    V = _mm_min_ps(_mm_max_ps(V, Zero), One);

                    // NOTE(casey): Bias texture coordinates to start
                    // on the boundary between the 0,0 and 1,1 pixels.
                    __m128 tX = _mm_add_ps(_mm_mul_ps(U, WidthM2), Half);
                    __m128 tY = _mm_add_ps(_mm_mul_ps(V, HeightM2), Half);
                
                    __m128i FetchX_4x = _mm_cvttps_epi32(tX);
                    __m128i FetchY_4x = _mm_cvttps_epi32(tY);
            
                    __m128 fX = _mm_sub_ps(tX, _mm_cvtepi32_ps(FetchX_4x));
                    __m128 fY = _mm_sub_ps(tY, _mm_cvtepi32_ps(FetchY_4x));

                    FetchX_4x = _mm_slli_epi32(FetchX_4x, 2);
                    FetchY_4x = _mm_or_si128(_mm_mullo_epi16(FetchY_4x, TexturePitch_4x),
                                             _mm_slli_epi32(_mm_mulhi_epi16(FetchY_4x, TexturePitch_4x), 16));
                    __m128i Fetch_4x = _mm_add_epi32(FetchX_4x, FetchY_4x);

                    int32 Fetch0 = Mi(Fetch_4x, 0);
                    int32 Fetch1 = Mi(Fetch_4x, 1);
                    int32 Fetch2 = Mi(Fetch_4x, 2);
                    int32 Fetch3 = Mi(Fetch_4x, 3);

                    uint8 *TexelPtr0 = ((uint8 *)TextureMemory) + Fetch0;
                    uint8 *TexelPtr1 = ((uint8 *)TextureMemory) + Fetch1;
                    uint8 *TexelPtr2 = ((uint8 *)TextureMemory) + Fetch2;
                    uint8 *TexelPtr3 = ((uint8 *)TextureMemory) + Fetch3;

                    __m128i SampleA = _mm_setr_epi32(*(uint32 *)(TexelPtr0),
                                                     *(uint32 *)(TexelPtr1),
                                                     *(uint32 *)(TexelPtr2),
                                                     *(uint32 *)(TexelPtr3));

                    __m128i SampleB = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + sizeof(uint32)),
                                                     *(uint32 *)(TexelPtr1 + sizeof(uint32)),
                                                     *(uint32 *)(TexelPtr2 + sizeof(uint32)),
                                                     *(uint32 *)(TexelPtr3 + sizeof(uint32)));

                    __m128i SampleC = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + TexturePitch),
                                                     *(uint32 *)(TexelPtr1 + TexturePitch),
                                                     *(uint32 *)(TexelPtr2 + TexturePitch),
                                                     *(uint32 *)(TexelPtr3 + TexturePitch));
                
                    __m128i SampleD = _mm_setr_epi32(*(uint32 *)(TexelPtr0 + TexturePitch + sizeof(uint32)),
                                                     *(uint32 *)(TexelPtr1 + TexturePitch + sizeof(uint32)),
                                                     *(uint32 *)(TexelPtr2 + TexturePitch + sizeof(uint32)),
                                                     *(uint32 *)(TexelPtr3 + TexturePitch + sizeof(uint32)));
                    
                    // NOTE(casey): Unpack bilinear samples
                    __m128i TexelArb = _mm_and_si128(SampleA, MaskFF00FF);
                    __m128i TexelAag = _mm_and_si128(_mm_srli_epi32(SampleA, 8), MaskFF00FF);
                    TexelArb = _mm_mullo_epi16(TexelArb, TexelArb);
                    __m128 TexelAa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelAag, 16));
                    TexelAag = _mm_mullo_epi16(TexelAag, TexelAag);

                    __m128i TexelBrb = _mm_and_si128(SampleB, MaskFF00FF);
                    __m128i TexelBag = _mm_and_si128(_mm_srli_epi32(SampleB, 8), MaskFF00FF);
                    TexelBrb = _mm_mullo_epi16(TexelBrb, TexelBrb);
                    __m128 TexelBa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelBag, 16));
                    TexelBag = _mm_mullo_epi16(TexelBag, TexelBag);

                    __m128i TexelCrb = _mm_and_si128(SampleC, MaskFF00FF);
                    __m128i TexelCag = _mm_and_si128(_mm_srli_epi32(SampleC, 8), MaskFF00FF);
                    TexelCrb = _mm_mullo_epi16(TexelCrb, TexelCrb);
                    __m128 TexelCa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelCag, 16));
                    TexelCag = _mm_mullo_epi16(TexelCag, TexelCag);

                    __m128i TexelDrb = _mm_and_si128(SampleD, MaskFF00FF);
                    __m128i TexelDag = _mm_and_si128(_mm_srli_epi32(SampleD, 8), MaskFF00FF);
                    TexelDrb = _mm_mullo_epi16(TexelDrb, TexelDrb);
                    __m128 TexelDa = _mm_cvtepi32_ps(_mm_srli_epi32(TexelDag, 16));
                    TexelDag = _mm_mullo_epi16(TexelDag, TexelDag);
            
                    // NOTE(casey): Load destination
                    __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest, MaskFF));
                    __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), MaskFF));
                    __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), MaskFF));
                    __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));
            
                    // NOTE(casey): Convert texture from 0-255 sRGB to "linear" 0-1 brightness space
                    __m128 TexelAr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelArb, 16));
                    __m128 TexelAg = _mm_cvtepi32_ps(_mm_and_si128(TexelAag, MaskFFFF));
                    __m128 TexelAb = _mm_cvtepi32_ps(_mm_and_si128(TexelArb, MaskFFFF));

                    __m128 TexelBr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelBrb, 16));
                    __m128 TexelBg = _mm_cvtepi32_ps(_mm_and_si128(TexelBag, MaskFFFF));
                    __m128 TexelBb = _mm_cvtepi32_ps(_mm_and_si128(TexelBrb, MaskFFFF));

                    __m128 TexelCr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelCrb, 16));
                    __m128 TexelCg = _mm_cvtepi32_ps(_mm_and_si128(TexelCag, MaskFFFF));
                    __m128 TexelCb = _mm_cvtepi32_ps(_mm_and_si128(TexelCrb, MaskFFFF));

                    __m128 TexelDr = _mm_cvtepi32_ps(_mm_srli_epi32(TexelDrb, 16));
                    __m128 TexelDg = _mm_cvtepi32_ps(_mm_and_si128(TexelDag, MaskFFFF));
                    __m128 TexelDb = _mm_cvtepi32_ps(_mm_and_si128(TexelDrb, MaskFFFF));
                    
                    // NOTE(casey): Bilinear texture blend
                    __m128 ifX = _mm_sub_ps(One, fX);
                    __m128 ifY = _mm_sub_ps(One, fY);
                
                    __m128 l0 = _mm_mul_ps(ifY, ifX);
                    __m128 l1 = _mm_mul_ps(ifY, fX);
                    __m128 l2 = _mm_mul_ps(fY, ifX);
                    __m128 l3 = _mm_mul_ps(fY, fX);

                    __m128 Texelr = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAr), _mm_mul_ps(l1, TexelBr)),
                                               _mm_add_ps(_mm_mul_ps(l2, TexelCr), _mm_mul_ps(l3, TexelDr)));
                                           __m128 Texelg = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAg), _mm_mul_ps(l1, TexelBg)),
                                               _mm_add_ps(_mm_mul_ps(l2, TexelCg), _mm_mul_ps(l3, TexelDg)));
                    __m128 Texelb = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAb), _mm_mul_ps(l1, TexelBb)),
                                               _mm_add_ps(_mm_mul_ps(l2, TexelCb), _mm_mul_ps(l3, TexelDb)));
                                           __m128 Texela = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAa), _mm_mul_ps(l1, TexelBa)),
                                               _mm_add_ps(_mm_mul_ps(l2, TexelCa), _mm_mul_ps(l3, TexelDa)));

                    // NOTE(casey): Modulate by incoming color
                    Texelr = _mm_mul_ps(Texelr, Colorr_4x);
                    Texelg = _mm_mul_ps(Texelg, Colorg_4x);
                    Texelb = _mm_mul_ps(Texelb, Colorb_4x);
                    Texela = _mm_mul_ps(Texela, Colora_4x);

                    Texelr = _mm_min_ps(_mm_max_ps(Texelr, Zero), MaxColorValue);
                    Texelg = _mm_min_ps(_mm_max_ps(Texelg, Zero), MaxColorValue);
                    Texelb = _mm_min_ps(_mm_max_ps(Texelb, Zero), MaxColorValue);
                            
                    // NOTE(casey): Go from sRGB to "linear" brightness space
                    Destr = mmSquare(Destr);
                    Destg = mmSquare(Destg);
                    Destb = mmSquare(Destb);

                    // NOTE(casey): Destination blend
                    __m128 InvTexelA = _mm_sub_ps(One, _mm_mul_ps(Inv255_4x, Texela));
                    __m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvTexelA, Destr), Texelr);
                    __m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvTexelA, Destg), Texelg);
                    __m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvTexelA, Destb), Texelb);
                    __m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvTexelA, Desta), Texela);
        
                    // NOTE(casey): Go from "linear" 0-65534 brightness space to sRGB 0-255
#if 1
                    Blendedr = _mm_mul_ps(Blendedr, _mm_rsqrt_ps(Blendedr));
                    Blendedg = _mm_mul_ps(Blendedg, _mm_rsqrt_ps(Blendedg));
                    Blendedb = _mm_mul_ps(Blendedb, _mm_rsqrt_ps(Blendedb));
#else
                    Blendedr = _mm_sqrt_ps(Blendedr);
                    Blendedg = _mm_sqrt_ps(Blendedg);
                    Blendedb = _mm_sqrt_ps(Blendedb);
#endif
                    Blendeda = Blendeda;
            
                    __m128i Intr = _mm_cvtps_epi32(Blendedr);
                    __m128i Intg = _mm_cvtps_epi32(Blendedg);
                    __m128i Intb = _mm_cvtps_epi32(Blendedb);
                    __m128i Inta = _mm_cvtps_epi32(Blendeda);

                    __m128i Sr = _mm_slli_epi32(Intr, 16);
                    __m128i Sg = _mm_slli_epi32(Intg, 8);
                    __m128i Sb = Intb;
                    __m128i Sa = _mm_slli_epi32(Inta, 24);

                    __m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));

                    __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out),
                                                     _mm_andnot_si128(WriteMask, OriginalDest));
                    _mm_store_si128((__m128i *)Pixel, MaskedOut);
                }
            
                PixelPx = _mm_add_ps(PixelPx, Four_4x);
                Pixel += 4;

                if((XI + 8) < MaxX)
                {
                    ClipMask = _mm_set1_epi8(-1);
                }
                else
                {
                    ClipMask = EndClipMask;
                }

                IACA_VC64_END;
            }
        
            Row += RowAdvance;
        }
    }
}

internal void
BlendRenderTarget(rectangle2i Rect, software_texture *DestTarget, r32 Alpha,
                  software_texture *SourceTarget)
{
    IGNORED_TIMED_FUNCTION();
    
#if 0
    u8 *DestRow = ((u8 *)DestTarget->Memory +
                   Rect.MinX*BITMAP_BYTES_PER_PIXEL +
                   Rect.MinY*DestTarget->Pitch);
    u8 *SourceRow = ((u8 *)SourceTarget->Memory +
                     Rect.MinX*BITMAP_BYTES_PER_PIXEL +
                     Rect.MinY*SourceTarget->Pitch);
    for(s32 Y = Rect.MinY;
        Y < Rect.MaxY;
        ++Y)
    {
        u32 *DestPixel = (u32 *)DestRow;
        u32 *SourcePixel = (u32 *)SourceRow;
        
        for(s32 X = Rect.MinX;
            X < Rect.MaxX;
            ++X)
        {
            v4 SourceColor = SRGB255ToLinear1(Unpack4x8(*SourcePixel));
            v4 DestColor = SRGB255ToLinear1(Unpack4x8(*DestPixel));
            
            r32 PixelAlpha = Alpha*SourceColor.a;
            
            v4 Result = (1.0f - PixelAlpha)*DestColor + PixelAlpha*SourceColor;
            
            *DestPixel = Pack4x8(Linear1ToSRGB255(Result));
            
            ++DestPixel;
            ++SourcePixel;
        }
        
        DestRow += DestTarget->Pitch;
        SourceRow += SourceTarget->Pitch;
    }
#else
    if(HasArea(Rect))
    {
        __m128i StartClipMask = _mm_set1_epi8(-1);
        __m128i EndClipMask = _mm_set1_epi8(-1);

        __m128i StartClipMasks[] =
        {
            _mm_slli_si128(StartClipMask, 0*4),
            _mm_slli_si128(StartClipMask, 1*4),
            _mm_slli_si128(StartClipMask, 2*4),
            _mm_slli_si128(StartClipMask, 3*4),
        };

        __m128i EndClipMasks[] =
        {
            _mm_srli_si128(EndClipMask, 0*4),
            _mm_srli_si128(EndClipMask, 3*4),
            _mm_srli_si128(EndClipMask, 2*4),
            _mm_srli_si128(EndClipMask, 1*4),
        };
        
        if(Rect.MinX & 3)
        {
            StartClipMask = StartClipMasks[Rect.MinX & 3];
            Rect.MinX = Rect.MinX & ~3;
        }

        if(Rect.MaxX & 3)
        {
            EndClipMask = EndClipMasks[Rect.MaxX & 3];
            Rect.MaxX = (Rect.MaxX & ~3) + 4;
        }
            
        real32 Inv255 = 1.0f / 255.0f;
        __m128 Inv255_4x = _mm_set1_ps(Inv255);
        real32 One255 = 255.0f;

        __m128 One = _mm_set1_ps(1.0f);
        __m128 Half = _mm_set1_ps(0.5f);
        __m128 Four_4x = _mm_set1_ps(4.0f);
        __m128 One255_4x = _mm_set1_ps(255.0f);
        __m128 Zero = _mm_set1_ps(0.0f);
        __m128i MaskFF = _mm_set1_epi32(0xFF);
        __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
        __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
        __m128 Alpha_4x = _mm_set1_ps(Alpha);
        __m128 InvAlpha_4x = _mm_set1_ps(1.0f - Alpha);
        __m128 MaxColorValue = _mm_set1_ps(255.0f*255.0f);
        
        u8 *DestRow = ((u8 *)DestTarget->Memory +
                       Rect.MinX*BITMAP_BYTES_PER_PIXEL +
                       Rect.MinY*DestTarget->Pitch);
        u8 *SourceRow = ((u8 *)SourceTarget->Memory +
                         Rect.MinX*BITMAP_BYTES_PER_PIXEL +
                         Rect.MinY*SourceTarget->Pitch);
        
        int32 DestRowAdvance = DestTarget->Pitch;
        int32 SourceRowAdvance = SourceTarget->Pitch;
        
        int MinY = Rect.MinY;
        int MaxY = Rect.MaxY;
        int MinX = Rect.MinX;
        int MaxX = Rect.MaxX;
        
        IGNORED_TIMED_BLOCK("Pixel Fill", GetClampedRectArea(FillRect) / 2);
        for(int Y = MinY;
            Y < MaxY;
            ++Y)
        {
            __m128i ClipMask = StartClipMask;
            
            uint32 *DestPixel = (uint32 *)DestRow;
            uint32 *SourcePixel = (uint32 *)SourceRow;
            for(int XI = MinX;
                XI < MaxX;
                XI += 4)
            {
                
                IACA_VC64_START;
                __m128i WriteMask = ClipMask;
            
// TODO(casey): Later, re-check if this helps
//            if(_mm_movemask_epi8(WriteMask))
                {
                    __m128i OriginalDest = _mm_load_si128((__m128i *)DestPixel);
                    __m128i OriginalSource = _mm_load_si128((__m128i *)SourcePixel);
                    
                    // NOTE(casey): Load destination
                    __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(OriginalDest, MaskFF));
                    __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 8), MaskFF));
                    __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 16), MaskFF));
                    __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));
                    
                    // NOTE(casey): Load source
                    __m128 Sourceb = _mm_cvtepi32_ps(_mm_and_si128(OriginalSource, MaskFF));
                    __m128 Sourceg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalSource, 8), MaskFF));
                    __m128 Sourcer = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalSource, 16), MaskFF));
                    __m128 Sourcea = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalSource, 24), MaskFF));
                    
                    // NOTE(casey): Go from sRGB to "linear" brightness space
                    Destr = mmSquare(Destr);
                    Destg = mmSquare(Destg);
                    Destb = mmSquare(Destb);
                    Sourcer = mmSquare(Sourcer);
                    Sourceg = mmSquare(Sourceg);
                    Sourceb = mmSquare(Sourceb);
                    
                    // NOTE(casey): Destination blend
                    __m128 PixelAlpha_4x = _mm_mul_ps(Alpha_4x, _mm_mul_ps(Inv255_4x, Sourcea));
                    __m128 InvPixelAlpha_4x = _mm_sub_ps(One, PixelAlpha_4x);
                    
                    __m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvPixelAlpha_4x, Destr), _mm_mul_ps(PixelAlpha_4x, Sourcer));
                    __m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvPixelAlpha_4x, Destg), _mm_mul_ps(PixelAlpha_4x, Sourceg));
                    __m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvPixelAlpha_4x, Destb), _mm_mul_ps(PixelAlpha_4x, Sourceb));
                    __m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvPixelAlpha_4x, Desta), _mm_mul_ps(PixelAlpha_4x, Sourcea));
        
                    // NOTE(casey): Go from "linear" 0-1 brightness space to sRGB 0-255
                    Blendedr = _mm_mul_ps(Blendedr, _mm_rsqrt_ps(Blendedr));
                    Blendedg = _mm_mul_ps(Blendedg, _mm_rsqrt_ps(Blendedg));
                    Blendedb = _mm_mul_ps(Blendedb, _mm_rsqrt_ps(Blendedb));
                    
                    __m128i Intr = _mm_cvtps_epi32(Blendedr);
                    __m128i Intg = _mm_cvtps_epi32(Blendedg);
                    __m128i Intb = _mm_cvtps_epi32(Blendedb);
                    __m128i Inta = _mm_cvtps_epi32(Blendeda);

                    __m128i Sr = _mm_slli_epi32(Intr, 16);
                    __m128i Sg = _mm_slli_epi32(Intg, 8);
                    __m128i Sb = Intb;
                    __m128i Sa = _mm_slli_epi32(Inta, 24);

                    __m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));

                    __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out),
                                                     _mm_andnot_si128(WriteMask, OriginalDest));
                    _mm_store_si128((__m128i *)DestPixel, MaskedOut);
                }
            
                SourcePixel += 4;
                DestPixel += 4;
                
                if((XI + 8) < MaxX)
                {
                    ClipMask = _mm_set1_epi8(-1);
                }
                else
                {
                    ClipMask = EndClipMask;
                }

                IACA_VC64_END;
            }
        
            SourceRow += SourceRowAdvance;
            DestRow += DestRowAdvance;
            
        }
    }
#endif
}


internal void
ClearRectangle(rectangle2i Rect, software_texture *DestTarget, v4 Color)
{
    IGNORED_TIMED_FUNCTION();
    
    if(HasArea(Rect))
    {
        __m128i StartClipMask = _mm_set1_epi8(-1);
        __m128i EndClipMask = _mm_set1_epi8(-1);

        __m128i StartClipMasks[] =
        {
            _mm_slli_si128(StartClipMask, 0*4),
            _mm_slli_si128(StartClipMask, 1*4),
            _mm_slli_si128(StartClipMask, 2*4),
            _mm_slli_si128(StartClipMask, 3*4),
        };

        __m128i EndClipMasks[] =
        {
            _mm_srli_si128(EndClipMask, 0*4),
            _mm_srli_si128(EndClipMask, 3*4),
            _mm_srli_si128(EndClipMask, 2*4),
            _mm_srli_si128(EndClipMask, 1*4),
        };
        
        if(Rect.MinX & 3)
        {
            StartClipMask = StartClipMasks[Rect.MinX & 3];
            Rect.MinX = Rect.MinX & ~3;
        }

        if(Rect.MaxX & 3)
        {
            EndClipMask = EndClipMasks[Rect.MaxX & 3];
            Rect.MaxX = (Rect.MaxX & ~3) + 4;
        }
            
#if 0
        real32 Inv255 = 1.0f / 255.0f;
        __m128 Inv255_4x = _mm_set1_ps(Inv255);
        real32 One255 = 255.0f;

        __m128 One = _mm_set1_ps(1.0f);
        __m128 Half = _mm_set1_ps(0.5f);
        __m128 Four_4x = _mm_set1_ps(4.0f);
        __m128 Zero = _mm_set1_ps(0.0f);
        __m128i MaskFF = _mm_set1_epi32(0xFF);
        __m128i MaskFFFF = _mm_set1_epi32(0xFFFF);
        __m128i MaskFF00FF = _mm_set1_epi32(0x00FF00FF);
        __m128 Alpha_4x = _mm_set1_ps(Alpha);
        __m128 InvAlpha_4x = _mm_set1_ps(1.0f - Alpha);
        __m128 MaxColorValue = _mm_set1_ps(255.0f*255.0f);
#endif
        
        __m128 Colorr = _mm_set1_ps(255.0f*255.0f*Color.r);
        __m128 Colorg = _mm_set1_ps(255.0f*255.0f*Color.g);
        __m128 Colorb = _mm_set1_ps(255.0f*255.0f*Color.b);
        __m128 Colora = _mm_set1_ps(255.0f*255.0f*Color.a);
        
        // NOTE(casey): OpenGL et al specify clear colors as linear, so they must
        // be converted to sRGB before storing.
        
        __m128 Blendedr = _mm_mul_ps(Colorr, _mm_rsqrt_ps(Colorr));
        __m128 Blendedg = _mm_mul_ps(Colorg, _mm_rsqrt_ps(Colorg));
        __m128 Blendedb = _mm_mul_ps(Colorb, _mm_rsqrt_ps(Colorb));
        __m128 Blendeda = Colora;
        
        __m128i Intr = _mm_cvtps_epi32(Blendedr);
        __m128i Intg = _mm_cvtps_epi32(Blendedg);
        __m128i Intb = _mm_cvtps_epi32(Blendedb);
        __m128i Inta = _mm_cvtps_epi32(Blendeda);
        
        __m128i Sr = _mm_slli_epi32(Intr, 16);
        __m128i Sg = _mm_slli_epi32(Intg, 8);
        __m128i Sb = Intb;
        __m128i Sa = _mm_slli_epi32(Inta, 24);
        __m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));
        //
        
        u8 *DestRow = ((u8 *)DestTarget->Memory +
                       Rect.MinX*BITMAP_BYTES_PER_PIXEL +
                       Rect.MinY*DestTarget->Pitch);
        int32 DestRowAdvance = DestTarget->Pitch;
        
        int MinY = Rect.MinY;
        int MaxY = Rect.MaxY;
        int MinX = Rect.MinX;
        int MaxX = Rect.MaxX;
        
        for(int Y = MinY;
            Y < MaxY;
            ++Y)
        {
            __m128i ClipMask = StartClipMask;
            
            uint32 *DestPixel = (uint32 *)DestRow;
            for(int XI = MinX;
                XI < MaxX;
                XI += 4)
            {
                __m128i WriteMask = ClipMask;
                
                __m128i OriginalDest = _mm_load_si128((__m128i *)DestPixel);
                __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out),
                                                 _mm_andnot_si128(WriteMask, OriginalDest));
                _mm_store_si128((__m128i *)DestPixel, MaskedOut);
                
                DestPixel += 4;
                
                if((XI + 8) < MaxX)
                {
                    ClipMask = _mm_set1_epi8(-1);
                }
                else
                {
                    ClipMask = EndClipMask;
                }
            }
        
            DestRow += DestRowAdvance;
        }
    }
}

internal void
RenderCommandsToBitmap(game_render_commands *Commands,
                       software_texture *RenderTargets, rectangle2i BaseClipRect)
{
#if 0
    IGNORED_TIMED_FUNCTION();

    real32 NullPixelsToMeters = 1.0f;

    u32 ClipRectIndex = 0xFFFFFFFF;
    rectangle2i ClipRect = BaseClipRect;
    
    // TODO(casey): Put clears back into the list!  Just use sort barriers to isolate them.
    for(u32 TargetIndex = 0;
        TargetIndex <= Commands->MaxRenderTargetIndex;
        ++TargetIndex)
    {
        software_texture *OutputTarget = RenderTargets + TargetIndex;
        ClearRectangle(ClipRect, OutputTarget, Commands->ClearColor);
    }
    
    software_texture *OutputTarget = RenderTargets;
    // TODO(casey): Make the loop work like it did before
    // (need to make the headers push in order again!)
    if(0)
    {
        u32 HeaderOffset = 0;
        
        render_group_entry_header *Header = (render_group_entry_header *)
            (Commands->PushBufferBase + HeaderOffset);
        if(ClipRectIndex != Header->ClipRectIndex)
        {
            ClipRectIndex = Header->ClipRectIndex;
            Assert(ClipRectIndex < Commands->ClipRectCount);
            
            render_entry_cliprect *Clip = Prep->ClipRects + ClipRectIndex;
            ClipRect = Intersect(BaseClipRect, Clip->Rect);
            
            OutputTarget = RenderTargets + Clip->RenderTargetIndex;
        }
        
        void *Data = (uint8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
#if 0
            case RenderGroupEntryType_render_entry_bitmap:
            {
                render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
                Assert(Entry->Bitmap);
                
#if 0
                //                DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y, Entry->Color.a);
                DrawRectangleSlowly(OutputTarget, Entry->P,
                                    V2(Entry->Size.x, 0),
                                    V2(0, Entry->Size.y), Entry->Color,
                                    Entry->Bitmap, 0, 0, 0, 0, NullPixelsToMeters);
#else
                DrawRectangleQuickly(OutputTarget, Entry->P.xy,
                                     Entry->XAxis.xy,
                                     Entry->YAxis.xy, Entry->PremulColor,
                                     Entry->Bitmap, NullPixelsToMeters, ClipRect);
#endif
            } break;
            
            case RenderGroupEntryType_render_entry_rectangle:
            {
                render_entry_rectangle *Entry = (render_entry_rectangle *)Data;
                DrawRectangle(OutputTarget, Entry->P.xy, Entry->P.xy + Entry->Dim, Entry->PremulColor, ClipRect);
            } break;
#endif
            case RenderGroupEntryType_render_entry_blend_render_target:
            {
                render_entry_blend_render_target *Entry = (render_entry_blend_render_target *)Data;
                software_texture *SourceTarget = RenderTargets + Entry->SourceTargetIndex;
                BlendRenderTarget(ClipRect, OutputTarget, Entry->Alpha, SourceTarget);
            } break;
            
            InvalidDefaultCase;
        }
    }
#endif
}

internal 
PLATFORM_WORK_QUEUE_CALLBACK(DoTiledRenderWork)
{
    TIMED_FUNCTION();

    tile_render_work *Work = (tile_render_work *)Data;

    RenderCommandsToBitmap(Work->Commands, Work->RenderTargets, Work->ClipRect);
}

internal void
SoftwareRenderCommands(platform_work_queue *RenderQueue,
                       game_render_commands *Commands,
                       software_texture *FinalOutputTarget, memory_arena *TempArena)
{
    TIMED_FUNCTION();

#if 0
    /*
      TODO(casey):
      
      - Make sure that tiles are all cache-aligned
      - Can we get hyperthreads synced so they do interleaved lines?
      - How big should the tiles be for performance?
      - Actually ballpark the memory bandwidth for our DrawRectangleQuickly
      - Re-test some of our instruction choices
    */
    u32 RenderTargetCount = Commands->MaxRenderTargetIndex + 1;
    software_texture *RenderTargets = PushArray(TempArena, RenderTargetCount, software_texture);
    RenderTargets[0] = *FinalOutputTarget;
    Assert(FinalOutputTarget->Pitch > 0);
    for(u32 TargetIndex =  1;
        TargetIndex < RenderTargetCount;
        ++TargetIndex)
    {
        software_texture *Target = RenderTargets + TargetIndex;
        *Target = *FinalOutputTarget;
        Target->Memory = PushSize(TempArena, Target->Pitch*Target->Height, AlignNoClear(16));
    }
    
    int const TileCountX = 4;
    int const TileCountY = 4;
    tile_render_work WorkArray[TileCountX*TileCountY];

    Assert(((uintptr)FinalOutputTarget->Memory & 15) == 0);
    int TileWidth = FinalOutputTarget->Width / TileCountX;
    int TileHeight = FinalOutputTarget->Height / TileCountY;

    TileWidth = ((TileWidth + 3) / 4) * 4;

    int WorkCount = 0;
    for(int TileY = 0;
        TileY < TileCountY;
        ++TileY)
    {
        for(int TileX = 0;
            TileX < TileCountX;
            ++TileX)
        {
            tile_render_work *Work = WorkArray + WorkCount++;

            rectangle2i ClipRect;
            ClipRect.MinX = TileX*TileWidth;
            ClipRect.MaxX = ClipRect.MinX + TileWidth;
            ClipRect.MinY = TileY*TileHeight;
            ClipRect.MaxY = ClipRect.MinY + TileHeight;

            if(TileX == (TileCountX - 1))
            {
                ClipRect.MaxX = FinalOutputTarget->Width;
            }
            if(TileY == (TileCountY - 1))
            {
                ClipRect.MaxY = FinalOutputTarget->Height;
            }

            Work->Commands = Commands;
            Work->RenderTargets = RenderTargets;
            Work->ClipRect = ClipRect;
#if 1
            // NOTE(casey): This is the multi-threaded path
            Platform.AddEntry(RenderQueue, DoTiledRenderWork, Work);
#else
            // NOTE(casey): This is the single-threaded path
            DoTiledRenderWork(RenderQueue, Work);
#endif
        }
    }
    Platform.CompleteAllWork(RenderQueue);
#endif
}
