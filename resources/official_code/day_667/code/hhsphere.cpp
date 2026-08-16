/* ========================================================================
   $File: C:\work\handmade\code\hhsphere.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

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
#include "handmade_light_atlas.h"
#include "handmade_stream.cpp"
#include "handmade_image.cpp"
#include "handmade_png.cpp"
#include "handmade_file_formats.cpp"
#include "handmade_light_atlas.cpp"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

struct direction_array
{
    u32 Count;
    v3 *E;
};

internal direction_array AllocDirArray(u32 Count)
{
    direction_array Result;

    Result.Count = Count;
    Result.E = (v3 *)malloc(Count*sizeof(v3));

    return Result;
}

internal direction_array GeneratePoissonDistribution(v2u OctDim, u32 RaysPerTexel)
{
    random_series Series = {1234};

    light_atlas Atlas = MakeLightAtlas({1, 1, 1}, OctDim + V2U(2, 2));

    u32 TotalDirectionCount = OctDim.x*OctDim.y*RaysPerTexel;
    direction_array Directions = AllocDirArray(TotalDirectionCount);
    direction_array Displacements = AllocDirArray(TotalDirectionCount);

    // NOTE(casey): Place points on the sphere, RaysPerTexel randomly per texel
    v2 InvOctDim = {1.0f / OctDim.x, 1.0f / OctDim.y};
    u32 SeedDirIndex = 0;
    for(u32 Ty = 0;
        Ty < OctDim.y;
        ++Ty)
    {
        for(u32 Tx = 0;
            Tx < OctDim.x;
            ++Tx)
        {
            for(u32 RayIndex = 0;
                RayIndex < RaysPerTexel;
                ++RayIndex)
            {
                v2 UV =
                {
                    ((f32)Tx + RandomUnilateral(&Series))*InvOctDim.x,
                    ((f32)Ty + RandomUnilateral(&Series))*InvOctDim.y,
                };

                Directions.E[SeedDirIndex++] = UnitVectorFromOctahedral(2.0f*UV - V2(1, 1));
            }
        }
    }

#if 1
    f32 LastMinMaxSeparation = F32Max;
    for(;;)
    {
        f32 MinClosestPointDistance = F32Max;
        f32 MaxClosestPointDistance = F32Min;

        for(u32 DirIndex = 0;
            DirIndex < TotalDirectionCount;
            ++DirIndex)
        {
            v3 Disp = {};
            v3 Dir = Directions.E[DirIndex];
            f32 ClosestPointDistance = F32Max;

            // TODO(casey): You could make this only do half the
            // iterations by only considering pairs at a time, but I'm not going to bother.
            for(u32 RepIndex = 0;
                RepIndex < TotalDirectionCount;
                ++RepIndex)
            {
                if(DirIndex != RepIndex)
                {
                    v3 Rep = Directions.E[RepIndex];

                    f32 Falloff = Inner(Dir, Rep);
                    if(Falloff > 0)
                    {
                        v3 ForceLine = (Dir - Rep);
                        f32 ForceLineLength = Length(ForceLine);

                        ForceLine = Cross(ForceLine, Dir);
                        ForceLine = Cross(Dir, ForceLine);

                        ForceLine = NOZ(ForceLine);
                        Disp += Falloff*ForceLine;

                        if(ClosestPointDistance > ForceLineLength)
                        {
                            ClosestPointDistance = ForceLineLength;
                        }
                    }
                }
            }

            Displacements.E[DirIndex] = Disp;
            MinClosestPointDistance = Minimum(MinClosestPointDistance, ClosestPointDistance);
            MaxClosestPointDistance = Maximum(MaxClosestPointDistance, ClosestPointDistance);
        }

        // TODO(casey): Determine what the real convergence criteria is.
        f32 MinMaxSeparation = MaxClosestPointDistance - MinClosestPointDistance;
        if(MinMaxSeparation < 0.14f)
        {
            break;
        }

        printf("\rGenerating directions... (%f - %f = %f +%f)            ",
               MaxClosestPointDistance, MinClosestPointDistance, MinMaxSeparation,
               LastMinMaxSeparation - MinMaxSeparation);
        LastMinMaxSeparation = MinMaxSeparation;

        f32 MaxDispPerStep = 0.01f*MaxClosestPointDistance;
        for(u32 DirIndex = 0;
            DirIndex < TotalDirectionCount;
            ++DirIndex)
        {
            v3 Displacement = Displacements.E[DirIndex];
            v3 *Direction = Directions.E + DirIndex;

            v3 Dir = *Direction;

            Dir += MaxDispPerStep*Displacement;
            Dir = NOZ(Dir);

            if(AreEqual(GetOctahedralOffset(Atlas.OctDimCoefficient, *Direction),
                        GetOctahedralOffset(Atlas.OctDimCoefficient, Dir)))
            {
                *Direction = Dir;
            }
        }
    }
    printf("\n");
#endif

    return Directions;
}

internal void
OutputSphereINL(v2u OctahedronDim,
                u32 RaysPerTexel,
                direction_array Directions,
                FILE *File)
{
    fprintf(File, "struct light_sample_direction\n");
    fprintf(File, "{\n");
    fprintf(File, "    v3 RayD;\n");
    fprintf(File, "};\n");
    fprintf(File, "\n");

    fprintf(File, "#define LIGHTING_OCTAHEDRAL_MAP_DIM %u\n", OctahedronDim.x);
    fprintf(File, "#define LIGHTING_OCTAHEDRAL_RAYS_PER_TEXEL %u\n", RaysPerTexel);
    fprintf(File, "#define TOTAL_LIGHT_SAMPLE_DIRECTION_COUNT (LIGHTING_OCTAHEDRAL_RAYS_PER_TEXEL*LIGHTING_OCTAHEDRAL_MAP_DIM*LIGHTING_OCTAHEDRAL_MAP_DIM)\n");

    fprintf(File, "global light_sample_direction SampleDirectionTable[] = \n");
    fprintf(File, "{\n");
    for(u32 DirIndex = 0;
        DirIndex < Directions.Count;
        ++DirIndex)
    {
        v3 Dir = Directions.E[DirIndex];
        fprintf(File, "    {%ff, %ff, %ff},\n", Dir.x, Dir.y, Dir.z);
    }
    fprintf(File, "};\n");
    fprintf(File, "CTAssert(ArrayCount(SampleDirectionTable) == TOTAL_LIGHT_SAMPLE_DIRECTION_COUNT);\n");
    fprintf(File, "\n");
};

int
main(int ArgCount, char **Args)
{
    if(ArgCount == 4)
    {
        u32 OctahedronSide = atoi(Args[1]);
        u32 RaysPerTexel = atoi(Args[2]);
        char *DestFileName = Args[3];

        v2u OctDim = {OctahedronSide, OctahedronSide};

        direction_array Directions = GeneratePoissonDistribution(OctDim, RaysPerTexel);

        FILE *File = fopen(DestFileName, "w");
        if(File)
        {
            OutputSphereINL(OctDim, RaysPerTexel, Directions, File);
        }
        else
        {
            fprintf(stderr, "Unable to open INL %s for writing.\n", DestFileName);
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s <octahedron texel count x (x==y assumed)> <rays per texel> <destination .inl>\n",
                Args[0]);
    }
}
