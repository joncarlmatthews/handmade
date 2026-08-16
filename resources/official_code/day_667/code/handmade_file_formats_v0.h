/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#pragma pack(push, 1)

struct hha_header_v0
{
    u32 MagicValue;
    u32 Version;
    
    u32 TagCount;
    u32 AssetTypeCount;
    u32 AssetCount;
    
    u64 Tags; // hha_tag[TagCount]
    u64 AssetTypes; // hha_asset_type[AssetTypeCount]
    u64 Assets; // hha_asset[AssetCount]
    
    // TODO(casey): Right now we have a situation where we are no longer making
    // contiguous asset type blocks - so it would be better to switch to just
    // having asset type IDs stored directly in the hha_asset, because it's
    // just burning space and cycles to store it in the AssetTypes array.
};

struct hha_asset_type_v0
{
    u32 TypeID;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct hha_bitmap_v0
{
    u32 Dim[2];
    f32 AlignPercentage[2];
    
    /* NOTE(casey): Data is:
    
       u32 Pixels[Dim[1]][Dim[0]]
    */
};

struct hha_asset_v0
{
    u64 DataOffset;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    union
    {
        hha_bitmap_v0 Bitmap;
        hha_sound Sound;
        hha_font Font;
    };
};

#if 0
enum asset_type_id_v0
{
    Asset_None,
    
    //
    // NOTE(casey): Bitmaps!
    //
    
    Asset_Shadow,
    Asset_Tree,
    Asset_Sword,
    //    Asset_Stairwell,
    Asset_Rock,
    
    Asset_Grass,
    Asset_Tuft,
    Asset_Stone,
    
    Asset_Head,
    Asset_Cape,
    Asset_Torso,
    
    Asset_Font,
    Asset_FontGlyph,
    
    //
    // NOTE(casey): Sounds!
    //
    
    Asset_Bloop,
    Asset_Crack,
    Asset_Drop,
    Asset_Glide,
    Asset_Music,
    Asset_Puhp,
    
    //
    //
    //
    
    Asset_OpeningCutscene,
    
    //
    //
    //
    
    Asset_Hand,
    
    Asset_Count,
};
#endif

#pragma pack(pop)
