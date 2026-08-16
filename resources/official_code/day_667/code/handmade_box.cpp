/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal v2s GetDirection(box_surface_index Dir)
{
    v2s Result = {};
    box_surface_params Params = GetBoxSurfaceParams(Dir);
    Result.E[Params.AxisIndex] = (Params.Positive ? 1 : -1);
    return(Result);
}

internal u32
GetOtherSide(u32 Side)
{
    u32 Result = ((Side + 1) % 2);
    return(Result);
}

internal box_surface_params
GetBoxSurfaceParams(u32 SurfaceIndex)
{
    box_surface_params Result;
    
    Result.AxisIndex = (SurfaceIndex >> 1);
    Result.Positive = SurfaceIndex & 0x1;
    
    return(Result);
}

internal box_surface_index
GetSurfaceIndex(u32 AxisIndex, u32 Positive)
{
    Assert(Positive <= 1);
    Assert(AxisIndex <= 2);
    
    box_surface_index Result = (box_surface_index)((AxisIndex << 1) | Positive);
    return(Result);
}

internal u32 
GetSurfaceMask(u32 AxisIndex, u32 Positive)
{
    u32 Result = (1 << GetSurfaceIndex(AxisIndex, Positive));
    return(Result);
}

internal u32
GetSurfaceMask(box_surface_index Index)
{
    u32 Result = (1 << Index);
    return(Result);
}

internal light_box_surface
GetBoxSurface(v3 P, v3 Radius, u32 SurfaceIndex)
{
    box_surface_params Params = GetBoxSurfaceParams(SurfaceIndex);
    u32 AxisIndex = Params.AxisIndex;
    u32 Positive = Params.Positive;
    
    v3 N = V3(0, 0, 0);
    v3 YAxis = (AxisIndex == 2) ? V3(0, 1, 0) : V3(0, 0, 1);
    
    if(Positive)
    {
        N.E[AxisIndex] = 1.0f;
        P.E[AxisIndex] += Radius.E[AxisIndex];
    }
    else
    {
        N.E[AxisIndex] = -1.0f;
        P.E[AxisIndex] -= Radius.E[AxisIndex];
    }
    
    f32 SignX = Positive ? 1.0f : -1.0f;
    if(AxisIndex == 1)
    {
        SignX *= -1.0f;
    }
    v3 XAxis = (AxisIndex == 0) ? V3(0, SignX, 0) : V3(SignX, 0, 0);
    
    f32 HalfWidth = AbsoluteValue(Inner(XAxis, Radius));
    f32 HalfHeight = AbsoluteValue(Inner(YAxis, Radius));
    
    light_box_surface Result;
    Result.P = P;
    Result.N = N;
    Result.XAxis = XAxis;
    Result.YAxis = YAxis;
    Result.HalfWidth = HalfWidth;
    Result.HalfHeight = HalfHeight;
    
    return(Result);
}

internal u32
GetBoxMaskComplement(u32 BoxMask)
{
    u32 BitMask = ((1 << 0) |
                   (1 << 2) |
                   (1 << 4));
    u32 Just024 = BoxMask & BitMask;
    u32 Just135 = BoxMask & (BitMask << 1);
    
    u32 Result = ((Just024 << 1) |
                  (Just135 >> 1));
    
    return(Result);
}

internal box_surface_index
GetBoxIndexFromDirMask(u32 DirMask)
{
    bit_scan_result Scan = FindLeastSignificantSetBit(DirMask);
    Assert(Scan.Found);
    Assert((Scan.Index >= BoxIndex_West) && (Scan.Index < BoxIndex_Count));
    
    box_surface_index Result = (box_surface_index)Scan.Index;
    Assert(GetSurfaceMask(Result) == DirMask);
    
    return(Result);
}
