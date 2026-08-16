/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct in_game_editor;

struct editable_asset
{
    dev_id ID;
    u32 AssetIndex;
    f32 SortKey; // NOTE(casey): Less is better
};

struct editable_asset_group
{
    u32 AssetCount;
    editable_asset Assets[8];
};

struct editable_hit_test
{
    editable_asset_group *DestGroup;
    v2 ClipSpaceMouseP;
    
    v4 HighlightColor;
    dev_id HighlightID;
    
    in_game_editor *Editor;
};

enum in_game_edit_type
{
    Edit_None,
    
    Edit_align_point_edit,
    
    Edit_Count,
};
enum in_game_edit_change_type
{
    EditChange_From = 0,
    EditChange_To = 1,
    
    EditChange_Count,
};
struct align_point_edit
{
    u32 AssetIndex;
    u32 AlignPointIndex;
    hha_align_point Change[EditChange_Count];
};

struct in_game_edit
{
    in_game_edit *Next;
    in_game_edit *Prev;
    
    in_game_edit_type Type;
    union
    {
        align_point_edit AlignPointEdit;
    };
    
    // TODO(casey): We may want to "group" edits together, so that "undo" can undo
    // more than one change at a time.  This facilitates undoing edit operations as
    // the user perceives rather than how their implemented internally in the case
    // of edits that edit multiple things at once.
};

struct in_game_editor
{
    memory_arena UndoMemory;
    game_assets *Assets;
    
    editable_asset_group ActiveGroup;
    editable_asset_group HotGroup;
    
    dev_mode DevMode;
    u32 ActiveAssetIndex;
    dev_id HighlightID;
    
    dev_ui_section_picker MainSection; 
    
    in_game_edit *CleanEdit; // NOTE(casey): Tells us whether we are "dirty" or not
    in_game_edit UndoSentinel;
    in_game_edit RedoSentinel;
    in_game_edit InProgressSentinel;
    
    dev_ui_interaction ToExecute;
    dev_ui_interaction NextToExecute;
    
    b32x DrawChildren;
    b32x DrawAlignPoint[HHA_BITMAP_ALIGN_POINT_COUNT];
};

internal b32x ShouldDrawChildren(editable_hit_test *HitTest, dev_id ParentID);
internal b32x ShouldHitTest(editable_hit_test *HitTest);
internal void AddHit(editable_hit_test *HitTest, dev_id ID, u32 AssetIndex, f32 SortKey);
internal b32x ShouldDrawAlignPoint(editable_hit_test *HitTest, u32 APIndex);
