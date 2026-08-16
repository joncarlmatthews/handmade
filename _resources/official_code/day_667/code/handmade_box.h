/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/* NOTE(casey):

   A P    XAXIS     YAXIS
   0 0   0,-1, 0   0, 0, 1
   0 1   0, 1, 0   0, 0, 1
   1 0   1, 0, 0   0, 0, 1
   1 1  -1, 0, 0   0, 0, 1
   2 0  -1, 0, 0   0, 1, 0
   2 1   1, 0, 0   0, 1, 0
*/
enum box_surface_index
{
    BoxIndex_West,
    BoxIndex_East,
    BoxIndex_South,
    BoxIndex_North,
    
    BoxIndex_Count,
};

enum box_surface_mask
{
    BoxMask_West = (1 << BoxIndex_West),
    BoxMask_East = (1 << BoxIndex_East),
    BoxMask_South = (1 << BoxIndex_South),
    BoxMask_North = (1 << BoxIndex_North),
    
    BoxMask_Planar = (BoxMask_West |
                      BoxMask_East |
                      BoxMask_South |
                      BoxMask_North),
};

struct box_surface_params
{
    u32 AxisIndex;
    u32 Positive;
};

struct light_box_surface
{
    v3 P;
    v3 N;
    v3 XAxis;
    v3 YAxis;
    f32 HalfWidth;
    f32 HalfHeight;
};

internal u32 GetOtherSide(u32 Side);
internal box_surface_params GetBoxSurfaceParams(u32 SurfaceIndex);
internal box_surface_index GetSurfaceIndex(u32 AxisIndex, u32 Positive);
internal u32 GetSurfaceMask(u32 AxisIndex, u32 Positive);
internal u32 GetSurfaceMask(box_surface_index Index);
internal light_box_surface GetBoxSurface(v3 P, v3 Radius, u32 SurfaceIndex);
internal u32 GetBoxMaskComplement(u32 BoxMask);
