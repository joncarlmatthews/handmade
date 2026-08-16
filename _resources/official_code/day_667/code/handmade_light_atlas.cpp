/* ========================================================================
   $File: C:\work\handmade\code\handmade_light_atlas.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

internal b32x
IsFloat(light_atlas *Atlas)
{
    b32x Result = LIGHT_ATLAS_IN_FLOAT;
    return(Result);
}

internal u32
GetLightAtlasTexelSize(light_atlas *Atlas)
{
    u32 Result = 3;
    
    if(IsFloat(Atlas))
    {
        Result *= sizeof(f32);
    }
    else
    {
        Result *= sizeof(u8);
    }
    
    return(Result);
}

internal u32
GetLightAtlasWidth(light_atlas *Atlas)
{
    u32 LightAtlasWidth = Atlas->VoxelDim.x*Atlas->VoxelDim.y*Atlas->TileDim.x;
    return(LightAtlasWidth);
}

internal u32
GetLightAtlasHeight(light_atlas *Atlas)
{
    u32 LightAtlasHeight = Atlas->VoxelDim.z*Atlas->TileDim.y;
    return(LightAtlasHeight);
}

internal umm
GetLightAtlasTexelCount(light_atlas *Atlas)
{
    umm Result = GetLightAtlasWidth(Atlas)*GetLightAtlasHeight(Atlas);
    return(Result);
}

internal umm
GetLightAtlasSize(light_atlas *Atlas)
{
    umm Result = GetLightAtlasTexelCount(Atlas)*GetLightAtlasTexelSize(Atlas);
    return(Result);
}

internal light_atlas
MakeLightAtlas(v3s VoxelDim, v2u TileDim)
{
    light_atlas Result = {};
    
    Result.VoxelDim = VoxelDim;
    Result.TileDim = TileDim;
    
    Result.OctDimCoefficient = V2((f32)(Result.TileDim.x - 3), (f32)(Result.TileDim.y - 3));
    Result.OxyCoefficient = V2(1.0f / Result.OctDimCoefficient.x,
                               1.0f / Result.OctDimCoefficient.y);
    
    Result.TileRowStride = Result.VoxelDim.x*Result.VoxelDim.y*Result.TileDim.x;
    
    Result.StrideXYZ_4x[0] = (f32)(Result.TileDim.x);
    Result.StrideXYZ_4x[1] = (f32)(Result.VoxelDim.x*Result.TileDim.x);
    Result.StrideXYZ_4x[2] = (f32)(Result.TileRowStride*Result.TileDim.y);
    Result.StrideXYZ_4x[3] = 0;
    
    Result.VoxelDim_4x[0] = (f32)Result.VoxelDim.x;
    Result.VoxelDim_4x[1] = (f32)Result.VoxelDim.y;
    Result.VoxelDim_4x[2] = (f32)Result.VoxelDim.z;
    Result.VoxelDim_4x[3] = 0;
    
    return(Result);
}

internal void
SetLightAtlasTexels(light_atlas *Atlas, void *Memory)
{
    Atlas->Texels = (u8 *)Memory;
    
    VALIDATE_TEXEL_ATLAS(Atlas);
}

internal void *
GetLightAtlasTexels(light_atlas *Atlas)
{
    return(Atlas->Texels);
}

internal b32x
PointerIsInBounds(light_atlas *Atlas, light_atlas_texel Texel)
{
    u8 *Check = (u8 *)Texel.Value;
    u8 *EndPtr = Atlas->Texels + GetLightAtlasSize(Atlas);
    
    b32x Result = ((Check >= Atlas->Texels) &&
                   (Check < EndPtr));
    return(Result);
}

internal b32 VoxelIndexIsInDim(light_atlas *Atlas, v3s Index)
{
    b32 Result = ((Index.x >= 0) &&
                  (Index.x < Atlas->VoxelDim.x) &&
                  (Index.y >= 0) &&
                  (Index.y < Atlas->VoxelDim.y) &&
                  (Index.z >= 0) &&
                  (Index.z < Atlas->VoxelDim.z));
    return Result;
}

internal light_atlas_texel
GetTileUnclamped(light_atlas *Atlas, s32 X, s32 Y, s32 Z, u32 Tx, u32 Ty)
{
    LIGHT_ATLAS_ASSERT(VoxelIndexIsInDim(Atlas, V3S(X, Y, Z)));
    
    LIGHT_ATLAS_ASSERT(Tx < Atlas->TileDim.x);
    LIGHT_ATLAS_ASSERT(Ty < Atlas->TileDim.y);
    
    v3 *Texels = (v3 *)GetLightAtlasTexels(Atlas);
    
    light_atlas_texel Result;
    Result.Value = Texels + LIGHT_ATLAS_OFFSET(Atlas, X, Y, Z, Tx, Ty);
    
    LIGHT_ATLAS_ASSERT(PointerIsInBounds(Atlas, Result));
    
    return(Result);
}

internal light_atlas_texel
GetTileClamped(light_atlas *Atlas, s32 X, s32 Y, s32 Z, u32 Tx, u32 Ty)
{
    X = Clamp(0, X, Atlas->VoxelDim.x - 1);
    Y = Clamp(0, Y, Atlas->VoxelDim.y - 1);
    Z = Clamp(0, Z, Atlas->VoxelDim.z - 1);
    
    light_atlas_texel Result = GetTileUnclamped(Atlas, X, Y, Z, Tx, Ty);
    
    return(Result);
}

internal light_atlas_texel
OffsetFromTexel(light_atlas *Atlas, light_atlas_texel Base, u32 Tx, u32 Ty)
{
    light_atlas_texel Result = Base;
    Result.Value += Atlas->TileRowStride*Ty + Tx;
    
    LIGHT_ATLAS_ASSERT(PointerIsInBounds(Atlas, Result));
    
    return(Result);
}

internal light_atlas_texel
OffsetFromTexel(light_atlas *Atlas, light_atlas_texel Base, v2u TOffset)
{
    light_atlas_texel Result = OffsetFromTexel(Atlas, Base, TOffset.x, TOffset.y);
    
    return(Result);
}

internal void
AdvanceRow(light_atlas *Atlas, light_atlas_texel *Texel)
{
    Texel->Value += Atlas->TileRowStride;
}

internal void
CopyTile(light_atlas *Atlas,
         u32 SourceX,
         u32 SourceY,
         u32 SourceZ,
         u32 DestX,
         u32 DestY,
         u32 DestZ)
{
    light_atlas_texel SourceTile = GetTileUnclamped(Atlas, SourceX, SourceY, SourceZ);
    light_atlas_texel DestTile = GetTileUnclamped(Atlas, DestX, DestY, DestZ);
    
    for(u32 Ty = 0;
        Ty < Atlas->TileDim.y;
        ++Ty)
    {
        LIGHT_ATLAS_ASSERT(PointerIsInBounds(Atlas, SourceTile));
        LIGHT_ATLAS_ASSERT(PointerIsInBounds(Atlas, DestTile));
        
        for(u32 Tx = 0;
            Tx < Atlas->TileDim.x;
            ++Tx)
        {
            DestTile.Value[Tx] = SourceTile.Value[Tx];
        }
        
        AdvanceRow(Atlas, &SourceTile);
        AdvanceRow(Atlas, &DestTile);
    }
}

internal void
ZeroTile(light_atlas *Atlas,
         u32 DestX,
         u32 DestY,
         u32 DestZ)
{
    light_atlas_texel DestTile = GetTileUnclamped(Atlas, DestX, DestY, DestZ);
    
    for(u32 Ty = 0;
        Ty < Atlas->TileDim.y;
        ++Ty)
    {
        LIGHT_ATLAS_ASSERT(PointerIsInBounds(Atlas, DestTile));
        
        for(u32 Tx = 0;
            Tx < Atlas->TileDim.x;
            ++Tx)
        {
            DestTile.Value[Tx] = V3(0, 0, 0);
        }
        
        AdvanceRow(Atlas, &DestTile);
    }
}

internal void
BlockCopyAtlas(light_atlas *Atlas, v3s FetchOffset)
{
    v3s dVoxel = FetchOffset;
    // TODO(casey): If we wanted to speed this up, there are probably ways of
    // doing this copy that work on entire rows / columns at a time, etc.
    
    VALIDATE_TEXEL_ATLAS(Atlas);
    
    s32 dY, StopY, InitialY, FinalY;
    if(dVoxel.y < 0)
    {
        dY = -1;
        InitialY = Atlas->VoxelDim.y - 1;
        StopY = Minimum(InitialY, -dVoxel.y + dY);
        FinalY = 0 + dY;
    }
    else
    {
        dY = 1;
        InitialY = 0;
        StopY = Maximum(InitialY, Atlas->VoxelDim.y - dVoxel.y);
        FinalY = Atlas->VoxelDim.y;
    }
    
    s32 dX, StopX, InitialX, FinalX;
    if(dVoxel.x < 0)
    {
        dX = -1;
        InitialX = Atlas->VoxelDim.x - 1;
        StopX = Minimum(InitialX, -dVoxel.x + dX);
        FinalX = 0 + dX;
    }
    else
    {
        dX = 1;
        InitialX = 0;
        StopX = Maximum(InitialX, Atlas->VoxelDim.x - dVoxel.x);
        FinalX = Atlas->VoxelDim.x;
    }
    
    s32 dZ, StopZ, InitialZ, FinalZ;
    if(dVoxel.z < 0)
    {
        dZ = -1;
        InitialZ = Atlas->VoxelDim.z - 1;
        StopZ = Minimum(InitialZ, -dVoxel.z + dZ);
        FinalZ = 0 + dZ;
    }
    else
    {
        dZ = 1;
        InitialZ = 0;
        StopZ = Maximum(InitialZ, Atlas->VoxelDim.z - dVoxel.z);
        FinalZ = Atlas->VoxelDim.z;
    }
    
    s32 Z = InitialZ;
    while(Z != StopZ)
    {
        s32 Y = InitialY;
        while(Y != StopY)
        {
            s32 X = InitialX;
            while(X != StopX)
            {
                CopyTile(Atlas,
                         X + dVoxel.x, Y + dVoxel.y, Z + dVoxel.z, // NOTE(casey): Source
                         X, Y, Z // NOTE(casey): Dest
                         );
                X += dX;
            }
            
            while(X != FinalX)
            {
                ZeroTile(Atlas, X, Y, Z);
                X += dX;
            }
            
            Y += dY;
        }
        
        while(Y != FinalY)
        {
            for(s32 X = 0; X < Atlas->VoxelDim.x; ++X)
            {
                ZeroTile(Atlas, X, Y, Z);
            }
            
            Y += dY;
        }
        
        Z += dZ;
    }
    
    while(Z != FinalZ)
    {
        for(s32 Y = 0; Y < Atlas->VoxelDim.y; ++Y)
        {
            for(s32 X = 0; X < Atlas->VoxelDim.x; ++X)
            {
                ZeroTile(Atlas, X, Y, Z);
            }
        }
        
        Z += dZ;
    }
    
    VALIDATE_TEXEL_ATLAS(Atlas);
}


internal b32
ValidateTexel_(f32_4x Texel)
{
    b32 Result = (ValidateTexelComponent(Texel.E[0]) &&
                      ValidateTexelComponent(Texel.E[1]) &&
                      ValidateTexelComponent(Texel.E[2]) &&
                      ValidateTexelComponent(Texel.E[3]));
    return(Result);
}

internal b32
ValidateAtlas_(light_atlas *Atlas)
{
    b32 Result = true;
    
    v3 *Texel = (v3 *)GetLightAtlasTexels(Atlas);
    umm TexelCount = GetLightAtlasTexelCount(Atlas);
    for(umm TexelIndex = 0;
        TexelIndex < TexelCount;
        ++TexelIndex)
    {
        Result = (Result && ValidateTexel_(Texel[TexelIndex]));
        
    }
    
    return(Result);
}
