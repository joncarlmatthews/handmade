/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  NOTE(casey):

  - Snow for shot 2 (and 1?)
  - Closed door in shot 1
  - Handedness of glove?
  
 */

enum scene_layer_flags
{
    SceneLayerFlag_AtInfinty = 0x1,
    SceneLayerFlag_CounterCameraX = 0x2,
    SceneLayerFlag_CounterCameraY = 0x4,
    SceneLayerFlag_Transient = 0x8,
    SceneLayerFlag_Floaty = 0x10,
};

struct scene_layer
{
    v3 P;
    r32 Height;
    u32 Flags;
    v2 Param;
};

struct layered_scene
{
    asset_basic_category AssetType;
    asset_tag_id ShotTag;
    u32 LayerCount;
    scene_layer *Layers;

    r32 Duration;
    v3 CameraStart;
    v3 CameraEnd;

    r32 tFadeIn;
};

// NOTE(casey): Thank you to Ron Gilbert and friends for
// introducing the word "cutscene" into the game development
// vocabulary.
enum cutscene_id
{
    CutsceneID_Intro,
};
struct game_mode_cutscene
{
    cutscene_id ID;
    r32 t;
};

struct game_mode_title_screen
{
    r32 t;
};

struct game_state;
internal void PlayIntroCutscene(game_state *GameState);
internal void PlayTitleScreen(game_state *GameState);
