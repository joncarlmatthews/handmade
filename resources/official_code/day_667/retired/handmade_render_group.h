/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/* NOTE(casey):

   1) Everywhere outside the renderer, Y _always_ goes upward, X to the right.

   2) All bitmaps including the render target are assumed to be bottom-up
      (meaning that the first row pointer points to the bottom-most row
       when viewed on screen).

   3) It is mandatory that all inputs to the renderer are in world
      coordinates ("meters"), NOT pixels.  If for some reason something
      absolutely has to be specified in pixels, that will be explicitly
      marked in the API, but this should occur exceedingly sparingly.

   4) Z is a special coordinate because it is broken up into discrete slices,
      and the renderer actually understands these slices.  Z slices are what
      control the _scaling_ of things, whereas Z offsets inside a slice are
      what control Y offsetting.

   5) All color values specified to the renderer as V4's are in
      NON-premulitplied alpha.

*/

#if 0
struct environment_map
{
    loaded_bitmap LOD[4];
    real32 Pz;
};
#endif

enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_textured_quads,
    RenderGroupEntryType_render_entry_full_clear,
    RenderGroupEntryType_render_entry_depth_clear,
    RenderGroupEntryType_render_entry_begin_peels,
    RenderGroupEntryType_render_entry_end_peels,

    RenderGroupEntryType_render_entry_lighting_transfer,
};
struct render_group_entry_header
{
    u16 Type;
    
#if HANDMADE_SLOW
    u32 DebugTag;
#endif
};

enum camera_transform_flag
{
    Camera_IsOrthographic = 0x1,
    Camera_IsDebug = 0x2,
};

struct render_setup
{
    rectangle2i ClipRect;
    u32 RenderTargetIndex;
    m4x4 Proj;
    v3 CameraP;
    v3 FogDirection;
    v3 FogColor;
    f32 FogStartDistance;
    f32 FogEndDistance;
    f32 ClipAlphaStartDistance;
    f32 ClipAlphaEndDistance;
};

struct render_entry_textured_quads
{
    render_setup Setup;
    u32 QuadCount;
    u32 VertexArrayOffset; // NOTE(casey): Uses 4 vertices per quad
};

struct render_entry_blend_render_target
{
    u32 SourceTargetIndex;
    r32 Alpha;
};

struct render_entry_full_clear
{
    v4 ClearColor; // NOTE(casey): This color is NOT in linear space, it is in sRGB space directly?
};

struct render_entry_begin_peels
{
    v4 ClearColor; // NOTE(casey): This color is NOT in linear space, it is in sRGB space directly?
};

struct render_entry_lighting_transfer
{
    v4 *LightData0;
    v4 *LightData1;
};
    
struct object_transform // TODO(casey): This may have become irrelevant!
{
    // TODO(casey): Move this out to its own thang
    b32 Upright;
    v3 OffsetP;
    r32 Scale;
};

struct camera_transform
{
    b32 Orthographic;
    r32 FocalLength;
    v3 CameraP;
};

struct render_transform
{
    v3 P;
    v3 X;
    v3 Y;
    v3 Z;
    
    // NOTE(casey): This is both the world camera transform _and_ the projection
    // matrix combined!
    m4x4_inv Proj;
};

struct render_group
{
    struct game_assets *Assets;
    
    v2 ScreenDim;
    
    b32 LightingEnabled;
    rectangle3 LightBounds;
    
#if HANDMADE_SLOW
    u32 DebugTag;
#endif
    
    uint32 MissingResourceCount;

    render_setup LastSetup;
    render_transform GameXForm;
    render_transform DebugXForm;
    
    u32 GenerationID;
    game_render_commands *Commands;
    
    render_entry_textured_quads *CurrentQuads;
};

struct used_bitmap_dim
{
    v3 BasisP;
    v2 Size;
    v2 Align;
    v3 P;
};

struct push_buffer_result
{
    render_group_entry_header *Header;
};

void DrawRectangleQuickly(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                          loaded_bitmap *Texture, real32 PixelsToMeters,
                          rectangle2i ClipRect);

inline object_transform
DefaultUprightTransform(void)
{
    object_transform Result = {};

    Result.Upright = true;
    Result.Scale = 1.0f;

    return(Result);
}

inline object_transform
DefaultFlatTransform(void)
{
    object_transform Result = {};

    Result.Scale = 1.0f;

    return(Result);
}
