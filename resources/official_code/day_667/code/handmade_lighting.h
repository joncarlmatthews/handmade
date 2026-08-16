/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define GRID_RAY_CAST_DEBUGGING 0
#define LIGHTS_ARE_SPHERES 0

/*

Grid: 7.479611s
Grid + Optimized ComputeIrradiance: 5.110175
Grid + Optimized ComputeIrradiance 2: 5.055217
Grid + Welded ComputeIrradiance: 4.701094
Grid + Streamlined ComputeIrradiance: 4.513986
2x spatial grid size: 4.256591

AABB: 7.287836s 
AABB + Optimized ComputeIrradiance: 6.390334

NONE: 1.246065s
NONE: 1.028882

Grid bookends only: 4.679288s
Grid bookends only + Optimized ComputeIrradiance: 2.583887

*/

struct debug_line
{
    v3 FromP;
    v3 ToP;
    v4 Color;
};

struct debug_ray_pick
{
    b32 Enabled;
    v3s AtlasIndex;
    u32 Tx, Ty, RayIndex;
};

struct lighting_stats
{
    u32 TotalCastsInitiated; // NOTE(casey): Number of attempts to raycast from a point
    u32 TotalPartitionsTested; // NOTE(casey): Number of partition boxes checked
    u32 TotalPartitionLeavesUsed; // NOTE(casey): Number of partition boxes used as leaves
    u32 TotalLeavesTested; // NOTE(casey): Number of leaf boxes checked
};

struct lighting_work
{
    struct lighting_solution *Solution;
    light_atlas *DiffuseLightAtlas;
    light_atlas *SpecularLightAtlas;
    void *Reserved;
    
    u32 VoxelY;
    u32 SamplePointEntropy;
    
    u32 TotalCastsInitiated; // NOTE(casey): Number of attempts to raycast from a point
    u32 TotalPartitionsTested; // NOTE(casey): Number of partition boxes checked
    u32 TotalPartitionLeavesUsed; // NOTE(casey): Number of partition boxes used as leaves
    u32 TotalLeavesTested; // NOTE(casey): Number of leaf boxes checked
    
    u8 SmallPad[4];
};
CTAssert(sizeof(lighting_work) == 64);

struct diffuse_weight_map
{
    f32_4x E[16][16];
};

#define SPATIAL_GRID_NODE_TERMINATOR 0xffff
struct lighting_spatial_grid_node
{
    // TODO(casey): Pack this as a 32-bit value that's a ~24-bit offset with a 8-bit count,
    // this would scale to much larger scenes more easily.
    u16 StartIndex;
    u16 OnePastLastIndex;
};

struct lighting_spatial_grid_leaf
{
    v3_4x BoxMin;
    v3_4x BoxMax;
    v3_4x RefColor;
    f32_4x IsEmission;
};

struct lighting_solution
{
    // NOTE(casey): Stuff that is automatically regenerated any time it needs to be
    
    memory_arena TableMemory;
    f32 FundamentalUnit;
    u32 MaxCostPerRay;
    
    voxel_grid AtlasGrid;
    voxel_grid SpatialGrid;
    
    diffuse_weight_map DiffuseWeightMap[16][16];
    
    f32 tUpdateBlend;
    
    // NOTE(casey): Stuff that carries from frame to frame
    
    v3s VoxCameraOffset;
    world_position LastOriginP;
    random_series Series;
    
    // NOTE(casey): Stuff that only exists during the solve
    
    // TODO(casey): Wrap these in their own thing
    // so we don't leave them in the lighting solution
    // since they're allocated in the temp arena
    // anyway?
    lighting_spatial_grid_node *SpatialGridNodes;
    lighting_spatial_grid_leaf *SpatialGridLeaves;
    
    b32x UpdateDebugLines;
    u32x DebugBoxDrawDepth;
    u32x DebugProbeIndex;
    u32x MaxDebugLineCount;
    u32x DebugLineCount;
    debug_line *DebugLines;
    
    // NOTE(casey): Parameters to the system
    
    v3 DebugLightP;
    
    debug_ray_pick DebugPick;
};

// TODO(casey): Split the lighting state into pieces
// so we can have a more "functional-esque" way of calling it
// where the two halves specify the persistent and new state
// so we can save them both easily in we want.
struct lighting_update_params
{
    f32 FundamentalUnit;
    v3 ChunkDimInMeters;
    world_position SimOriginP;
    world_position OriginP;
};

internal void UpdateLighting(lighting_solution *Solution, f32 FundamentalUnit,
                             light_atlas *SpecAtlas, light_atlas *DiffuseAtlas,
                             world *World, world_position SimOriginP, world_position OriginP,
                             u16 OccluderCount, lighting_box *Occluders,
                             platform_work_queue *LightingQueue, memory_arena *TempArena);
internal void PushLightingRenderValues(lighting_solution *Solution, render_group *Group);
