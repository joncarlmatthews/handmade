/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal b32
IsToParent(hha_align_point Point)
{
    b32 Result = (Point.Type & HHAAlign_ToParent);
    return(Result);
}

internal hha_align_point_type
GetType(hha_align_point Point)
{
    hha_align_point_type Result = (hha_align_point_type)(Point.Type & ~HHAAlign_ToParent);
    return(Result);
}

internal v2
GetPPercent(hha_align_point Point)
{
    v2 Result =
    {
        -2.0f + 4.0f*((f32)Point.PPercent[0] / (f32)(U16Max - 1)),
        -2.0f + 4.0f*((f32)Point.PPercent[1] / (f32)(U16Max - 1)),
    };
    
    return(Result);
}

internal f32
GetSize(hha_align_point Point)
{
    f32 Result = (85.0f * (f32)Point.Size) / (f32)U16Max;
    
    return(Result);
}

internal void
SetAlignPoint(hha_align_point *Dest,
              hha_align_point_type Type, b32x ToParent,
              f32 Size, v2 PPercent)
{
    Dest->PPercent[0] = (u16)RoundReal32ToUInt32(Clamp01MapToRange(-2.0f, PPercent.x, 2.0f) * (f32)(U16Max - 1));
    Dest->PPercent[1] = (u16)RoundReal32ToUInt32(Clamp01MapToRange(-2.0f, PPercent.y, 2.0f) * (f32)(U16Max - 1));
    Dest->Size = (u16)RoundReal32ToUInt32((Size * (f32)U16Max) / 85.0f);
    Dest->Type = (u16)(Type | (ToParent ? HHAAlign_ToParent : 0));
}

internal v2
GetFirstAlign(hha_bitmap *BitmapInfo)
{
    // TODO(casey): We should probably have a more sane thing here than "GetFirstAlign".
    v2 Result = {0.5f, 0.5f};
    
    if(BitmapInfo->AlignPoints[0].Type)
    {
        Result = GetPPercent(BitmapInfo->AlignPoints[0]);
    }
    
    return(Result);
}

internal hha_align_point
FindAlign(hha_bitmap *BitmapInfo, u32 CompleteType)
{
    hha_align_point Result = {};
    
    for(u32 PIndex = 0;
        PIndex < ArrayCount(BitmapInfo->AlignPoints);
        ++PIndex)
    {
        if(BitmapInfo->AlignPoints[PIndex].Type == CompleteType)
        {
            Result = BitmapInfo->AlignPoints[PIndex];
            break;
        }
    }
    
    if((CompleteType == (HHAAlign_Default|HHAAlign_ToParent)) &&
       (Result.Type == 0))
    {
        SetAlignPoint(&Result,
                      HHAAlign_Default, true,
                      1.0f, V2(0.5f, 0.5f));
    }
    
    return(Result);
}

struct name_tag
{
    string Name;
    asset_tag_id ID;
};

struct type_from_id_v0
{
    hha_asset_type Type;
    char *Name;
    asset_basic_category Category;
};

struct category_tag
{
    string Name;
    asset_basic_category ID;
};

global category_tag CategoryTags[] =
{
    {ConstZ("None"), Asset_None},
    
    {ConstZ("Shadow"),Asset_Shadow},
    {ConstZ("Tree"),Asset_Tree},
    {ConstZ("Sword"),Asset_Sword},
    {ConstZ("Rock"),Asset_Rock},
    {ConstZ("Grass"),Asset_Grass},
    {ConstZ("Tuft"),Asset_Tuft},
    {ConstZ("Stone"),Asset_Stone},
    {ConstZ("Head"),Asset_Head},
    {ConstZ("Cape"),Asset_Cape},
    {ConstZ("Body"),Asset_Body},
    {ConstZ("Font"),Asset_Font},
    {ConstZ("FontGlyph"),Asset_FontGlyph},
    {ConstZ("Bloop"),Asset_Bloop},
    {ConstZ("Crack"),Asset_Crack},
    {ConstZ("Drop"),Asset_Drop},
    {ConstZ("Glide"),Asset_Glide},
    {ConstZ("Music"),Asset_Music},
    {ConstZ("Puhp"),Asset_Puhp},
    {ConstZ("OpeningCutscene"),Asset_OpeningCutscene},
    {ConstZ("Hand"),Asset_Hand},
    {ConstZ("Block"),Asset_Block},
    {ConstZ("Particle"),Asset_Particle},
    {ConstZ("Item"),Asset_Item},
    {ConstZ("Scenery"),Asset_Scenery},
    {ConstZ("Plate"),Asset_Plate},
    {ConstZ("Audio"),Asset_Audio},
};

internal string
CategoryNameFromID(asset_basic_category ID)
{
    string Result = ConstZ("UNKNOWN");
    for(u32 NameIndex = 0;
        NameIndex < ArrayCount(CategoryTags);
        ++NameIndex)
    {
        if(CategoryTags[NameIndex].ID == ID)
        {
            Result = CategoryTags[NameIndex].Name;
        }
    }
    
    return(Result);
}

global name_tag NameTags[] =
{
    {ConstZ("none"), Tag_None},
    
    {ConstZ("Flatness"), Tag_Flatness},
    {ConstZ("FacingDirection"), Tag_FacingDirection},
    {ConstZ("UnicodeCodepoint"), Tag_UnicodeCodepoint},
    {ConstZ("FontType"), Tag_FontType},
    
    {ConstZ("ShotIndex"), Tag_ShotIndex},
    {ConstZ("LayerIndex"), Tag_LayerIndex},
    
    {ConstZ("Primacy"), Tag_Primacy},
    {ConstZ("BasicCategory"), Tag_BasicCategory},
    
    {ConstZ("Bones"), Tag_Bones},
    {ConstZ("Dark"), Tag_DarkEnergy},
    {ConstZ("DarkEnergy"), Tag_DarkEnergy},
    {ConstZ("Glove"), Tag_Glove},
    {ConstZ("Fingers"), Tag_Fingers},
    
    {ConstZ("Wood"), Tag_Wood},
    {ConstZ("Stone"), Tag_Stone},
    {ConstZ("Drywall"), Tag_Drywall},
    {ConstZ("Manmade"), Tag_Manmade},
    {ConstZ("Wall"), Tag_Wall},
    {ConstZ("Floor"), Tag_Floor},
    {ConstZ("Grass"), Tag_Grass},
    
    {ConstZ("Idle"), Tag_Idle},
    {ConstZ("Dodge"), Tag_Dodge},
    {ConstZ("Move"), Tag_Move},
    {ConstZ("Hit"), Tag_Hit},
    {ConstZ("Attack1"), Tag_Attack1},
    {ConstZ("Attack2"), Tag_Attack2},
    {ConstZ("Surprise"), Tag_Surprise},
    {ConstZ("Anger"), Tag_Anger},
    
    {ConstZ("Cat"), Tag_Cat},
    {ConstZ("Birman"), Tag_Birman},
    {ConstZ("Ghost"), Tag_Ghost},
    {ConstZ("Tabby"), Tag_Tabby},
    {ConstZ("Brown"), Tag_Brown},
    {ConstZ("Gray"), Tag_Gray},
    {ConstZ("Krampus"), Tag_Krampus},
    {ConstZ("Undead"), Tag_Undead},
    {ConstZ("Broken"), Tag_Broken},
    {ConstZ("Wrapped"), Tag_Wrapper},
    
    {ConstZ("Orphan"), Tag_Orphan},
    {ConstZ("Baby"), Tag_Baby},
    {ConstZ("Hero"), Tag_Hero},
    {ConstZ("Brahm"), Tag_Brahm},
    {ConstZ("Carla"), Tag_Carla},
    {ConstZ("Cassidy"), Tag_Cassidy},
    {ConstZ("Drew"), Tag_Drew},
    {ConstZ("Dylan"), Tag_Dylan},
    {ConstZ("Giles"), Tag_Giles},
    {ConstZ("Kline"), Tag_Kline},
    {ConstZ("Laird"), Tag_Laird},
    {ConstZ("Lambert"), Tag_Lambert},
    {ConstZ("Rhoda"), Tag_Rhoda},
    {ConstZ("Slade"), Tag_Slade},
    {ConstZ("Sunny"), Tag_Sunny},
    {ConstZ("Viva"), Tag_Viva},
    
    {ConstZ("Cook"), Tag_Cook},
    {ConstZ("Earth"), Tag_Earth},
    {ConstZ("Fall"), Tag_Fall},
    {ConstZ("Health"), Tag_Health},
    {ConstZ("Fauna"), Tag_Fauna},
    {ConstZ("Speed"), Tag_Speed},
    {ConstZ("Spring"), Tag_Spring},
    {ConstZ("Strength"), Tag_Strength},
    {ConstZ("Summer"), Tag_Summer},
    {ConstZ("Tailor"), Tag_Tailor},
    {ConstZ("Tank"), Tag_Tank},
    {ConstZ("Winter"), Tag_Winter},
    
    {ConstZ("IntroCutscene"), Tag_IntroCutscene},
    {ConstZ("TitleScreen"), Tag_TitleScreen},
    
    {ConstZ("Bloop"), Tag_Bloop},
    {ConstZ("Crack"), Tag_Crack},
    {ConstZ("Drop"), Tag_Drop},
    {ConstZ("Glide"), Tag_Glide},
    {ConstZ("Puhp"), Tag_Puhp},
    
    {ConstZ("Variant"), Tag_Variant},
    {ConstZ("ChannelIndex"), Tag_ChannelIndex},
    
    {ConstZ("Flowers"), Tag_Flowers},
    {ConstZ("Size"), Tag_Size},
    {ConstZ("Chair"), Tag_Chair},
    {ConstZ("Lamp"), Tag_Lamp},
    
    {ConstZ("Worn"),Tag_Worn},
    {ConstZ("Hat"),Tag_Hat},
    
    {ConstZ("__DEPRECATED_Snow"), Tag_DEPRECATED_Snow},
    {ConstZ("Tree"), Tag_Tree},
    {ConstZ("__DEPRECATED_Evergreen"), Tag_DEPRECATED_Evergreen},
    {ConstZ("Damaged"), Tag_Damaged},
    
    {ConstZ("Stars"), Tag_Stars},
    {ConstZ("Smoke"), Tag_Smoke},
    {ConstZ("Cover"), Tag_Cover},
    {ConstZ("Particle"), Tag_Particle},
    {ConstZ("Fire"), Tag_Fire},
    
    {ConstZ("Table"), Tag_Table},
    {ConstZ("Rug"), Tag_Rug},
    {ConstZ("Sofa"), Tag_Sofa},

    {ConstZ("Large"), Tag_Large},
    
    { ConstZ("ShotIndex1"),  Tag_ShotIndex1},
    { ConstZ("ShotIndex2"),  Tag_ShotIndex2},
    { ConstZ("ShotIndex3"),  Tag_ShotIndex3},
    { ConstZ("ShotIndex4"),  Tag_ShotIndex4},
    { ConstZ("ShotIndex5"),  Tag_ShotIndex5},
    { ConstZ("ShotIndex6"),  Tag_ShotIndex6},
    { ConstZ("ShotIndex7"),  Tag_ShotIndex7},
    { ConstZ("ShotIndex8"),  Tag_ShotIndex8},
    { ConstZ("ShotIndex9"),  Tag_ShotIndex9},
    {ConstZ("ShotIndex10"), Tag_ShotIndex10},
    {ConstZ("ShotIndex11"), Tag_ShotIndex11},
    {ConstZ("ShotIndex12"), Tag_ShotIndex12},
    {ConstZ("ShotIndex13"), Tag_ShotIndex13},
    {ConstZ("ShotIndex14"), Tag_ShotIndex14},
    {ConstZ("ShotIndex15"), Tag_ShotIndex15},
    {ConstZ("ShotIndex16"), Tag_ShotIndex16},
    
    { ConstZ("LayerIndex1"),  Tag_LayerIndex1},
    { ConstZ("LayerIndex2"),  Tag_LayerIndex2},
    { ConstZ("LayerIndex3"),  Tag_LayerIndex3},
    { ConstZ("LayerIndex4"),  Tag_LayerIndex4},
    { ConstZ("LayerIndex5"),  Tag_LayerIndex5},
    { ConstZ("LayerIndex6"),  Tag_LayerIndex6},
    { ConstZ("LayerIndex7"),  Tag_LayerIndex7},
    { ConstZ("LayerIndex8"),  Tag_LayerIndex8},
    { ConstZ("LayerIndex9"),  Tag_LayerIndex9},
    {ConstZ("LayerIndex10"), Tag_LayerIndex10},
    {ConstZ("LayerIndex11"), Tag_LayerIndex11},
    {ConstZ("LayerIndex12"), Tag_LayerIndex12},
    {ConstZ("LayerIndex13"), Tag_LayerIndex13},
    {ConstZ("LayerIndex14"), Tag_LayerIndex14},
    {ConstZ("LayerIndex15"), Tag_LayerIndex15},
    {ConstZ("LayerIndex16"), Tag_LayerIndex16},
        
    {ConstZ("DebugFont"), Tag_DebugFont},
    
    {ConstZ("Molly"), Tag_Molly},
    {ConstZ("Fred"), Tag_Fred},
    {ConstZ("Hannah"), Tag_Hannah},
};

internal string
TagNameFromID(asset_tag_id ID)
{
    string Result = ConstZ("UNKNOWN");
    for(u32 NameIndex = 0;
        NameIndex < ArrayCount(NameTags);
        ++NameIndex)
    {
        if(NameTags[NameIndex].ID == ID)
        {
            Result = NameTags[NameIndex].Name;
        }
    }
    
    return(Result);
}

internal asset_tag_id
TagIDFromName(string Name)
{
    asset_tag_id Result = Tag_None;
    for(u32 NameIndex = 0;
        NameIndex < ArrayCount(NameTags);
        ++NameIndex)
    {
        if(StringsAreEqualLowercase(Name, NameTags[NameIndex].Name))
        {
            Result = NameTags[NameIndex].ID;
        }
    }
    
    return(Result);
}

struct align_point_type_name
{
    string Name;
    hha_align_point_type Type;
};

global align_point_type_name AlignPointTypeNames[] =
{
    {ConstZ("none"), HHAAlign_None},
    {ConstZ("Default"), HHAAlign_Default},
    {ConstZ("TopOfHead"), HHAAlign_TopOfHead},
    {ConstZ("BaseOfNeck"), HHAAlign_BaseOfNeck},
};

internal string
AlignPointNameFromType(hha_align_point_type Type)
{
    string Result = ConstZ("UNKNOWN");
    for(u32 NameIndex = 0;
        NameIndex < ArrayCount(AlignPointTypeNames);
        ++NameIndex)
    {
        if(AlignPointTypeNames[NameIndex].Type == Type)
        {
            Result = AlignPointTypeNames[NameIndex].Name;
        }
    }
    
    return(Result);
}

internal hha_align_point_type
AlignPointTypeFromName(string Name)
{
    hha_align_point_type Result = HHAAlign_None;
    for(u32 NameIndex = 0;
        NameIndex < ArrayCount(AlignPointTypeNames);
        ++NameIndex)
    {
        if(StringsAreEqualLowercase(Name, AlignPointTypeNames[NameIndex].Name))
        {
            Result = AlignPointTypeNames[NameIndex].Type;
        }
    }
    
    return(Result);
}

type_from_id_v0 TypeFromIDV0[] =
{
    {HHAAsset_None, "NONE", Asset_None},
    
    {HHAAsset_Bitmap, "Asset_Shadow", Asset_Shadow},
    {HHAAsset_Bitmap, "Asset_Tree", Asset_Tree},
    {HHAAsset_Bitmap, "Asset_Sword", Asset_Sword},
    {HHAAsset_Bitmap, "Asset_Rock", Asset_Rock},
    
    {HHAAsset_Bitmap, "Asset_Grass", Asset_Grass},
    {HHAAsset_Bitmap, "Asset_Tuft", Asset_Tuft},
    {HHAAsset_Bitmap, "Asset_Stone", Asset_Stone},
    
    {HHAAsset_Bitmap, "Asset_Head", Asset_Head},
    {HHAAsset_Bitmap, "Asset_Cape", Asset_Cape},
    {HHAAsset_Bitmap, "Asset_Body", Asset_Body},
    
    {HHAAsset_Font, "Asset_Font", Asset_Font},
    {HHAAsset_Bitmap, "Asset_FontGlyph", Asset_FontGlyph},
    
    {HHAAsset_Sound, "Asset_Bloop", Asset_Bloop},
    {HHAAsset_Sound, "Asset_Crack", Asset_Crack},
    {HHAAsset_Sound, "Asset_Drop", Asset_Drop},
    {HHAAsset_Sound, "Asset_Glide", Asset_Glide},
    {HHAAsset_Sound, "Asset_Music", Asset_Music},
    {HHAAsset_Sound, "Asset_Puhp", Asset_Puhp},
    
    {HHAAsset_Bitmap, "Asset_OpeningCutscene", Asset_OpeningCutscene},
    {HHAAsset_Bitmap, "Asset_Hand", Asset_Hand},
};
