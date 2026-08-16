/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define ASSET_MAX_PLATE_DIM 2048
#define ASSET_MAX_SPRITE_DIM 512

struct asset_tag_hash
{
    u64 Value;
};

enum asset_match_vector_element
{
    MatchElement_FacingDirection,
    MatchElement_Variant,

    MatchElement_Count,
    
    MatchElement_Channel = MatchElement_FacingDirection,
};

struct asset_match_vector
{
    f32 E[MatchElement_Count];
};

enum asset_tag_id
{
    Tag_None = 0,

    Tag_Flatness,
    Tag_FacingDirection, // NOTE(casey): Angle in radians off of due right
    Tag_UnicodeCodepoint,
    Tag_FontType, // NOTE(casey): 0 - Default Game Font, 10 - Debug Font?

    Tag_ShotIndex,
    Tag_LayerIndex,

    Tag_Primacy, // NOTE(casey): 0 everywhere, except if you are trying to override
    // an existing asset, in which case you set it to its Tag_Primacy + 1.

    Tag_BasicCategory,

    Tag_Bones,
    Tag_DarkEnergy,
    Tag_Glove,
    Tag_Fingers,

    Tag_Wood,
    Tag_Stone,
    Tag_Drywall,
    Tag_Manmade,
    Tag_Wall,
    Tag_Floor,
    Tag_Grass,

    Tag_Idle,
    Tag_Dodge,
    Tag_Move,
    Tag_Hit,
    Tag_Attack1,
    Tag_Attack2,
    Tag_Surprise,
    Tag_Anger,

    Tag_Cat,
    Tag_Birman,
    Tag_Ghost,
    Tag_Tabby,
    Tag_Brown,
    Tag_Gray,
    Tag_Krampus,
    Tag_Undead,
    Tag_Broken,
    Tag_Wrapper,

    Tag_Orphan,

    Tag_Baby,
    Tag_Hero,
    Tag_Brahm,
    Tag_Carla,
    Tag_Cassidy,
    Tag_Drew,
    Tag_Dylan,
    Tag_Giles,
    Tag_Kline,
    Tag_Laird,
    Tag_Lambert,
    Tag_Rhoda,
    Tag_Slade,
    Tag_Sunny,
    Tag_Viva,

    Tag_Cook,
    Tag_Earth,
    Tag_Fall,
    Tag_Health,
    Tag_Fauna,
    Tag_Speed,
    Tag_Spring,
    Tag_Strength,
    Tag_Summer,
    Tag_Tailor,
    Tag_Tank,
    Tag_Winter,

    Tag_IntroCutscene,
    Tag_TitleScreen,

    Tag_Bloop,
    Tag_Crack,
    Tag_Drop,
    Tag_Glide,
    Tag_Puhp,

    Tag_Variant,
    Tag_ChannelIndex,

    Tag_Flowers,
    Tag_Size,
    Tag_Chair,
    Tag_Lamp,

    Tag_Worn,
    Tag_Hat,

    Tag_DEPRECATED_Snow,
    Tag_Tree,
    Tag_DEPRECATED_Evergreen,
    Tag_Damaged,

    Tag_Stars,
    Tag_Smoke,
    Tag_Cover,
    Tag_Particle,
    Tag_Fire,

    Tag_Table,
    Tag_Rug,
    Tag_Sofa,

    Tag_Large,

    Tag_ShotIndex1,
    Tag_ShotIndex2,
    Tag_ShotIndex3,
    Tag_ShotIndex4,
    Tag_ShotIndex5,
    Tag_ShotIndex6,
    Tag_ShotIndex7,
    Tag_ShotIndex8,
    Tag_ShotIndex9,
    Tag_ShotIndex10,
    Tag_ShotIndex11,
    Tag_ShotIndex12,
    Tag_ShotIndex13,
    Tag_ShotIndex14,
    Tag_ShotIndex15,
    Tag_ShotIndex16,

    Tag_LayerIndex1,
    Tag_LayerIndex2,
    Tag_LayerIndex3,
    Tag_LayerIndex4,
    Tag_LayerIndex5,
    Tag_LayerIndex6,
    Tag_LayerIndex7,
    Tag_LayerIndex8,
    Tag_LayerIndex9,
    Tag_LayerIndex10,
    Tag_LayerIndex11,
    Tag_LayerIndex12,
    Tag_LayerIndex13,
    Tag_LayerIndex14,
    Tag_LayerIndex15,
    Tag_LayerIndex16,

    Tag_DebugFont,
    
    Tag_Molly,
    Tag_Fred,
    Tag_Hannah,
    
    Tag_Count,
};

enum hha_asset_type
{
    HHAAsset_None,

    HHAAsset_Bitmap,
    HHAAsset_Sound,
    HHAAsset_Font,

    HHAAsset_Count,
};

enum asset_basic_category
{
    Asset_None,

    // NOTE(casey): Legacy categories
    Asset_Shadow,
    Asset_Tree,
    Asset_Sword,
    Asset_Rock,
    Asset_Grass,
    Asset_Tuft,
    Asset_Stone,
    Asset_Head, // NOTE(casey): Still used
    Asset_Cape,
    Asset_Body, // NOTE(casey): Still used
    Asset_Font, // NOTE(casey): Still used
    Asset_FontGlyph, // NOTE(casey): Still used
    Asset_Bloop,
    Asset_Crack,
    Asset_Drop,
    Asset_Glide,
    Asset_Music,
    Asset_Puhp,
    Asset_OpeningCutscene, // NOTE(casey): Still used
    Asset_Hand, // NOTE(casey): Still used

    // NOTE(casey): New categories
    Asset_Block,
    Asset_Particle,
    Asset_Item,
    Asset_Scenery,
    Asset_Plate,
    Asset_Audio,

    Asset_Category_Count,
};

#define HHA_CODE(a, b, c, d) (((uint32)(a) << 0) | ((uint32)(b) << 8) | ((uint32)(c) << 16) | ((uint32)(d) << 24))

#pragma pack(push, 1)

#define HHA_MAGIC_VALUE HHA_CODE('h','h','a','f')
#define HHA_VERSION 2

struct hha_header
{
    u32 MagicValue;
    u32 Version;

    u32 TagCount;
    u32 AssetCount;

    u32 Reserved32[12];

    u64 Tags; // hha_tag[TagCount]
    u64 Assets; // hha_asset[AssetCount]
    u64 Annotations; // hha_annotation[AssetCount]

    u64 Reserved64[5];
};
CTAssert(sizeof(hha_header) == (16*4 + 8*8));

struct hha_tag
{
    enum32(asset_tag_id) ID;
    r32 Value;
};
CTAssert(sizeof(hha_tag) == (2*4));

struct hha_annotation
{
    // TODO(casey): Should we start _also_ storing machine-specific SourceFileDate out-of-band so that we don't update HHAs where dates change but checksums don't?
    u64 SourceFileDate;
    u64 SourceFileChecksum;
    u64 SourceFileBaseNameOffset;
    u64 AssetNameOffset;
    u64 AssetDescriptionOffset;
    u64 AuthorOffset;
    u64 ErrorStreamOffset;
    u64 HHTBlockChecksum;
    u64 Reserved[3];

    u32 ErrorStreamCount;
    u32 Reserved32;
    u32 SourceFileBaseNameCount;
    u32 AssetNameCount;
    u32 AssetDescriptionCount;
    u32 AuthorCount;

    u32 SpriteSheetX;
    u32 SpriteSheetY;
    u32 Reserved32_2[2];
};
CTAssert(sizeof(hha_annotation) == (16*8));

enum hha_sound_chain
{
    HHASoundChain_None,
    HHASoundChain_Loop,
    HHASoundChain_Advance,
};
enum hha_align_point_type
{
    HHAAlign_None,

    HHAAlign_Default,

    HHAAlign_TopOfHead,
    HHAAlign_BaseOfNeck,

    HHAAlign_Count,

    HHAAlign_ToParent = 0x8000,
};
struct hha_align_point // NOTE(casey): 8 bytes
{
    u16 PPercent[2];
    u16 Size;
    u16 Type;
};

#define HHA_BITMAP_ALIGN_POINT_COUNT 12
struct hha_bitmap
{
    // NOTE(casey): These are generally imported from TXT file augmentation of the PNG
    hha_align_point AlignPoints[HHA_BITMAP_ALIGN_POINT_COUNT];
    u16 Dim[2];
    u16 OrigDim[2];

    /* NOTE(casey): Data is:

       u32 Pixels[Dim[1]][Dim[0]]
    */
};

struct hha_sound
{
    // NOTE(casey): The SampleCount and ChannelCount are the _total_ samples and channels
    // for the sound, even though it is broken up into chunks and split across assets,
    // one per channel per chunk.
    u32 SampleCount;

    // TODO(casey): There's no reason to have 32-bits for each of these, they
    // could have just been 16 bits...
    // {
    u32 ChannelCount;
    enum32(hha_sound_chain) Chain;
    // }
    /* NOTE(casey): Data is:

       s16 Channels[SampleCount]
    */
};
struct hha_font_glyph
{
    u32 UnicodeCodePoint;
    u32 BitmapID;
};
struct hha_font
{
    u32 OnePastHighestCodepoint;
    u32 GlyphCount;
    r32 AscenderHeight;
    r32 DescenderHeight;
    r32 ExternalLeading;
    /* NOTE(casey): Data is:

       hha_font_glyph CodePoints[GlyphCount];
       r32 HorizontalAdvance[GlyphCount][GlyphCount];
    */
};

struct hha_asset
{
    u64 DataOffset;
    u32 DataSize;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    enum32(hha_asset_type) Type;
    union
    {
        hha_bitmap Bitmap;
        hha_sound Sound;
        hha_font Font;
        u64 MaxUnionSize[13];
    };
};
CTAssert(sizeof(hha_asset) == (16*8));

#pragma pack(pop)

struct loaded_hha_annotation
{
    u64 SourceFileDate;
    u64 SourceFileChecksum;
    u64 HHTBlockChecksum;
    u32 SpriteSheetX;
    u32 SpriteSheetY;

    string SourceFileBaseName;
    string AssetName;
    string AssetDescription;
    string Author;
    string ErrorStream;
};
