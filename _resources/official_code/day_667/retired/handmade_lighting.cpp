internal void
FullCast(lighting_work *Work, light_atlas_texel Tile, v3 LightProbeP, v3s AtlasIndex)
{
    lighting_solution *Solution = Work->Solution;
    light_atlas *SpecAtlas = Work->SpecularLightAtlas;
    light_atlas *DiffuseAtlas = Work->DiffuseLightAtlas;

    v3_4x RayOrigin = V3_4x(LightProbeP, LightProbeP, LightProbeP, LightProbeP);

    v3s InitialGridI = GetIndexForP(&Solution->SpatialGrid, LightProbeP);
    Assert(IsInBounds(&Solution->SpatialGrid, InitialGridI));
    // TODO(casey): Switch all this to 32-bit?
    u16 InitialGridIndex = (u16)FlatIndexFrom(&Solution->SpatialGrid, InitialGridI);

#if 0
    f32_4x IsInside = ZeroF32_4x();
    lighting_spatial_grid_node Node = Solution->SpatialGridNodes[InitialGridIndex];
    for(u16 LeafIndexIndex = Node.StartIndex;
        LeafIndexIndex < Node.OnePastLastIndex;
        ++LeafIndexIndex)
    {
        lighting_spatial_grid_leaf Leaf = Solution->SpatialGridLeaves[LeafIndexIndex];

        v3_4x BoxMin = Leaf.BoxMin;
        v3_4x BoxMax = Leaf.BoxMax;
        f32_4x IsEmission = Leaf.IsEmission;

        f32_4x InBoundsX = (RayOrigin.x > BoxMin.x) & (RayOrigin.x < BoxMax.x);
        f32_4x InBoundsY = (RayOrigin.y > BoxMin.y) & (RayOrigin.y < BoxMax.y);
        f32_4x InBoundsZ = (RayOrigin.z > BoxMin.z) & (RayOrigin.z < BoxMax.z);
        f32_4x IsOccluder = (IsEmission == ZeroF32_4x());
        f32_4x Mask = InBoundsX & InBoundsY & InBoundsZ & IsOccluder;

        IsInside |= Mask;
    }

    if(AnyTrue(IsInside))
    {
        // TODO(casey): This is buggy because you can be inside a light source, and in
        // that case, clearly the right answer is not that you don't have light.  You
        // should at least set your value to the emission value.

        // TODO(casey): Handle lerping towards zero in case we ever have moving occluders.

#if 0
        for(u32 Sy = 0;
            Sy < (SpecAtlas->TileDim.y - 2);
            ++Sy)
        {
            light_atlas_texel SpecTexel = OffsetFromTexel(SpecAtlas, Tile, 1, Sy+1);

#if 0
            f32_4x S0 = F32_4x(1.0f, 0.0f, 1.0f,  1.0f);
            f32_4x S1 = F32_4x(0.0f, 1.0f,  1.0f, 0.0f);
            f32_4x S2 = F32_4x(1.0f,  1.0f, 0.0f, 1.0f);
            f32_4x S3 = F32_4x(1.0f, 0.0f, 1.0f,  1.0f);
            f32_4x S4 = F32_4x(0.0f, 1.0f,  1.0f, 0.0f);
            f32_4x S5 = F32_4x(1.0f,  1.0f, 0.0f, 1.0f);
#else
            f32_4x S0, S1, S2, S3, S4, S5;
            S0 = S1 = S2 = S3 = S4 = S5 = {};
#endif

            StoreF32_4X(S0, (f32_4x *)SpecTexel.Value + 0);
            StoreF32_4X(S1, (f32_4x *)SpecTexel.Value + 1);
            StoreF32_4X(S2, (f32_4x *)SpecTexel.Value + 2);
            StoreF32_4X(S3, (f32_4x *)SpecTexel.Value + 3);
            StoreF32_4X(S4, (f32_4x *)SpecTexel.Value + 4);
            StoreF32_4X(S5, (f32_4x *)SpecTexel.Value + 5);

        }
#endif
    }
    else
#endif
    {
#if GRID_RAY_CAST_DEBUGGING
        debug_ray_pick Pick = {};
        if(AreEqual(Solution->DebugPick.AtlasIndex, AtlasIndex))
        {
            Solution->DebugPick.AtlasIndex = Clamp({}, Solution->DebugPick.AtlasIndex, Solution->AtlasGrid.CellCount);
            Solution->DebugPick.Tx = Clamp(0, Solution->DebugPick.Tx, 7);
            Solution->DebugPick.Ty = Clamp(0, Solution->DebugPick.Ty, 7);
            Solution->DebugPick.RayIndex = Clamp(0, Solution->DebugPick.RayIndex,
                                                 LIGHTING_OCTAHEDRAL_RAYS_PER_TEXEL - 1);
            v3 ExpectedDirection = DirectionFromTxTy(SpecAtlas->OxyCoefficient,
                                                     Solution->DebugPick.Tx + 1,
                                                     Solution->DebugPick.Ty + 1);

            PushDebugLine(Solution, LightProbeP, LightProbeP + ExpectedDirection, V4(1, 0, 1, 1));
            Pick = Solution->DebugPick;
        }
#endif
        GridRayCast(Work, LightProbeP, SpecAtlas, DiffuseAtlas, Tile, InitialGridIndex
#if GRID_RAY_CAST_DEBUGGING
                    , Pick
#endif
                    );
    }
}
