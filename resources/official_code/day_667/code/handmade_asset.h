/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct loaded_font
{
    hha_font_glyph *Glyphs;
    f32 *HorizontalAdvance;
    u32 BitmapIDOffset;
    u16 *UnicodeMap;
};

// TODO(casey): Streamling this, by using header pointer as an indicator of unloaded status?
enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
};

struct asset_lru_link
{
    asset_lru_link *Prev;
    asset_lru_link *Next;
};
struct asset_hash_entry
{
    asset_hash_entry *Next;
    asset_tag_hash TagHash;
    asset_match_vector MatchVector;
    u32 AssetIndex;
};

struct asset
{
    asset_lru_link LRU;

    u32 State;

    // TODO(casey): In the end, we probably want to make separate asset structs
    // for each of these because they are handled so differently now, it may not
    // make a whole lot of sense to have a single large asset struct...
    renderer_texture TextureHandle;
    u64 LoadedAtSoundBufferIndex;
    loaded_font Font;
    
    hha_asset HHA;
    hha_annotation Annotation;

    u32 FileIndex;
    u32 AssetIndexInFile;

    u32 Type;
    u32 NextOfType;
    
    u32 NextInTagHash;
    asset_tag_hash TagHash;
    asset_match_vector MatchVector;
};

struct asset_file
{
    platform_file_handle Handle;

    string Stem;
    hha_header Header;

    u32 TagBase;
    u32 AssetBase;

    u64 HighWaterMark;
    b32 Modified; // NOTE(casey): This will be set to "true" if changed were made to this file
};

struct asset_source_file
{
    asset_source_file *NextInHash;
    string BaseName;
    u64 FileDate;
    u64 FileCheckSum;
    u32 DestFileIndex; // NOTE(casey): Index of the asset_file to which this source file writes

    // NOTE(casey): [Y][X] - asset index in the game_assets::Assets array
    u32 AssetIndices[ASSET_IMPORT_GRID_MAX][ASSET_IMPORT_GRID_MAX];

    stream Errors;
};

struct asset_sound_buffer_ranges
{
    u64 SoundBufferBaseIndex;
    u64 SoundBufferLRUIndex;
};

struct sound_buffer_memory
{
    u64 BufferIndex;
    void *Pointer;
};

struct game_assets
{
    memory_arena NonRestoredMemory;

    renderer_texture_queue *TextureOpQueue;

    // TODO(casey): Not thrilled about this back-pointer
    struct game_state *GameState;

    // TODO(casey): This could just be allocated on demand now - there is no
    // reason for it to be an array, nobody uses it that way.
    u32 MaxFileCount;
    u32 FileCount;
    asset_file *Files;

    u32 MaxTagCount;
    u32 TagCount;
    hha_tag *Tags;

    u32 MaxAssetCount;
    u32 AssetCount;
    asset *Assets;

    u32 AssetsByTag[256]; 

    asset_source_file *SourceFileHash[256];

    u32 SampleBufferSize;
    u8 *SampleBuffer;
    u64 SampleBufferMappingMask;
    u64 SampleBufferTopIndex;
    u32 SampleBufferLRURange;

    u32 NormalTextureHandleCount;
    u32 SpecialTextureHandleCount;

    // TODO(casey): This needs to get updated into a more robust system
    u32 NextFreeTextureHandle;
    u32 NextSpecialTextureHandle;

    asset_lru_link SpecialTextureLRUSentinel;
    asset_lru_link RegularTextureLRUSentinel;

    u32 SaveNumber;
    memory_arena ErrorStreamMemory;
    stream ErrorStream;

    hha_asset NullHHAAsset;
    hha_bitmap NullHHABitmap;
    hha_font NullHHAFont;

#if HANDMADE_INTERNAL
    import_grid_tags AudioChannelTags;
    import_grid_tags ArtBlockTags;
    import_grid_tags ArtHeadTags;
    import_grid_tags ArtBodyTags;
    import_grid_tags ArtCharacterTags;
    import_grid_tags ArtParticleTags;
    import_grid_tags ArtHandTags;
    import_grid_tags ArtItemTags;
    import_grid_tags ArtSceneryTags;
    import_grid_tags ArtPlateTags;
#endif
};

internal renderer_texture GetBitmap(game_assets *Assets, bitmap_id ID);
internal hha_bitmap *GetBitmapInfo(game_assets *Assets, bitmap_id ID);
internal s16 *GetSoundSamples(game_assets *Assets, sound_id ID);
internal hha_asset *GetSoundInfo(game_assets *Assets, sound_id ID);
internal loaded_font *GetFont(game_assets *Assets, font_id ID);
internal hha_font *GetFontInfo(game_assets *Assets, font_id ID);

internal void LoadBitmap(game_assets *Assets, bitmap_id ID);
internal void LoadSound(game_assets *Assets, sound_id ID);
internal void LoadFont(game_assets *Assets, font_id ID);

internal void PrefetchBitmap(game_assets *Assets, bitmap_id ID);
internal void PrefetchSound(game_assets *Assets, sound_id ID);
internal void PrefetchFont(game_assets *Assets, font_id ID);

internal void UnloadBitmap(game_assets *Assets, bitmap_id ID);
internal void UnloadAudio(game_assets *Assets, sound_id ID);
internal u32 InitSourceHHA(game_assets *Assets, platform_file_group *FileGroup, platform_file_info *FileInfo);
internal asset_file *GetFile(game_assets *Assets, u32 FileIndex);
internal u32 GetAssetHashSlot(game_assets *Assets, asset_tag_hash TagHash);
internal b32 AddAssetToHash(game_assets *Assets, u32 AssetIndex);
internal void RemoveAssetFromHash(game_assets *Assets, u32 AssetIndex);
internal asset_source_file *GetOrCreateAssetSourceFile(game_assets *Assets, char *BaseName);
internal asset_source_file *GetOrCreateAssetSourceFile(game_assets *Assets, string BaseName);
internal asset *GetAsset(game_assets *Assets, u32 ID);

internal b32x DimensionsRequireSpecialTexture(game_assets *Assets, u32 Width, u32 Height);
