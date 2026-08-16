/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define _CRT_SECURE_NO_WARNINGS 1

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include "handmade_types.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_shared.h"
#include "handmade_light_atlas.h"
#include "handmade_renderer.h"
#include "handmade_camera.h"

enum test_scene_element_type
{
    Element_Grass,
    Element_Tree,
    Element_Wall,
};
struct test_scene_element
{
    test_scene_element_type Type;
    cube_uv_layout CubeUVLayout;
};
#define TEST_SCENE_DIM_X 40
#define TEST_SCENE_DIM_Y 50
struct test_scene
{
    v3 MinP;
    test_scene_element Elements[TEST_SCENE_DIM_Y][TEST_SCENE_DIM_X];
    
    renderer_texture GrassTexture;
    renderer_texture WallTexture;
    renderer_texture TreeTexture;
    renderer_texture HeadTexture;
    renderer_texture CoverTexture;
};

//
//
//
//
//
// NOTE(casey): Everything below here is just for the support routines that
// open Win32 windows and load BMPs.
//
//
//
//
//

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    s32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    s32 HorzResolution;
    s32 VertResolution;
    u32 ColorsUsed;
    u32 ColorsImportant;
    
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)

struct loaded_bitmap
{
    void *Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
};

struct entire_file
{
    u32 ContentsSize;
    void *Contents;
};

struct frame_stats
{
    LARGE_INTEGER PerfCountFrequencyResult;
    LARGE_INTEGER LastCounter;
    
    f32 MinSPF;
    f32 MaxSPF;
    u32 DisplayCounter;
};

internal renderer_texture LoadBMP(renderer_texture_queue *TextureOpQueue, char *FileName,
                                  u16 TextureIndex);
internal void *Win32AllocateMemory(umm Size);

internal frame_stats InitFrameStats(void);
internal f32 UpdateFrameStats(frame_stats *Stats);

