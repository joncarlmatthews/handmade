/* ========================================================================
   $File: C:\work\handmade\code\hhlightprof.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

#undef HANDMADE_INTERNAL
#define HANDMADE_INTERNAL 0

#undef HANDMADE_SLOW
#define HANDMADE_SLOW 0

#include "handmade_platform.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_shared.h"
#include "handmade_memory.h"
#include "handmade_stream.h"
#include "handmade_image.h"
#include "handmade_png.h"
#include "handmade_file_formats.h"
#include "handmade_simd.h"
#include "handmade_random.h"
#include "handmade_voxel.h"
#include "handmade_light_atlas.h"
#include "handmade_stream.cpp"
#include "handmade_image.cpp"
#include "handmade_png.cpp"
#include "handmade_file_formats.cpp"
#include "handmade_voxel.cpp"
#include "handmade_light_atlas.cpp"
#include "handmade_renderer.h"
#include "handmade_renderer.cpp"
#include "handmade_world.h"
#include "handmade_lighting.h"
#include "handmade_config.h"

#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#define RECORD_RAYCAST_STACK 0

#if RECORD_RAYCAST_STACK
global FILE *LeafBoxOut = 0;
global FILE *PartitionBoxOut = 0;
global u32 StackDumpSwitch = 16;
#define RECORD_LEAF_BOX(BoxIndex) if(StackDumpSwitch) {fprintf(LeafBoxOut, "%u\n", BoxIndex);}
#define RECORD_PARTITION_BOX(BoxIndex) if(StackDumpSwitch) {fprintf(PartitionBoxOut, "%u\n", BoxIndex);}
#define RECORD_RAYCAST_END(...) if(StackDumpSwitch > 0) {--StackDumpSwitch; fprintf(PartitionBoxOut, "\n");}
#define RECORD_PARTITION_PUSH(ShouldPush, Pack) if(StackDumpSwitch && ShouldPush) {fprintf(PartitionBoxOut, "  + %u->%u\n", Pack.FirstChildIndex, Pack.FirstChildIndex + Pack.ChildCount);}
#endif

#define DevMode_lighting 0

platform_api Platform = {};

internal v3
Subtract(world *World, world_position *A, world_position *B)
{
    // NOTE(casey): Stubbed for lighting system linkage
    v3 Result = {};
    return(Result);
}

internal bool32
AreInSameChunk(world *World, world_position *A, world_position *B)
{
    // NOTE(casey): Stubbed for lighting system linkage
    bool32 Result = false;
    return(Result);
}

internal void
DEBUGDumpData(char *FileName, umm DumpSize, void *DumpData)
{
    // NOTE(casey): Stubbed for lighting system linkage
}

#include "handmade_lighting.cpp"

internal buffer
LoadEntireFile(char *Filename, umm ExtraSize = 0)
{
    buffer Result = {};
    
    FILE *File = fopen(Filename, "rb");
    if(File)
    {
        fseek(File, 0, SEEK_END);
        Result.Count = ftell(File);
        fseek(File, 0, SEEK_SET);
        
        Result.Data = (u8 *)malloc(Result.Count + ExtraSize);
        fread(Result.Data, Result.Count, 1, File);
        
        fclose(File);
    }
    else
    {
        fprintf(stderr, "Unable to open dump %s for reading.\n", Filename);
    }
    
    return(Result);
}

internal void
ProfileRun(u32 RepeatCount)
{
    buffer SourceLighting = LoadEntireFile("source_lighting.dump");
    buffer SourceLightBoxes = LoadEntireFile("source_lightboxes.dump");
    buffer SourceSpecAtlas = LoadEntireFile("source_specatlas.dump");
    buffer SourceDiffuseAtlas = LoadEntireFile("source_diffuseatlas.dump");
    buffer ResultSpecAtlas = LoadEntireFile("result_specatlas.dump");
    buffer ResultDiffuseAtlas = LoadEntireFile("result_diffuseatlas.dump");
    
    if(SourceLighting.Count &&
       SourceLightBoxes.Count &&
       SourceSpecAtlas.Count &&
       SourceDiffuseAtlas.Count &&
       ResultSpecAtlas.Count &&
       ResultDiffuseAtlas.Count)
    {
        lighting_solution *Solution = (lighting_solution *)SourceLighting.Data;
        Solution->SamplingSpheres = LightSamplingSphereTable;
        Solution->UpdateDebugLines = false;
        
        {
            v3 VoxCellDim = 2.0f*Solution->AtlasGrid.CellDim;
            v3s SpatialCellCount = (Solution->AtlasGrid.CellCount/2) + V3S(2, 2, 2);
            
            Solution->AtlasToSpatialGridIndexOffset = V3S(1, 1, 1);
            
            v3 AtlasMinCorner = GetMinCorner(&Solution->AtlasGrid);
            v3 SpatialMinCorner = (AtlasMinCorner - VoxCellDim);
            
            Solution->SpatialGrid = MakeVoxelGrid(SpatialCellCount, VoxCellDim);
            SetMinCorner(&Solution->SpatialGrid, SpatialMinCorner);
            
            Solution->MaxCostPerRay = 8;
        }
        
        u32 AllocWorkCount = MAX_LIGHT_LOOKUP_VOXEL_DIM;
        u32 SourceLightBoxCount = (u32)(SourceLightBoxes.Count / sizeof(lighting_box));
        
        v3s LightAtlasVoxelDim = V3S(LIGHT_LOOKUP_VOXEL_DIM_X, LIGHT_LOOKUP_VOXEL_DIM_Y, LIGHT_LOOKUP_VOXEL_DIM_Z);
        u32 LIGHT_COLOR_LOOKUP_SQUARE_DIM = (8+2);
        v2u LightAtlasTileDim = V2U(LIGHT_COLOR_LOOKUP_SQUARE_DIM, LIGHT_COLOR_LOOKUP_SQUARE_DIM);
        light_atlas DiffuseAtlas = MakeLightAtlas(LightAtlasVoxelDim, LightAtlasTileDim);
        light_atlas SpecAtlas = MakeLightAtlas(LightAtlasVoxelDim, LightAtlasTileDim);
        
        SetLightAtlasTexels(&DiffuseAtlas, SourceDiffuseAtlas.Data);
        SetLightAtlasTexels(&SpecAtlas, SourceSpecAtlas.Data);
        
        {
            for(u32 Sy = 0;
                Sy < 8;
                ++Sy)
            {
                for(u32 Sx = 0;
                    Sx < 8;
                    ++Sx)
                {
                    f32 WTotal = 0;
                    f32 Threshold = 0.025f;
                    
                    fprintf(stdout, "Map[%u][%u] = \n", Sy, Sx);
                    
                    // TODO(casey): If we really cared, this total weight distribution could
                    // probably be shown to be the same for all texels, so we could just
                    // compute it once rather than doing it for each spec texel location.
                    for(u32 Ty = 0;
                        Ty < 8;
                        ++Ty)
                    {
                        for(u32 Tx = 0;
                            Tx < 8;
                            ++Tx)
                        {
                            v3 Outgoing = DirectionFromTxTy(DiffuseAtlas.OxyCoefficient, Tx + 1, Ty + 1);
                            v3 Incoming = DirectionFromTxTy(SpecAtlas.OxyCoefficient, Sx + 1, Sy + 1);
                            f32 W = Clamp01(Inner(Outgoing, Incoming));
                            WTotal += W;
                        }
                    }
                    
                    for(u32 Ty = 0;
                        Ty < 8;
                        ++Ty)
                    {
                        for(u32 Tx = 0;
                            Tx < 8;
                            ++Tx)
                        {
                            v3 Outgoing = DirectionFromTxTy(DiffuseAtlas.OxyCoefficient, (Tx % 8) + 1, (Ty % 8) + 1);
                            v3 Incoming = DirectionFromTxTy(SpecAtlas.OxyCoefficient, Sx + 1, Sy + 1);
                            f32 W = Clamp01(Inner(Outgoing, Incoming));
                            f32 Entry = W / WTotal;
                            if(Entry > Threshold)
                            {
                                fprintf(stdout, "%.03f ", Entry);
                            }
                            else
                            {
                                fprintf(stdout, "----- ");
                            }
                        }
                        
                        fprintf(stdout, "\n");
                    }
                    
                    fprintf(stdout, "\n");
                }
            }
            
            for(u32 ElementIndex = 0;
                ElementIndex < 3;
                ++ElementIndex)
            {
                fprintf(stdout, "Element[%u]:\n", ElementIndex);
                for(u32 Sy = 0;
                    Sy < 8;
                    ++Sy)
                {
                    for(u32 Sx = 0;
                        Sx < 8;
                        ++Sx)
                    {
                        v3 Incoming = DirectionFromTxTy(SpecAtlas.OxyCoefficient, Sx + 1, Sy + 1);
                        f32 Entry = Incoming.E[ElementIndex];
                        if(Entry)
                        {
                            fprintf(stdout, "%+.03f ", Entry);
                        }
                        else
                        {
                            fprintf(stdout, "------ ");
                        }
                    }
                    
                    fprintf(stdout, "\n");
                }
                
                fprintf(stdout, "\n");
            }
            
        }
        
        if(RepeatCount == 1)
        {
            fprintf(stderr, "Total input boxes: %u\n", SourceLightBoxCount);
        }
        
        memory_arena TempArena = {};
        platform_memory_block TempBlock = {};
        TempBlock.Size = Megabytes(16);
        TempBlock.Base = (u8 *)malloc(TempBlock.Size);
        TempArena.CurrentBlock = &TempBlock;
        
        Solution->SampleDirectionTable = PushArray(&TempArena, ArrayCount(SampleDirectionTable), light_sample_direction);
        ComputeWalkTable(&TempArena, Solution->SpatialGrid.CellCount,
                         Solution->MaxCostPerRay, Solution->SpatialGrid.CellDim, 
                         ArrayCount(SampleDirectionTable),
                         SampleDirectionTable,
                         Solution->SampleDirectionTable,
                         Solution->LightSamplingWalkTable);
        
        
        LARGE_INTEGER StartTime, EndTime;
        QueryPerformanceCounter(&StartTime);
        lighting_stats Stats = InternalLightingCore(Solution, &SpecAtlas, &DiffuseAtlas, 0, &TempArena, SafeTruncateToU16(SourceLightBoxCount), (lighting_box *)SourceLightBoxes.Data, RepeatCount);
        QueryPerformanceCounter(&EndTime);
        
        LARGE_INTEGER Freq;
        QueryPerformanceFrequency(&Freq);
        
        u64 TotalCastsInitiated = Stats.TotalCastsInitiated;
        u64 TotalPartitionsTested = Stats.TotalPartitionsTested;
        u64 TotalLeavesTested = Stats.TotalLeavesTested;
        u64 TotalPartitionLeavesUsed = Stats.TotalPartitionLeavesUsed;
        
        if(RepeatCount == 1)
        {
            f32 *ExpectedTexels = (f32 *)ResultSpecAtlas.Data;
            f32 *GotTexels = (f32 *)GetLightAtlasTexels(&SpecAtlas);
            umm TexelCount = GetLightAtlasTexelCount(&SpecAtlas);
            
            f32 MaxError = 0;
            f32 TotalError = 0;
            umm Count = TexelCount;
            while(Count--)
            {
                f32 Expected = *ExpectedTexels++;
                f32 Got = *GotTexels++;
                
                f32 GotError = AbsoluteValue(Expected - Got);
                
                if(MaxError < GotError)
                {
                    MaxError = GotError;
                }
                
                TotalError += GotError;
            }
            
            u32 TotalGridNodeCount = GetTotalCellCount(&Solution->SpatialGrid);
            u32 NodesPerBoxCount[64] = {};
            for(u32 NodeIndex = 0;
                NodeIndex < TotalGridNodeCount;
                ++NodeIndex)
            {
                lighting_spatial_grid_node *Node = Solution->SpatialGridNodes + NodeIndex;
                u32 BoxCount = 0;
                for(u16 LeafIndexIndex = Node->StartIndex;
                    LeafIndexIndex < Node->OnePastLastIndex;
                    ++LeafIndexIndex)
                {
                    lighting_spatial_grid_leaf Leaf = 
                        Solution->SpatialGridLeaves[LeafIndexIndex];
                    f32_4x Valid = (Leaf.BoxMin.x != Leaf.BoxMax.x);
                    u32 ValidCount = __popcnt(_mm_movemask_ps(Valid.P));
                    BoxCount += ValidCount;
                }
                
                BoxCount = Minimum(ArrayCount(NodesPerBoxCount) - 1, BoxCount);
                ++NodesPerBoxCount[BoxCount];
            }
            
            for(u32 BoxCount = 0;
                BoxCount < ArrayCount(NodesPerBoxCount);
                ++BoxCount)
            {
                if(NodesPerBoxCount[BoxCount])
                {
                    fprintf(stderr, "BoxCount[%u]: %u\n", BoxCount, NodesPerBoxCount[BoxCount]);
                }
            }
            
            fprintf(stderr, "Total texels: %f\n", (f32)TexelCount);
            fprintf(stderr, "Total error: %f\n", TotalError);
            fprintf(stderr, "Total error/texel: %f\n", TotalError / (f32)TexelCount);
            fprintf(stderr, "Max error/texel: %f\n", MaxError);
        }
        else
        {
            double Diff = (double)(EndTime.QuadPart - StartTime.QuadPart);
            double TotalSeconds = Diff / (double)Freq.QuadPart;
            fprintf(stdout, "T:%.0f P:%.0f L:%.0f P/L:%.2f\n",
                    (double)TotalPartitionsTested + (double)TotalLeavesTested,
                    (double)TotalPartitionsTested, (double)TotalLeavesTested,
                    (double)TotalPartitionsTested / (double)TotalLeavesTested);
            fprintf(stdout, "P/ray:%.0f L/ray:%.0f\n",
                    (double)TotalPartitionsTested / (double)TotalCastsInitiated,
                    (double)TotalLeavesTested / (double)TotalCastsInitiated);
            fprintf(stdout, "Total Seconds Elapsed: %f\n", TotalSeconds);
            fprintf(stdout, "Expected ms/thread: %.02f\n", (1000.0*(TotalSeconds / (double)RepeatCount)) / 6.0);
        }
    }
    else
    {
        fprintf(stderr, "Unable to run raycast test due to missing data.\n");
    }
}

int
main(int ArgCount, char **Args)
{
    if(ArgCount == 1)
    {
#if RECORD_RAYCAST_STACK
        LeafBoxOut = fopen("leaf_box_traversal.txt", "w");
        PartitionBoxOut = fopen("partition_box_traversal.txt", "w");
#endif
        
        ProfileRun(1);
#if !RECORD_RAYCAST_STACK
        ProfileRun(60*2);
#endif
        
#if RECORD_RAYCAST_STACK
        fclose(LeafBoxOut);
        fclose(PartitionBoxOut);
#endif
    }
    else
    {
        fprintf(stderr, "Usage: %s\n", Args[0]);
    }
}
