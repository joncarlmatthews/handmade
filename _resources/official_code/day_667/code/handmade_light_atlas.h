/* ========================================================================
   $File: C:\work\handmade\code\handmade_light_atlas.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

#define LIGHT_ATLAS_IN_FLOAT 1
struct light_atlas
{
    f32 StrideXYZ_4x[4];
    f32 VoxelDim_4x[4];
    
    v3s VoxelDim;
    v2u TileDim;
    u8 *Texels;
    
    v2 OctDimCoefficient;
    v2 OxyCoefficient;
    
    u32 TileRowStride;
};

struct light_atlas_texel
{
    v3 *Value;
};

internal b32 IsFloat(light_atlas *Atlas);

internal u32 GetLightAtlasWidth(light_atlas *Atlas);
internal u32 GetLightAtlasHeight(light_atlas *Atlas);

internal umm GetLightAtlasTexelCount(light_atlas *Atlas);
internal u32 GetLightAtlasTexelSize(light_atlas *Atlas);
internal umm GetLightAtlasSize(light_atlas *Atlas);

internal light_atlas MakeLightAtlas(v3s VoxelDim, v2u TileDim);
internal void SetLightAtlasTexels(light_atlas *Atlas, void *Memory);
internal void *GetLightAtlasTexels(light_atlas *Atlas);

// NOTE(casey): Only the VOXEL is clamped, not the Tx/Ty, since that is never out of bounds
internal light_atlas_texel GetTileUnclamped(light_atlas *Atlas, s32 X, s32 Y, s32 Z, u32 Tx = 0, u32 Ty = 0);
internal light_atlas_texel GetTileClamped(light_atlas *Atlas, s32 X, s32 Y, s32 Z, u32 Tx = 0, u32 Ty = 0);
internal light_atlas_texel OffsetFromTexel(light_atlas *Atlas, light_atlas_texel Base, v2u TOffset);
internal void AdvanceRow(light_atlas *Atlas, light_atlas_texel *Texel);

// NOTE(casey): BlockCopyAtlas rewrites the atlas contents so that each
// cell |X,Y,Z| grabs its contents from |X+FetchOffset.x,Y+FetchOffset.y,Z+FetchOffset.z|
internal void BlockCopyAtlas(light_atlas *Atlas, v3s FetchOffset);

internal void CopyTile(light_atlas *Atlas, 
                       u32 SourceX, u32 SourceY, u32 SourceZ,
                       u32 DestX, u32 DestY, u32 DestZ);
internal void ZeroTile(light_atlas *Atlas, u32 DestX, u32 DestY, u32 DestZ);

internal b32
ValidateTexelComponent(f32 C)
{
#if 1
    b32 Result = ((C >= 0.0f) && (C <= 10.0f));
#else
    b32 Result = (C >= 0.0f);
#endif
    return(Result);
}

internal b32
ValidateTexel_(v3 Texel)
{
    b32 Result = (ValidateTexelComponent(Texel.r) &&
                  ValidateTexelComponent(Texel.g) &&
                  ValidateTexelComponent(Texel.b));
    return(Result);
}

#define LIGHT_ATLAS_ASSERT(...)
//#define LIGHT_ATLAS_ASSERT Assert

#define VALIDATE_TEXEL(...) LIGHT_ATLAS_ASSERT(ValidateTexel_(__VA_ARGS__))
#define VALIDATE_TEXEL_ATLAS(...) LIGHT_ATLAS_ASSERT(ValidateAtlas_(__VA_ARGS__))

internal b32 ValidateAtlas_(light_atlas *Atlas);

#define LIGHT_ATLAS_OFFSET(Atlas, X, Y, Z, Tx, Ty) \
((Z)*(Atlas)->TileRowStride*(Atlas)->TileDim.y + \
(Y)*(Atlas)->VoxelDim.x*(Atlas)->TileDim.x + \
(Ty)*(Atlas)->TileRowStride + \
(X)*(Atlas)->TileDim.x + (Tx))
