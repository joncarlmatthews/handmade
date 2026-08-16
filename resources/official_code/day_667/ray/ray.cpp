/* TODO(casey):

   This is only the very barest of bones for a raytracer!  We are computing
   things inaccurately and physically incorrect _everywhere_.  We'll
   fix it some day when we come back to it :)
   
   
   - XOrShift32 needs to be replaced by something that can generate good random
     values reasonably quickly!
*/


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <float.h>
#include <time.h>
#include <assert.h>

#include "ray.h"

internal u32
GetTotalPixelSize(image_u32 Image)
{
    u32 Result = Image.Width*Image.Height*sizeof(u32);
    return(Result);
}

internal u32 *
GetPixelPointer(image_u32 Image, u32 X, u32 Y)
{
    u32 *Result = Image.Pixels + Y*Image.Width + X;
    return(Result);
}

internal image_u32
AllocateImage(u32 Width, u32 Height)
{
    image_u32 Image = {};
    Image.Width = Width;
    Image.Height = Height;
    
    u32 OutputPixelSize = GetTotalPixelSize(Image);
    Image.Pixels = (u32 *)malloc(OutputPixelSize);
    
    return(Image);
}

internal void
WriteImage(image_u32 Image, char *OutputFileName)
{
    u32 OutputPixelSize = GetTotalPixelSize(Image);
    
    bitmap_header Header = {};
    Header.FileType = 0x4D42;
    Header.FileSize = sizeof(Header) + OutputPixelSize;
    Header.BitmapOffset = sizeof(Header);
    Header.Size = sizeof(Header) - 14;
    Header.Width = Image.Width;
    Header.Height = Image.Height;
    Header.Planes = 1;
    Header.BitsPerPixel = 32;
    Header.Compression = 0;
    Header.SizeOfBitmap = OutputPixelSize;
    Header.HorzResolution = 0;
    Header.VertResolution = 0;
    Header.ColorsUsed = 0;
    Header.ColorsImportant = 0;
    
#if 0
    FILE *InFile = fopen("reference.bmp", "rb");
    bitmap_header RefHeader = {};
    fread(&RefHeader, sizeof(RefHeader), 1, InFile);
    fclose(InFile);
#endif
    
    FILE *OutFile = fopen(OutputFileName, "wb");
    if(OutFile)
    {
        fwrite(&Header, sizeof(Header), 1, OutFile);
        fwrite(Image.Pixels, OutputPixelSize, 1, OutFile);
        fclose(OutFile);
    }
    else
    {
        fprintf(stderr, "[ERROR] Unable to write output file %s.\n", OutputFileName);
    }
}

internal f32
ExactLinearTosRGB(f32 L)
{
    if(L < 0.0f)
    {
        L = 0.0f;
    }
    
    if(L > 1.0f)
    {
        L = 1.0f;
    }
    
    f32 S = L*12.92f;
    if(L > 0.0031308f)
    {
        S = 1.055f*Pow(L, 1.0f/2.4f) - 0.055f;
    }
    
    return(S);
}

internal lane_u32
XOrShift32(random_series *Series)
{
    // NOTE(casey): Reference XOrShift from https://en.wikipedia.org/wiki/Xorshift
    lane_u32 x = Series->State;
    
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    
    Series->State = x;
    
    return(x);
}

internal lane_f32
RandomUnilateral(random_series *Series)
{
    lane_f32 Result = LaneF32FromU32(XOrShift32(Series) >> 1) / (f32)(U32Max >> 1);
    return(Result);
}

internal lane_f32
RandomBilateral(random_series *Series)
{
    lane_f32 Result = -1.0f + 2.0f*RandomUnilateral(Series);
    return(Result);
}

internal lane_v3
BRDFLookup(material *Materials, lane_u32 MatIndex, lane_v3 ViewDir, lane_v3 Normal, lane_v3 Tangent, lane_v3 Binormal, lane_v3 LightDir)
{
    lane_v3 HalfVector = NOZ(0.5f*(ViewDir + LightDir));
    
    lane_v3 LW;
    LW.x = Inner(LightDir, Tangent);
    LW.y = Inner(LightDir, Binormal);
    LW.z = Inner(LightDir, Normal);
    
    lane_v3 HW;
    HW.x = Inner(HalfVector, Tangent);
    HW.y = Inner(HalfVector, Binormal);
    HW.z = Inner(HalfVector, Normal);
    
    lane_v3 DiffY = NOZ(Cross(HW, Tangent));
    lane_v3 DiffX = Cross(DiffY, HW);
    
    lane_f32 DiffXInner = Inner(DiffX, LW);
    lane_f32 DiffYInner = Inner(DiffY, LW);
    lane_f32 DiffZInner = Inner(HW, LW);
    
    lane_v3 Result;
    for(u32 SubIndex = 0;
        SubIndex < LANE_WIDTH;
        ++SubIndex)
    {
        f32 ThetaHalf = acosf(ExtractF32(HW.z, SubIndex));
        f32 ThetaDiff = acosf(ExtractF32(DiffZInner, SubIndex));
        f32 PhiDiff = atan2f(ExtractF32(DiffYInner, SubIndex),
                             ExtractF32(DiffXInner, SubIndex));
        if(PhiDiff < 0)
        {
            PhiDiff += Pi32;
        }
        
        brdf_table *Table = &Materials[((u32 *)&MatIndex)[SubIndex]].BRDF;
        
        // TODO(casey): Does this just undo what the acos did?  Because I 
        // think it does, and then we could just avoid the acos altogether...
        f32 F0 = SquareRoot(Clamp01(ThetaHalf / (0.5f*Pi32)));
        u32 I0 = RoundReal32ToUInt32((Table->Count[0] - 1)*F0);
        
        f32 F1 = Clamp01(ThetaDiff / (0.5f*Pi32));
        u32 I1 = RoundReal32ToUInt32((Table->Count[1] - 1)*F1);
        
        f32 F2 = Clamp01(PhiDiff / Pi32);
        u32 I2 = RoundReal32ToUInt32((Table->Count[2] - 1)*F2);
        
        u32 Index = I2 + I1*Table->Count[2] + I0*Table->Count[1]*Table->Count[2];
        
        Assert(Index < (Table->Count[0]*Table->Count[1]*Table->Count[2]));
        v3 Color = Table->Values[Index];
        
        ((f32 *)&Result.x)[SubIndex] = Color.x;
        ((f32 *)&Result.y)[SubIndex] = Color.y;
        ((f32 *)&Result.z)[SubIndex] = Color.z;
    }
    
    return(Result);
}

internal void
CastSampleRays(cast_state *State)
{
    world *World = State->World;
    u32 RaysPerPixel = State->RaysPerPixel;
    u32 MaxBounceCount = State->MaxBounceCount;
    lane_f32 FilmW = LaneF32FromF32(State->FilmW);
    lane_f32 FilmH = LaneF32FromF32(State->FilmH);
    lane_f32 HalfPixW = LaneF32FromF32(State->HalfPixW);
    lane_f32 HalfPixH = LaneF32FromF32(State->HalfPixH);
    lane_f32 FilmX = State->FilmX + HalfPixW;
    lane_f32 FilmY = State->FilmY + HalfPixH;
    lane_v3 FilmCenter = LaneV3FromV3(State->FilmCenter);
    lane_f32 HalfFilmW = LaneF32FromF32(State->HalfFilmW);
    lane_f32 HalfFilmH = LaneF32FromF32(State->HalfFilmH);
    lane_v3 CameraX = LaneV3FromV3(State->CameraX);
    lane_v3 CameraY = LaneV3FromV3(State->CameraY);
    lane_v3 CameraZ = LaneV3FromV3(State->CameraZ);
    lane_v3 CameraP = LaneV3FromV3(State->CameraP);
    random_series Series = State->Series;
    
    // TODO(casey): Are we worried about wrapping here?
    lane_u32 BouncesComputed = LaneU32FromU32(0);
    u64 LoopsComputed = 0;
    lane_v3 FinalColor = {};
    
    u32 LaneWidth = LANE_WIDTH;
    u32 LaneRayCount = (RaysPerPixel / LaneWidth);
    Assert((LaneRayCount*LANE_WIDTH) == RaysPerPixel);
    
    f32 Contrib = 1.0f / (f32)(RaysPerPixel);
    for(u32 RayIndex = 0;
        RayIndex < LaneRayCount;
        ++RayIndex)
    {
        lane_f32 OffX = FilmX + RandomBilateral(&Series) * HalfPixW;
        lane_f32 OffY = FilmY + RandomBilateral(&Series) * HalfPixH;
        lane_v3 FilmP = FilmCenter + OffX*HalfFilmW*CameraX + OffY*HalfFilmH*CameraY;
        
        lane_v3 RayOrigin = CameraP;
        lane_v3 RayDirection = NOZ(FilmP - CameraP);
        
        // TODO(casey): This is a completely ad-hoc tolerance for now.
        lane_f32 Tolerance = LaneF32FromF32(0.0001f);
        lane_f32 MinHitDistance = LaneF32FromF32(0.001f);
        
        lane_v3 Sample = {};
        lane_v3 Attenuation = V3(1, 1, 1);
        
        lane_u32 LaneMask = LaneU32FromU32(0xFFFFFFFF);
        
        // TODO(casey): Monte-carlo termination?
        for(u32 BounceCount = 0;
            BounceCount < MaxBounceCount;
            ++BounceCount)
        {
            lane_f32 HitDistance = LaneF32FromF32(F32Max);
            lane_u32 HitMatIndex = LaneU32FromU32(0);
            lane_v3 Normal = {};
            lane_v3 Tangent = {};
            lane_v3 Binormal = {};
            
            lane_u32 LaneIncrement = LaneU32FromU32(1);
            BouncesComputed += (LaneIncrement & LaneMask);
            LoopsComputed += LANE_WIDTH;
            
            for(u32 PlaneIndex = 0;
                PlaneIndex < World->PlaneCount;
                ++PlaneIndex)
            {
                plane Plane = World->Planes[PlaneIndex];
                
                lane_v3 PlaneN = LaneV3FromV3(Plane.Normal);
                lane_v3 PlaneT = LaneV3FromV3(Plane.Tangent);
                lane_v3 PlaneB = LaneV3FromV3(Plane.Binormal);
                lane_f32 PlaneD = LaneF32FromF32(Plane.d);
                
                lane_f32 Denom = Inner(PlaneN, RayDirection);
                lane_u32 DenomMask = ((Denom < -Tolerance) | (Denom > Tolerance));
                if(!MaskIsZeroed(DenomMask))
                {
                    lane_f32 t = (-PlaneD - Inner(PlaneN, RayOrigin)) / Denom;
                    lane_u32 tMask = ((t > MinHitDistance) & (t < HitDistance));
                    lane_u32 HitMask = (DenomMask & tMask);
                    if(!MaskIsZeroed(HitMask))
                    {
                        lane_u32 PlaneMatIndex = LaneU32FromU32(Plane.MatIndex);
                        
                        ConditionalAssign(&HitDistance, HitMask, t);
                        ConditionalAssign(&HitMatIndex, HitMask, PlaneMatIndex);
                        ConditionalAssign(&Normal, HitMask, PlaneN);
                        ConditionalAssign(&Tangent, HitMask, PlaneT);
                        ConditionalAssign(&Binormal, HitMask, PlaneB);
                    }
                }
            }
            
            for(u32 SphereIndex = 0;
                SphereIndex < World->SphereCount;
                ++SphereIndex)
            {
                sphere Sphere = World->Spheres[SphereIndex];
                
                lane_v3 SphereP = LaneV3FromV3(Sphere.P);
                lane_f32 Spherer = LaneF32FromF32(Sphere.r);
                
                /* TODO(casey): This code is butt. First, remove the 2's and 4's.
                   Second, you need to test the root term BEFORE you square root it! */
                
                lane_v3 SphereRelativeRayOrigin = RayOrigin - SphereP;
                lane_f32 a = Inner(RayDirection, RayDirection);
                lane_f32 b = 2.0f*Inner(RayDirection, SphereRelativeRayOrigin);
                lane_f32 c = Inner(SphereRelativeRayOrigin, SphereRelativeRayOrigin) - Spherer*Spherer;
                
                lane_f32 RootTerm = SquareRoot(b*b - 4.0f*a*c);
                lane_u32 RootMask = (RootTerm > Tolerance);
                if(!MaskIsZeroed(RootMask))
                {
                    lane_f32 Denom = 2.0f*a;
                    lane_f32 tp = (-b + RootTerm) / Denom;
                    lane_f32 tn = (-b - RootTerm) / Denom;
                    
                    lane_f32 t = tp;
                    lane_u32 PickMask = ((tn > MinHitDistance) & (tn < tp));
                    ConditionalAssign(&t, PickMask, tn);
                    
                    lane_u32 tMask = ((t > MinHitDistance) & (t < HitDistance));
                    lane_u32 HitMask = (RootMask & tMask);
                    
                    if(!MaskIsZeroed(HitMask))
                    {
                        lane_u32 SphereMatIndex = LaneU32FromU32(Sphere.MatIndex);
                        
                        // TODO(casey): Reuse the intersection computation to get the normal
                        // out!
                        ConditionalAssign(&HitDistance, HitMask, t);
                        ConditionalAssign(&HitMatIndex, HitMask, SphereMatIndex);
                        ConditionalAssign(&Normal, HitMask, NOZ(t*RayDirection + SphereRelativeRayOrigin));
                        
                        lane_v3 SphereTangent = NOZ(Cross(V3(0, 0, 1), Normal));
                        lane_v3 SphereBinormal = Cross(Normal, SphereTangent);
                        
                        ConditionalAssign(&Tangent, HitMask, Tangent);
                        ConditionalAssign(&Binormal, HitMask, Binormal);
                    }
                }
            }
            
            //
            // TODO(casey): n-way load
            //
            lane_v3 MatEmitColor = LaneMask & GatherV3(World->Materials, HitMatIndex, EmitColor);
            lane_v3 MatRefColor = GatherV3(World->Materials, HitMatIndex, RefColor);
            lane_f32 MatSpecular = GatherF32(World->Materials, HitMatIndex, Specular);
            
            //
            //
            //
            
            Sample += Hadamard(Attenuation, MatEmitColor);
            LaneMask &= (HitMatIndex != LaneU32FromU32(0));
            
            if(MaskIsZeroed(LaneMask))
            {
                break;
            }
            else
            {
                // lane_f32 CosAtten = Max(Inner(-RayDirection, Normal), LaneF32FromF32(0));
                
                RayOrigin += HitDistance*RayDirection;
                
                // TODO(casey): These are not accurate permutations!
                lane_v3 PureBounce = RayDirection - 2.0f*Inner(RayDirection, Normal)*Normal;
                lane_v3 RandomBounce = NOZ(Normal +
                                           LaneV3(RandomBilateral(&Series),
                                                  RandomBilateral(&Series),
                                                  RandomBilateral(&Series)));
                lane_v3 NextRayDirection = NOZ(Lerp(RandomBounce, MatSpecular, PureBounce));
                
                lane_v3 RefC = BRDFLookup(World->Materials, HitMatIndex, -RayDirection, Normal, Tangent, Binormal, NextRayDirection);
                
                Attenuation = Hadamard(Attenuation, RefC);
                
                RayDirection = NextRayDirection;
            }
        }
        
        FinalColor += Contrib*Sample;
    }
    
    State->BouncesComputed += HorizontalAdd(BouncesComputed);
    State->LoopsComputed += LoopsComputed;
    State->FinalColor = HorizontalAdd(FinalColor);
    State->Series = Series;
}

internal b32x
RenderTile(work_queue *Queue)
{
    u64 WorkOrderIndex = LockedAddAndReturnPreviousValue(&Queue->NextWorkOrderIndex, 1);
    if(WorkOrderIndex >= Queue->WorkOrderCount)
    {
        return(false);
    }
    
    work_order *Order = Queue->WorkOrders + WorkOrderIndex;
    
    image_u32 Image = Order->Image;
    u32 XMin = Order->XMin;
    u32 YMin = Order->YMin;
    u32 OnePastXMax = Order->OnePastXMax;
    u32 OnePastYMax = Order->OnePastYMax;
    f32 FilmDist = 1.0f;
    
    lane_v3 CameraP = V3(0, -10, 1);
    lane_v3 CameraZ = NOZ(CameraP);
    lane_v3 CameraX = NOZ(Cross(V3(0, 0, 1), CameraZ));
    lane_v3 CameraY = NOZ(Cross(CameraZ, CameraX));
    lane_v3 FilmCenter = CameraP - FilmDist*CameraZ;
    
    cast_state State;
    
    State.World = Order->World;
    State.RaysPerPixel = Queue->RaysPerPixel;
    State.MaxBounceCount = Queue->MaxBounceCount;
    State.Series = Order->Entropy;
    
    State.CameraP = Extract0(CameraP);
    State.CameraZ = Extract0(CameraZ);
    State.CameraX = Extract0(CameraX);
    State.CameraY = Extract0(CameraY);
    
    State.FilmW = 1.0f;
    State.FilmH = 1.0f;
    if(Image.Width > Image.Height)
    {
        State.FilmH = State.FilmW * ((f32)Image.Height / (f32)Image.Width);
    }
    else if(Image.Height > Image.Width)
    {
        State.FilmW = State.FilmH * ((f32)Image.Width / (f32)Image.Height);
    }
    
    State.HalfFilmW = 0.5f*State.FilmW;
    State.HalfFilmH = 0.5f*State.FilmH;
    State.FilmCenter = Extract0(FilmCenter);
    
    State.HalfPixW = 0.5f / Image.Width;
    State.HalfPixH = 0.5f / Image.Height;
    
    State.BouncesComputed = 0;
    State.LoopsComputed = 0;
    
    for(u32 Y = YMin;
        Y < OnePastYMax;
        ++Y)
    {
        u32 *Out = GetPixelPointer(Image, XMin, Y);
        
        State.FilmY = -1.0f + 2.0f*((f32)Y / (f32)Image.Height);
        for(u32 X = XMin;
            X < OnePastXMax;
            ++X)
        {
            State.FilmX = -1.0f + 2.0f*((f32)X / (f32)Image.Width);
            
            CastSampleRays(&State);
            
            // TODO(casey): Real sRGB here
            f32 R = 255.0f*ExactLinearTosRGB(State.FinalColor.x);
            f32 G = 255.0f*ExactLinearTosRGB(State.FinalColor.y);
            f32 B = 255.0f*ExactLinearTosRGB(State.FinalColor.z);
            f32 A = 255.0f;
            
            u32 BMPValue = ((RoundReal32ToUInt32(A) << 24) |
                            (RoundReal32ToUInt32(R) << 16) |
                            (RoundReal32ToUInt32(G) << 8) |
                            (RoundReal32ToUInt32(B) << 0));
            
            *Out++ = BMPValue; // (Y < 32) ? 0xFFFF0000 : 0xFF0000FF;
        }
    }
    
    LockedAddAndReturnPreviousValue(&Queue->BouncesComputed, State.BouncesComputed);
    LockedAddAndReturnPreviousValue(&Queue->LoopsComputed, State.LoopsComputed);
    LockedAddAndReturnPreviousValue(&Queue->TileRetiredCount, 1);
    
    return(true);
}

internal void
LoadMERLBinary(char *Filename, brdf_table *Dest)
{
    FILE *File = fopen(Filename, "rb");
    if(File)
    {
        fread(Dest->Count, sizeof(Dest->Count), 1, File);
        u32 TotalCount = Dest->Count[0]*Dest->Count[1]*Dest->Count[2];
        u32 TotalReadSize = TotalCount*sizeof(f64)*3;
        u32 TotalTableSize = TotalCount*sizeof(v3);
        f64 *Temp = (f64 *)malloc(TotalReadSize);
        Dest->Values = (v3 *)malloc(TotalTableSize);
        fread(Temp, TotalReadSize, 1, File);
        for(u32 ValueIndex = 0;
            ValueIndex < TotalCount;
            ++ValueIndex)
        {
            Dest->Values[ValueIndex].x = (f32)Temp[ValueIndex];
            Dest->Values[ValueIndex].y = (f32)Temp[TotalCount + ValueIndex];
            Dest->Values[ValueIndex].z = (f32)Temp[2*TotalCount + ValueIndex];
        }
        
        fclose(File);
        free(Temp);
    }
    else
    {
        fprintf(stderr, "Unable to open MERL binary %s.\n", Filename);
    }
}

global v3 NullBRDFValue = {0, 0, 0};
internal void
NullBRDF(brdf_table *Table)
{
    Table->Count[0] = Table->Count[1] = Table->Count[2] = 1;
    Table->Values = &NullBRDFValue;
}

int main(int ArgCount, char **Args)
{
    material Materials[] =
    {
        {    0, {                   }, {0.3f, 0.4f, 0.5f}}, // 0
        {    0, { 0.5f,  0.5f,  0.5f}, {                }}, // 1
        {    0, { 0.7f,  0.5f,  0.3f}, {                }}, // 2
        {    0, {                   }, {50.0f, 10.0f, 5.0f}}, // 3
        { 0.7f, { 0.2f,  0.8f,  0.2f}, {                }}, // 4
        {0.85f, { 0.4f,  0.8f,  0.9f}, {                }}, // 5
        { 1.0f, {0.95f, 0.95f, 0.95f}, {                }}, // 6
    };
    
    NullBRDF(&Materials[0].BRDF);
    LoadMERLBinary("w:/merl/gray-plastic.binary", &Materials[1].BRDF);
    LoadMERLBinary("w:/merl/chrome.binary", &Materials[2].BRDF);
    
    plane Planes[] =
    {
        {{0, 0, 1}, {0}, 1},
#if 0
        {{1, 0, 0}, {2}, 1},
#endif
    };
    
    sphere Spheres[] =
    {
        {{ 0,  0, 0}, 1.0f, 2},
#if 0
        {{ 3, -2, 0}, 1.0f, 3},
        {{-2, -1, 2}, 1.0f, 4},
        {{ 1, -1, 3}, 1.0f, 5},
        {{-2,  3, 0}, 2.0f, 6},
#endif
    };
    
    world World = {};
    World.MaterialCount = ArrayCount(Materials);
    World.Materials = Materials;
    World.PlaneCount = ArrayCount(Planes);
    World.Planes = Planes;
    World.SphereCount = ArrayCount(Spheres);
    World.Spheres = Spheres;
    
    image_u32 Image = AllocateImage(1280, 720);
    
    u32 CoreCount = GetCPUCoreCount();
    u32 TileWidth = Image.Width / CoreCount;
    u32 TileHeight = TileWidth;
    // TODO(casey): It seems like prescribing the tile to be 64x64 was the sweet spot
    // on my mahcine, we'll leave it for now?
    TileWidth = TileHeight = 64;
    
    u32 TileCountX = (Image.Width + TileWidth - 1) / TileWidth;
    u32 TileCountY = (Image.Height + TileHeight - 1) / TileHeight;
    u32 TotalTileCount = TileCountX*TileCountY;
    
    work_queue Queue = {};
    Queue.WorkOrders = (work_order *)malloc(TotalTileCount*sizeof(work_order));
    Queue.MaxBounceCount = 8;
    Queue.RaysPerPixel = 1024;
    
    if(ArgCount == 2)
    {
        Queue.RaysPerPixel = atoi(Args[1]);
    }
    
    printf("Configuration: %d cores with %d %dx%d (%dk/tile) tiles, %d-wide lanes\n",
           CoreCount, TotalTileCount, TileWidth, TileHeight,
           TileWidth*TileHeight*4/1024,
           LANE_WIDTH);
    printf("Quality: %d rays/pixel, %d bounces (max) per ray\n",
           Queue.RaysPerPixel, Queue.MaxBounceCount);
    
    for(u32 TileY = 0;
        TileY < TileCountY;
        ++TileY)
    {
        u32 MinY = TileY*TileHeight;
        u32 OnePastMaxY = MinY + TileHeight;
        if(OnePastMaxY > Image.Height)
        {
            OnePastMaxY = Image.Height;
        }
        
        for(u32 TileX = 0;
            TileX < TileCountX;
            ++TileX)
        {
            u32 MinX = TileX*TileWidth;
            u32 OnePastMaxX = MinX + TileWidth;
            if(OnePastMaxX > Image.Width)
            {
                OnePastMaxX = Image.Width;
            }
            
            work_order *Order = Queue.WorkOrders + Queue.WorkOrderCount++;
            Assert(Queue.WorkOrderCount <= TotalTileCount);
            
            Order->World = &World;
            Order->Image = Image;
            Order->XMin = MinX;
            Order->YMin = MinY;
            Order->OnePastXMax = OnePastMaxX;
            Order->OnePastYMax = OnePastMaxY;
            
            // TODO(casey): Replace this with real entropy!
            random_series Entropy = {LaneU32FromU32(2397458 + TileX*29083 + TileY*97843,
                                                    9878934 + TileX*89243 + TileY*12938,
                                                    6783234 + TileX*29738 + TileY*97853,
                                                    2945085 + TileX*54378 + TileY*89722,
                                                    7653902 + TileX*65422 + TileY*57821,
                                                    5839067 + TileX*89435 + TileY*78120,
                                                    9435786 + TileX*12398 + TileY*32178,
                                                    9604351 + TileX*20789 + TileY*43521)};
            Order->Entropy = Entropy;
        }
    }
    Assert(Queue.WorkOrderCount == TotalTileCount);
    
    // NOTE(casey): This locked add is strictly for fencing, which is not really necessary
    LockedAddAndReturnPreviousValue(&Queue.NextWorkOrderIndex, 0);
    
    clock_t StartClock = clock();
    
    for(u32 CoreIndex = 1;
        CoreIndex < CoreCount;
        ++CoreIndex)
    {
        CreateWorkThread(&Queue);
    }
    
    while(Queue.TileRetiredCount < TotalTileCount)
    {
        if(RenderTile(&Queue))
        {
            fprintf(stderr, "\rRaycasting %d%%...    ",
                    100*(u32)Queue.TileRetiredCount / TotalTileCount);
            fflush(stdout);
        }
    }
    
    clock_t EndClock = clock();
    clock_t TimeElapsed = EndClock - StartClock;
    
    u64 UsedBounces = Queue.BouncesComputed;
    u64 TotalBounces = Queue.LoopsComputed;
    u64 WastedBounces = TotalBounces - UsedBounces;
    
    fprintf(stderr, "\n");
    printf("Raycasting time: %dms\n", TimeElapsed);
    printf("Used bounces: %llu\n", UsedBounces);
    printf("Total bounces: %llu\n", TotalBounces);
    printf("Wasted bounces: %llu (%.02f%%)\n", WastedBounces, 100.0f * (f32)WastedBounces / (f32)TotalBounces);
    printf("Performance: %fms/bounce\n", (f64)TimeElapsed / (f64)Queue.BouncesComputed);
    
    WriteImage(Image, "test.bmp");
    
    fprintf(stderr, "Done.\n");
    
    return(0);
}

#if RAY_WIN32
#include "win32_ray.cpp"
#else
#error "You need to define a platform!"
#endif
