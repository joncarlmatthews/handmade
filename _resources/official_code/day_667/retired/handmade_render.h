/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct loaded_bitmap
{
    void *Memory;
    v2 AlignPercentage;
    r32 WidthOverHeight;
    s32 Width;
    s32 Height;
    // TODO(casey): Get rid of pitch!
    s32 Pitch;
    void *TextureHandle;
};

struct tile_render_work
{
    game_render_commands *Commands;
    struct loaded_bitmap *RenderTargets;
    rectangle2i ClipRect;
};

struct texture_op_allocate
{
    u32 Width;
    u32 Height;
    void *Data;
    
    void **ResultHandle;
};
struct texture_op_deallocate
{
    void *Handle;
};
struct texture_op
{
    texture_op *Next;
    b32 IsAllocate;
    union
    {
        texture_op_allocate Allocate;
        texture_op_deallocate Deallocate;
    };
};

struct camera_params
{
    f32 FocalLength;
};
inline camera_params
GetStandardCameraParams(u32 WidthInPixels, f32 FocalLength)
{
    camera_params Result;
    
    // NOTE(casey): Horizontal measurement of monitor in meters
    Result.FocalLength = FocalLength;
    
    return(Result);
}
