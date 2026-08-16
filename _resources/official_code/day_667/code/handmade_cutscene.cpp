/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define CUTSCENE_WARMUP_SECONDS 2.0f

internal void
RenderLayeredScene(game_assets *Assets, render_group *RenderGroup, 
                   layered_scene *Scene, r32 tNormal)
{
    f32 FocalLength = 0.78f;
    
    r32 SceneFadeValue = 1.0f;
    if(tNormal < Scene->tFadeIn)
    {
        SceneFadeValue = Clamp01MapToRange(0.0f, tNormal, Scene->tFadeIn);
    }
    
    // TODO(casey): Why does this fade seem to be wrong?  It appears nonlinear, but
    // if it is in linear brightness, shouldn't it _appear_ linear?
    v4 Color = {SceneFadeValue, SceneFadeValue, SceneFadeValue, 1.0f};
    
    v3 CameraStart = Scene->CameraStart;
    v3 CameraEnd = Scene->CameraEnd;
    v3 CameraOffset = Lerp(CameraStart, tNormal, CameraEnd);
    if(RenderGroup)
    {
        // TODO(casey): We could simplify this potentially in the future
        // by changing the camera situation here to use CameraP
        SetCameraTransform(RenderGroup, false, FocalLength,
                           V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 0),
                           0.1f, 1000.0f);
    }
    
    for(u32 LayerIndex = 0;
        LayerIndex < Scene->LayerCount;
        ++LayerIndex)
    {
        scene_layer Layer = Scene->Layers[LayerIndex];
        b32 Active = true;
        if(Layer.Flags & SceneLayerFlag_Transient)
        {
            Active = ((tNormal >= Layer.Param.x) &&
                      (tNormal < Layer.Param.y));
        }
        
        if(Active)
        {
            asset_tag_hash TagHash = GetTagHash(Scene->AssetType, Tag_IntroCutscene, Scene->ShotTag, (asset_tag_id)(Tag_LayerIndex1 + LayerIndex));
            bitmap_id LayerImage = GetBestMatchBitmapFrom(Assets, TagHash);
            
            if(RenderGroup)
            {
                v3 OffsetP = {};
                v3 P = Layer.P;
                if(Layer.Flags & SceneLayerFlag_AtInfinty)
                {
                    P.z += CameraOffset.z;
                }
                
                if(Layer.Flags & SceneLayerFlag_Floaty)
                {
                    P.y += Layer.Param.x*Sin(Layer.Param.y*tNormal);
                }
                
                if(Layer.Flags & SceneLayerFlag_CounterCameraX)
                {
                    OffsetP.x = P.x + CameraOffset.x;
                }
                else
                {
                    OffsetP.x = P.x - CameraOffset.x;
                }
                
                if(Layer.Flags & SceneLayerFlag_CounterCameraY)
                {
                    OffsetP.y = P.y + CameraOffset.y;
                }
                else
                {
                    OffsetP.y = P.y - CameraOffset.y;
                }
                
                hha_bitmap *BitmapInfo = GetBitmapInfo(RenderGroup->Assets, LayerImage);
                v2 AlignP = GetFirstAlign(BitmapInfo);
                OffsetP.z = P.z - CameraOffset.z;
                
                PushBitmap(RenderGroup, LayerImage, Layer.Height, OffsetP, AlignP, Color);
            }
            else
            {
                PrefetchBitmap(Assets, LayerImage);
            }
        }
    }
}

global scene_layer IntroLayers1[] =
{
    {{0.0f, 0.0f, -200.0f}, 300.0f, SceneLayerFlag_AtInfinty}, // NOTE(casey): Sky background
    {{0.0f, 0.0f, -170.0f}, 300.0f}, // NOTE(casey): Weird sky light
    {{0.0f, 0.0f, -100.0f}, 40.0f}, // NOTE(casey): Backmost row of trees
    {{0.0f, 10.0f, -70.0f}, 80.0f}, // NOTE(casey): Middle hills and trees
    {{0.0f, 0.0f, -50.0f}, 70.0f}, // NOTE(casey): Front hills and trees
    {{30.0f, 0.0f, -30.0f}, 50.0f}, // NOTE(casey): Right side tree and fence
    {{0.0f, -2.0f, -20.0f}, 40.0f}, // NOTE(casey): 7
    {{2.0f, -1.0f, -5.0f}, 25.0f}, // NOTE(casey): 8
};

global scene_layer IntroLayers2[] =
{
    {{3.0f, -4.0f, -62.0f}, 102.0f}, // NOTE(casey): Hero and tree
    {{0.0f, 0.0f, -14.0f}, 22.0f}, // NOTE(casey): Wall and window
    {{0.0f, 2.0f, -8.0f}, 10.0f}, // NOTE(casey): Icicles
};

global scene_layer IntroLayers3[] =
{
    {{0.0f, 0.0f, -30.0f}, 100.0f, SceneLayerFlag_AtInfinty}, // NOTE(casey): Sky
    {{0.0f, 0.0f, -20.0f}, 45.0f, SceneLayerFlag_CounterCameraY}, // NOTE(casey): Wall and window
    {{0.0f, -2.0f, -4.0f}, 15.0f, SceneLayerFlag_CounterCameraY}, // NOTE(casey): Icicles
    {{0.0f, 0.35f, -0.5f}, 1.0f}, // NOTE(casey): Icicles
};

global scene_layer IntroLayers4[] =
{
    {{0.0f, 0.0f, -4.1f}, 6.0f},
    {{-1.2f, -0.2f, -4.0f}, 4.0f, SceneLayerFlag_Transient, {0.0f, 0.5f}},
    {{-1.2f, -0.2f, -4.0f}, 4.0f, SceneLayerFlag_Transient, {0.5f, 1.0f}},
    {{2.25f, -1.5f, -3.0f}, 2.0f},
    {{0.0f, 0.35f, -1.0f}, 1.0f},
};

global scene_layer IntroLayers5[] =
{
    {{0.0f, 0.0f, -20.0f}, 30.0f},
    {{0.0f, 0.0f, -5.0f}, 8.0f, SceneLayerFlag_Transient, {0.0f, 0.5f}},
    {{0.0f, 0.0f, -5.0f}, 8.0f, SceneLayerFlag_Transient, {0.5f, 1.0f}},
    {{0.0f, 0.0f, -3.0f}, 4.0f, SceneLayerFlag_Transient, {0.5f, 1.0f}},
    {{0.0f, 0.0f, -2.0f}, 3.0f, SceneLayerFlag_Transient, {0.5f, 1.0f}},
};

global scene_layer IntroLayers6[] =
{
    {{0.0f, 0.0f, -8.0f}, 12.0f},
    {{0.0f, 0.0f, -5.0f}, 8.0f},
    {{1.0f, -1.0f, -3.0f}, 3.0f},
    {{0.85f, -0.95f, -2.9f}, 0.5f},
    {{-2.0f, -1.0f, -2.5f}, 2.0f},
    {{0.2f, 0.5f, -1.0f}, 1.0f},
};

global scene_layer IntroLayers7[] =
{
    {{-0.5f, 0.0f, -8.0f}, 12.0f, SceneLayerFlag_CounterCameraX},
    {{-1.0f, 0.0f, -4.0f}, 6.0f},
};

global scene_layer IntroLayers8[] =
{
    {{0.0f, 0.0f, -8.0f}, 12.0f},
    {{0.0f, -1.0f, -5.0f}, 4.0f, SceneLayerFlag_Floaty, {0.05f, 15.0f}},
    {{3.0f, -1.5f, -3.0f}, 2.0f},
    {{0.0f, 0.0f, -1.5f}, 2.5f},
};

global scene_layer IntroLayers9[] =
{
    {{0.0f, 0.0f, -8.0f}, 12.0f},
    {{0.0f, 0.25f, -3.0f}, 4.0f},
    {{1.0f, 0.0f, -2.0f}, 3.0f},
    {{1.0f, 0.1f, -1.0f}, 2.0f},
};

global scene_layer IntroLayers10[] =
{
    {{-15.0f, 25.0f, -100.0f}, 130.0f, SceneLayerFlag_AtInfinty},
    {{0.0f, 0.0f, -10.0f}, 22.0f},
    {{-0.8f, -0.2f, -3.0f}, 4.5f},
    {{0.0f, 0.0f, -2.0f}, 4.5f},
    {{0.0f, -0.25f, -1.0f}, 1.5f},
    {{0.2f, 0.2f, -0.5f}, 1.0f},
};

global scene_layer IntroLayers11[] =
{
    {{0.0f, 0.0f, -100.0f}, 150.0f, SceneLayerFlag_AtInfinty},
    {{0.0f, 10.0f, -40.0f}, 40.0f},
    {{0.0f, 3.2f, -20.0f}, 23.0f},
    {{0.25f, 0.9f, -10.0f}, 13.5f},
    {{-0.5f, 0.625f, -5.0f}, 7.0f},
    {{0.0f, 0.1f, -2.5f}, 3.9f},
    {{-0.3f, -0.15f, -1.0f}, 1.2f},
};

#define INTRO_SHOT(Index) Asset_Plate, Tag_ShotIndex##Index, ArrayCount(IntroLayers##Index), IntroLayers##Index
global layered_scene IntroCutscene[] =
{
    {Asset_None, Tag_None, 0, 0, CUTSCENE_WARMUP_SECONDS},
    {INTRO_SHOT(1), 20.0f, {0.0f, 0.0f, 10.0f}, {-4.0f, -2.0f, 5.0f}, 0.5f},
    {INTRO_SHOT(2), 20.0f, {0.0f, 0.0f, 0.0f}, {2.0f, -2.0f, -4.0f}},
    {INTRO_SHOT(3), 20.0f, {0.0f, 0.5f, 0.0f}, {0.0f, 6.5f, -1.5f}},
    {INTRO_SHOT(4), 20.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -0.5f}},
    {INTRO_SHOT(5), 20.0f, {0.0f, 0.0f, 0.0f}, {0.0f, 0.5f, -1.0f}},
    {INTRO_SHOT(6), 20.0f, {0.0f, 0.0f, 0.0f}, {-0.5f, 0.5f, -1.0f}},
    {INTRO_SHOT(7), 20.0f, {0.0f, 0.0f, 0.0f}, {2.0f, 0.0f, 0.0f}},
    {INTRO_SHOT(8), 20.0f, {0.0f, 0.0f, 0.0f}, {0.0f, -0.5f, -1.0f}},
    {INTRO_SHOT(9), 20.0f, {0.0f, 0.0f, 0.0f}, {-0.75f, -0.5f, -1.0f}},
    {INTRO_SHOT(10), 20.0f, {0.0f, 0.0f, 0.0f}, {-0.1f, 0.05f, -0.5f}},
    {INTRO_SHOT(11), 20.0f, {0.0f, 0.0f, 0.0f}, {0.6f, 0.5f, -2.0f}},
};

struct cutscene
{
    u32 SceneCount;
    layered_scene *Scenes;
};
global cutscene Cutscenes[] =
{
    {ArrayCount(IntroCutscene), IntroCutscene},
};

internal b32
RenderCutsceneAtTime(game_assets *Assets, render_group *RenderGroup, 
                     game_mode_cutscene *Cutscene, r32 tCutScene)
{
    b32 CutsceneStillRunning = false;
    
    cutscene Info = Cutscenes[Cutscene->ID];
    r32 tBase = 0.0f;
    for(u32 ShotIndex = 0;
        ShotIndex < Info.SceneCount;
        ++ShotIndex)
    {
        layered_scene *Scene = Info.Scenes + ShotIndex;
        r32 tStart = tBase;
        r32 tEnd = tStart + Scene->Duration;
        
        if((tCutScene >= tStart) &&
           (tCutScene < tEnd))
        {
            r32 tNormal = Clamp01MapToRange(tStart, tCutScene, tEnd);
            RenderLayeredScene(Assets, RenderGroup, &IntroCutscene[ShotIndex], tNormal);
            CutsceneStillRunning = true;
        }
        
        tBase = tEnd;
    }
    
    return(CutsceneStillRunning);
}

internal b32
CheckForMetaInput(game_state *GameState, game_input *Input, game_render_commands *RenderCommands)
{
    b32 Result = false;
    for(u32 ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(WasPressed(Controller->Back))
        {
            Input->QuitRequested = true;
            break;
        }
        else if(WasPressed(Controller->Start))
        {
            u32 InitialSeed = Input->Entropy;
            Input->EntropyRequested = true;
#if 0
            InitialSeed = 1234;
#endif
            PlayWorld(GameState, InitialSeed);
            Result = true;
            break;
        }
    }
    
    return(Result);
}

internal b32
UpdateAndRenderCutScene(game_state *GameState, game_render_commands *RenderCommands,
                        game_input *Input, game_mode_cutscene *CutScene)
{
    game_assets *Assets = GameState->Assets;
    
    b32 Result = CheckForMetaInput(GameState, Input, RenderCommands);
    if(!Result)
    {
        render_group RenderGroup = BeginRenderGroup(GameState->Assets, RenderCommands,
                                                    Render_Default,
                                                    V4(0, 0, 0, 0));
        EndRenderGroup(&RenderGroup);
        
        RenderGroup = BeginRenderGroup(GameState->Assets, RenderCommands,
                                       0);
        
        RenderCutsceneAtTime(Assets, 0, CutScene, CutScene->t + CUTSCENE_WARMUP_SECONDS);
        b32 CutsceneStillRunning = RenderCutsceneAtTime(Assets, &RenderGroup, CutScene, CutScene->t);
        if(CutsceneStillRunning)
        {
            CutScene->t += Input->dtForFrame;
        }
        else
        {
            PlayTitleScreen(GameState);
        }
        
        EndRenderGroup(&RenderGroup);
    }
    
    return(Result);
}

internal b32
UpdateAndRenderTitleScreen(game_state *GameState, game_render_commands *RenderCommands,
                           game_input *Input, game_mode_title_screen *TitleScreen)
{
    game_assets *Assets = GameState->Assets;
    b32 Result = CheckForMetaInput(GameState, Input, RenderCommands);
    if(!Result)
    {
        v4 BackgroundColor = {0.0f, 0.0f, 0.0f, 0.0f};
        render_group RenderGroup = BeginRenderGroup(GameState->Assets, RenderCommands,
                                                    Render_Default,
                                                    BackgroundColor);
        EndRenderGroup(&RenderGroup);
        
        // TODO(casey): We could turn off depth testing here, if we wanted to?
        RenderGroup = BeginRenderGroup(GameState->Assets, RenderCommands,
                                       0, BackgroundColor);
        if(TitleScreen->t > 10.0f)
        {
            PlayIntroCutscene(GameState);
        }
        else
        {
            f32 FocalLength = 0.78f;
            SetCameraTransform(&RenderGroup, false, FocalLength,
                               V3(1, 0, 0), V3(0, 1, 0), V3(0, 0, 1), V3(0, 0, 10.0f),
                               0.1f, 1000.0f);

            asset_tag_hash TagHash = GetTagHash(Asset_Plate, Tag_TitleScreen, Tag_ShotIndex1, Tag_LayerIndex1);
            bitmap_id LayerImage = GetBestMatchBitmapFrom(Assets, TagHash);
            
            hha_bitmap *BitmapInfo = GetBitmapInfo(RenderGroup.Assets, LayerImage);
            v2 AlignP = GetFirstAlign(BitmapInfo);
            PushBitmap(&RenderGroup, LayerImage, 10.0f, V3(0, 2.0f, 0), AlignP, V4(1, 1, 1, 1));
            
            TitleScreen->t += Input->dtForFrame;
        }
        
        EndRenderGroup(&RenderGroup);
    }
    
    return(Result);
}

internal void
PlayIntroCutscene(game_state *GameState)
{
    SetGameMode(GameState, GameMode_CutScene);
    
    game_mode_cutscene *Result = PushStruct(&GameState->ModeArena, game_mode_cutscene);
    
    Result->ID = CutsceneID_Intro;
    Result->t = 0;
    
    GameState->CutScene = Result;
}

internal void
PlayTitleScreen(game_state *GameState)
{
    SetGameMode(GameState, GameMode_TitleScreen);
    
    game_mode_title_screen *Result = PushStruct(&GameState->ModeArena, game_mode_title_screen);
    
    Result->t = 0;
    
    GameState->TitleScreen = Result;
}
