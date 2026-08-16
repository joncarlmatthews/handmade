/* ========================================================================
   $File: C:\work\handmade\code\handmade_voxel.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

struct voxel_grid
{
    rectangle3 TotalBounds;
    v3 CellDim;
    v3 InvCellDim;
    
    v3s CellCount;
};

struct voxel_relative_p
{
    v3s Index;
    v3 UVW;
};

internal v3 GetCellCenterP(voxel_grid *Grid, v3s Index);
internal rectangle3 GetCellBounds(voxel_grid *Grid, v3s Index);
internal voxel_relative_p GetRelativeP(voxel_grid *Grid, v3 WorldP);
internal v3s GetIndexForP(voxel_grid *Grid, v3 WorldP);
internal u32 GetTotalCellCount(voxel_grid *Grid);
