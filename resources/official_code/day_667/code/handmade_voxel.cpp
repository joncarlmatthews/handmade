/* ========================================================================
   $File: C:\work\handmade\code\handmade_voxel.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

internal v3
GetCellCenterP(voxel_grid *Grid, v3s Index)
{
    v3 Result = Grid->TotalBounds.Min + Grid->CellDim*(V3(Index) + V3(0.5f, 0.5f, 0.5f));
    return(Result);
}

internal rectangle3
GetCellBounds(voxel_grid *Grid, v3s Index)
{
    rectangle3 Result;

    Result.Min = Grid->TotalBounds.Min + Grid->CellDim*V3(Index);
    Result.Max = Result.Min + Grid->CellDim;

    return(Result);
}

internal voxel_relative_p
GetRelativeP(voxel_grid *Grid, v3 WorldP)
{
    voxel_relative_p Result;

    v3 FCoord = Grid->InvCellDim*(WorldP - Grid->TotalBounds.Min);
    Result.Index = FloorToV3S(FCoord);
    v3 BCoord = V3(Result.Index);
    Result.UVW = FCoord - BCoord;

    return(Result);
}

internal v3s
ClampToVoxelDim(voxel_grid *Grid, v3s Index)
{
    v3s Result;
    Result.x = Clamp(0, Index.x, Grid->CellCount.x);
    Result.y = Clamp(0, Index.y, Grid->CellCount.y);
    Result.z = Clamp(0, Index.z, Grid->CellCount.z);
    return Result;
}

internal v3s
GetIndexForP(voxel_grid *Grid, v3 WorldP)
{
    v3s Result = GetRelativeP(Grid, WorldP).Index;

    return(Result);
}

internal b32x
IsInBounds(voxel_grid *Grid, v3s Index)
{
    v3s CellCount = Grid->CellCount;
    b32x Result = ((Index.x >= 0) &&
                   (Index.y >= 0) &&
                   (Index.z >= 0) &&
                   (Index.x < CellCount.x) &&
                   (Index.y < CellCount.y) &&
                   (Index.z < CellCount.z));

    return(Result);
}

internal u32
FlatIndexFrom(voxel_grid *Grid, v3s Index)
{
    Assert(IsInBounds(Grid, Index));
    v3s CellCount = Grid->CellCount;
    u32 Result = CellCount.x*(CellCount.y*Index.z + Index.y) + Index.x;
    return(Result);
}

internal v3s
DimIndexFrom(voxel_grid *Grid, u32 Index)
{
    v3s Result;

    v3s CellCount = Grid->CellCount;

    u32 IndexRemaining = Index;

    u32 ZSheet = CellCount.x*CellCount.y;
    Result.z = IndexRemaining / ZSheet;
    IndexRemaining -= Result.z*ZSheet;

    u32 YRow = CellCount.x;
    Result.y = IndexRemaining / YRow;
    IndexRemaining -= Result.y*YRow;

    Result.x = IndexRemaining;

    return(Result);
}

internal voxel_grid
MakeVoxelGrid(v3s CellCount, v3 CellDim)
{
    voxel_grid Result = {};

    Result.CellDim = CellDim;
    Result.InvCellDim = 1.0f / CellDim;
    Result.CellCount = CellCount;

    return(Result);
}

internal void
SetMinCorner(voxel_grid *Grid, v3 MinCorner)
{
    Grid->TotalBounds = RectMinDim(MinCorner, V3(Grid->CellCount)*Grid->CellDim);
}

internal v3
GetMinCorner(voxel_grid *Grid)
{
    v3 Result = Grid->TotalBounds.Min;
    return(Result);
}

internal u32
GetTotalCellCount(voxel_grid *Grid)
{
    v3s CellCount = Grid->CellCount;
    u32 Result = (CellCount.x*CellCount.y*CellCount.z);

    return(Result);
}