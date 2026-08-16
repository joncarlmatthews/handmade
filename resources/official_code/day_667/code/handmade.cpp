/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "handmade.h"
#include "handmade_stream.cpp"
#include "handmade_file_formats.cpp"
#include "handmade_sort.cpp"
#include "handmade_voxel.cpp"
#include "handmade_light_atlas.cpp"
#include "handmade_renderer.cpp"
#include "handmade_renderer_geometry.cpp"
#include "handmade_diagram.cpp"
#include "handmade_camera.cpp"
#include "handmade_asset_rendering.cpp"
#include "handmade_lighting.cpp"
#include "handmade_riff.cpp"
#include "handmade_image.cpp"
#include "handmade_png.cpp"
#include "handmade_wav.cpp"
#include "handmade_import.cpp"
#include "handmade_asset.cpp"
#include "handmade_audio.cpp"
#include "handmade_box.cpp"
#include "handmade_world.cpp"
#include "handmade_particles.cpp"
#include "handmade_brain.cpp"
#include "handmade_entity.cpp"
#include "handmade_world_mode.cpp"
#include "handmade_entity_gen.cpp"
#include "handmade_world_gen.cpp"
#include "handmade_room_gen.cpp"
#include "handmade_cutscene.cpp"
#include "handmade_dev_ui.cpp"
#include "handmade_in_game_editor.cpp"
#include "handmade_tokenizer.cpp"

internal void MarkBit(u64 *Array, umm Index)
{
    umm OccIndex = Index / 64;
    umm BitIndex = Index % 64;
    Array[OccIndex] |= ((u64)1 << BitIndex);
}

internal b32x IsEmpty(u64 *Array, umm Index)
{
    umm OccIndex = Index / 64;
    umm BitIndex = Index % 64;
    b32x Result = !(Array[OccIndex] & ((u64)1 << BitIndex));
    return(Result);
}

internal task_with_memory *
BeginTaskWithMemory(game_state *GameState, b32 DependsOnGameMode)
{
    task_with_memory *FoundTask = 0;

    for(uint32 TaskIndex = 0;
        TaskIndex < ArrayCount(GameState->Tasks);
        ++TaskIndex)
    {
        task_with_memory *Task = GameState->Tasks + TaskIndex;
        if(!Task->BeingUsed)
        {
            FoundTask = Task;
            Task->BeingUsed = true;
            Task->DependsOnGameMode = DependsOnGameMode;
            Task->MemoryFlush = BeginTemporaryMemory(&Task->Arena);
            break;
        }
    }

    return(FoundTask);
}

internal void
EndTaskWithMemory(task_with_memory *Task)
{
    EndTemporaryMemory(Task->MemoryFlush);

    CompletePreviousWritesBeforeFutureWrites;
    Task->BeingUsed = false;
}

internal game_assets *
DEBUGGetGameAssets(game_memory *Memory)
{
    game_assets *Assets = 0;

    game_state *GameState = Memory->GameState;
    if(GameState)
    {
        Assets = GameState->Assets;
    }

    return(Assets);
}

internal void
DEBUGDumpData(char *FileName, umm DumpSize, void *DumpData)
{
    platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_Dump);
    platform_file_info *DumpFile = Platform.GetFileByPath(&FileGroup, FileName, OpenFile_Write);
    platform_file_handle FileHandle = Platform.OpenFile(&FileGroup, DumpFile, OpenFile_Write);
    Platform.WriteDataToFile(&FileHandle, 0, DumpSize, DumpData);
    Platform.SetFileSize(&FileHandle, DumpSize);
    Platform.CloseFile(&FileHandle);
    Platform.GetAllFilesOfTypeEnd(&FileGroup);
}

internal void
SetGameMode(game_state *GameState, game_mode GameMode)
{
    b32 NeedToWait = false;
    for(u32 TaskIndex = 0;
        TaskIndex < ArrayCount(GameState->Tasks);
        ++TaskIndex)
    {
        NeedToWait = NeedToWait || GameState->Tasks[TaskIndex].DependsOnGameMode;
    }
    if(NeedToWait)
    {
        Platform.CompleteAllWork(GameState->LowPriorityQueue);
    }
    Clear(&GameState->ModeArena);
    GameState->GameMode = GameMode;
}

#if HANDMADE_INTERNAL
debug_table *GlobalDebugTable;
game_memory *DebugGlobalMemory;
#endif
platform_api Platform;
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Platform = Memory->PlatformAPI;

#if HANDMADE_INTERNAL
    GlobalDebugTable = Memory->DebugTable;
    DebugGlobalMemory = Memory;

    {DEBUG_DATA_BLOCK("Renderer");
        DEBUG_UI_HUD(DevMode_rendering);
        DEBUG_B32(RenderCommands->Settings.MultisamplingDebug);
        DEBUG_B32(RenderCommands->Settings.MultisamplingHint);
        DEBUG_B32(RenderCommands->Settings.PixelationHint);
        DEBUG_B32(RenderCommands->Settings.LightingDisabled);
        DEBUG_B32(RenderCommands->Settings.NearestTexelFiltering);
        DEBUG_B32(RenderCommands->Settings.NoMIPMaps);
        DEBUG_B32(RenderCommands->Settings.RequestVSync);
        DEBUG_B32(Global_Renderer_TestWeirdDrawBufferSize);
    }
    // TODO(casey): Had to move this out into its own block to avoid MSVC 2015 warning -
    // probably should eventually fix the macros to generate unique names to avoid
    // enclosed blocks complaining of reused DebugBlock__ name...
    {DEBUG_DATA_BLOCK("Renderer/Camera");
        DEBUG_B32(Global_Renderer_Camera_UseDebug);
        DEBUG_VALUE(Global_Renderer_Camera_DebugDistance);
        DEBUG_B32(Global_Renderer_Camera_RoomBased);
    }
    {DEBUG_DATA_BLOCK("AI/Familiar");
        DEBUG_B32(Global_AI_Familiar_FollowsHero);
    }
    {DEBUG_DATA_BLOCK("Particles");
        DEBUG_B32(Global_Particles_Test);
        DEBUG_B32(Global_Particles_ShowGrid);
    }
    {DEBUG_DATA_BLOCK("Simulation");
        DEBUG_VALUE(Global_Timestep_Percentage);
        DEBUG_B32(Global_Simulation_UseSpaceOutlines);
    }
    {DEBUG_DATA_BLOCK("Profile");
        DEBUG_UI_HUD(DevMode_profiling);
        DEBUG_UI_ELEMENT(DebugType_FrameSlider, FrameSlider);
        DEBUG_UI_ELEMENT(DebugType_LastFrameInfo, LastFrame);
        DEBUG_UI_ELEMENT(DebugType_TopClocksList, GameUpdateAndRender);
    }
    {DEBUG_DATA_BLOCK("Memory");
        DEBUG_UI_HUD(DevMode_memory);
        DEBUG_UI_ELEMENT(DebugType_MemoryBySize, DebugMemory);
    }

#endif
    TIMED_FUNCTION();

    // NOTE(casey): Clamp the dt for the frame so that we don't have too high
    // or too low frame rates in any circumstance.
    if(Input->dtForFrame > 0.1f)
    {
        Input->dtForFrame = 0.1f;
        // TODO(casey): Warn on out-of-range refresh
    }
    else if(Input->dtForFrame < 0.001f)
    {
        Input->dtForFrame = 0.001f;
        // TODO(casey): Warn on out-of-range refresh
    }

    Input->dtForFrame *= Global_Timestep_Percentage / 100.0f;

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));

    game_state *GameState = Memory->GameState;
    if(!GameState)
    {
        GameState = Memory->GameState = BootstrapPushStruct(game_state, TotalArena);
        GameState->FrameArena = (memory_arena *)BootstrapPushSize_(DEBUG_MEMORY_NAME("FrameArena") sizeof(memory_arena),
                                                                   0, NonRestoredArena());
        GameState->FrameArenaTemp = BeginTemporaryMemory(GameState->FrameArena);

        InitializeAudioState(&GameState->AudioState, &GameState->AudioArena);

        GameState->HighPriorityQueue = Memory->HighPriorityQueue;
        GameState->LowPriorityQueue = Memory->LowPriorityQueue;
        for(uint32 TaskIndex = 0;
            TaskIndex < ArrayCount(GameState->Tasks);
            ++TaskIndex)
        {
            task_with_memory *Task = GameState->Tasks + TaskIndex;
            Task->BeingUsed = false;
        }

        GameState->Assets = AllocateGameAssets(Megabytes(256), GameState, Memory->TextureQueue);

        InitializeUI(&GameState->DevUI, GameState->Assets);
        InitializeEditor(&GameState->Editor, GameState->Assets);
    }

    EndTemporaryMemory(GameState->FrameArenaTemp);
    GameState->FrameArenaTemp = BeginTemporaryMemory(GameState->FrameArena);

    if(Input->FKeyPressed[1])
    {
        GameState->DevMode = DevMode_none;
    }
    else if(Input->FKeyPressed[2])
    {
        GameState->DevMode = DevMode_camera;
    }
    else if(Input->FKeyPressed[3])
    {
        GameState->DevMode = DevMode_editing_assets;
    }
    else if(Input->FKeyPressed[4])
    {
        GameState->DevMode = DevMode_editing_hha;
    }
    else if(Input->FKeyPressed[5])
    {
        GameState->DevMode = DevMode_profiling;
    }
    else if(Input->FKeyPressed[6])
    {
        GameState->DevMode = DevMode_rendering;
    }
    else if(Input->FKeyPressed[7])
    {
        GameState->DevMode = DevMode_lighting;
    }
    else if(Input->FKeyPressed[8])
    {
        GameState->DevMode = DevMode_memory;
    }
    else if(Input->FKeyPressed[9])
    {
        GameState->DevMode = DevMode_collision;
        if(Input->ShiftDown)
        {
            DIAGRAM_Capture(true);
        }
    }
    
    DEBUG_ARENA_NAME(&GameState->TotalArena, "Game");
    DEBUG_ARENA_NAME(&GameState->ModeArena, "Game Mode");
    DEBUG_ARENA_NAME(&GameState->AudioArena, "Audio Playback");
    DEBUG_ARENA_NAME(GameState->FrameArena, "Frame Temporary");
    DEBUG_ARENA_NAME(&GameState->Assets->NonRestoredMemory, "Asset Storage");

    if(GameState->GameMode == GameMode_None)
    {
        PlayIntroCutscene(GameState);
#if 1
        // NOTE(casey): This jumps straight to the game, no title sequence
        game_controller_input *Controller = GetController(Input, 0);
        Controller->Start.EndedDown = true;
        Controller->Start.HalfTransitionCount = 1;
#endif
    }

    //
    // NOTE(casey):
    //
#if 1
    if(!GameState->Music)
    {
        asset_match_vector Match = {};
        Match.E[MatchElement_Channel] = 0;
        sound_id Channel0ID  = GetBestMatchSoundFrom(GameState->Assets, GetTagHash(Asset_Audio, Tag_TitleScreen));
        Match.E[MatchElement_Channel] = 1;
        sound_id Channel1ID  = GetBestMatchSoundFrom(GameState->Assets, GetTagHash(Asset_Audio, Tag_TitleScreen));
        
        GameState->Music = PlaySound(&GameState->AudioState, Channel0ID);
    }

    {
        v2 MusicVolume;
        MusicVolume.y = Clamp01(Input->ClipSpaceMouseP.x);
        MusicVolume.x = 1.0f - MusicVolume.y;
        ChangeVolume(&GameState->AudioState, GameState->Music, 0.01f, MusicVolume);
    }
#endif

    editable_hit_test HitTest = BeginHitTest(&GameState->Editor, Input, GameState->DevMode);

    b32 Rerun = false;
    do
    {
        switch(GameState->GameMode)
        {
            case GameMode_TitleScreen:
            {
                Rerun = UpdateAndRenderTitleScreen(GameState, RenderCommands,
                                                   Input, GameState->TitleScreen);
            } break;

            case GameMode_CutScene:
            {
                Rerun = UpdateAndRenderCutScene(GameState, RenderCommands,
                                                Input, GameState->CutScene);
            } break;

            case GameMode_World:
            {
                Rerun = UpdateAndRenderWorld(GameState, GameState->WorldMode,
                                             Input, RenderCommands, &HitTest);
            } break;

            InvalidDefaultCase;
        }
    } while(Rerun);

    EndHitTest(&GameState->Editor, Input, &HitTest);
        
    BeginUIFrame(&GameState->DevUI, GameState->Assets, RenderCommands, Input);
    UpdateAndRenderEditor(&GameState->Editor, &GameState->DevUI, GameState);
    EditorInteract(&GameState->Editor, &GameState->DevUI, Input);
    EndUIFrame(&GameState->DevUI);
    
    DIAGRAM_Reset();

#if 0
    if(!HeroesExist && QuitRequested)
    {
        Memory->QuitRequested = true;
    }
#endif

    CheckArena(&GameState->ModeArena);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = Memory->GameState;

#if 0 TEMPORARY
    OutputPlayingSounds(&GameState->AudioState, SoundBuffer, GameState->Assets, GameState->FrameArena);
#endif
}

#if HANDMADE_INTERNAL
#include "handmade_debug.cpp"
#else
extern "C" DEBUG_GAME_FRAME_END(DEBUGGameFrameEnd)
{
}
#endif

