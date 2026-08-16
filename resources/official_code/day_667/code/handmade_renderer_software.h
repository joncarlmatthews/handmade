/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct software_texture
{
    void *Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
};

struct tile_render_work
{
    game_render_commands *Commands;
    struct software_texture *RenderTargets;
    rectangle2i ClipRect;
};

