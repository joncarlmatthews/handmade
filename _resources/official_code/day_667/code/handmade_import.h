/* ========================================================================
   $File: C:\work\handmade\code\handmade_import.h $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

struct game_assets;
struct asset_source_file;
#define ASSET_IMPORT_GRID_MAX 8

enum import_type
{
    ImportType_None, 
    
    ImportType_Plate,
    ImportType_SingleTile,
    ImportType_MultiTile,
    ImportType_Audio,
    
    ImportType_Count,
};

struct import_tag_array
{
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
};

struct import_grid_tag
{
    asset_basic_category TypeID;
    import_tag_array Tags;
    u32 VariantGroup;
};

struct import_grid_tags
{
    import_grid_tag Tags[ASSET_IMPORT_GRID_MAX][ASSET_IMPORT_GRID_MAX];
    u32 VariantGroupCount; // TODO(casey): Scan to compute this value, rather than manually setting it
};

struct import_file_match
{
    b32 Valid;
    b32 NeedsFullRebuild;
    asset_source_file *Source;
    platform_file_info *FileInfo;
};

struct hht_fields
{
    string Name;
    string Author;
    string Description;
};

struct hht_context
{
    platform_file_group FileGroup;
    game_assets *Assets;
    hht_fields DefaultFields;
    memory_arena *TempArena;
    string HHAStem;
    u32 HHAIndex;
    u32 IncludeDepth;
    stream *ErrorStream;
    
    // NOTE(casey): These are only non-zero when we are rewriting HHTs
    // {
    stream *HHTOut;
    u8 *HHTCopyPoint;
    // }
};

internal void SynchronizeAssetFileChanges(game_assets *Assets, b32x SaveChangesToHHTs);
