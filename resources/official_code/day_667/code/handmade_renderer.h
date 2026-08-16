/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
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

/* TODO(casey):

   So, we can now see where the renderer is heading and we should take a moment
   to clean things up to the point where it actually happens:
   
   We should just have the opening of a quad set happen inside the entity
   renderer, and it should stream out quads directly, using utility functions
   to write them directly into the quad stream.
*/

// TODO(casey): This should be parameterized at some point, rather than constant

#define MAX_LIGHT_BOX_COUNT (2*8192)
#define MAX_LIGHT_OCCLUDER_COUNT (2*8192)

#define LIGHT_LOOKUP_VOXEL_DIM_X (28)
#define LIGHT_LOOKUP_VOXEL_DIM_Y (20)
#define LIGHT_LOOKUP_VOXEL_DIM_Z 2
#define MAX_LIGHT_LOOKUP_VOXEL_DIM 64

#define LIGHTING_OCTAHEDRAL_MAP_DIM 8

#define TEXTURE_ARRAY_DIM 512
#define TEXTURE_SPECIAL_BIT 0x80000000

#define MAX_LIGHT_INTENSITY 10.0

#define LIGHT_TEST_ACCUMULATION_COUNT 1024

struct platform_renderer;
struct game_render_commands;

#define RENDERER_BEGIN_FRAME(name) game_render_commands *name(platform_renderer *Renderer, v2u OSWindowDim, v2u RenderDim, rectangle2i DrawRegion)
#define RENDERER_END_FRAME(name) void name(platform_renderer *Renderer, game_render_commands *Frame)

typedef RENDERER_BEGIN_FRAME(renderer_begin_frame);
typedef RENDERER_END_FRAME(renderer_end_frame);

union renderer_texture
{
    u64 Packed;
    struct
    {
        // NOTE(casey): You could pack an offset in here if you wanted!  Just use
        // a 16-bit index.
        u32 Index;
        u16 Width;
        u16 Height;
    };
};

enum texture_op_state
{
    TextureOp_Empty,
    TextureOp_PendingLoad,
    TextureOp_ReadyToTransfer,
};
struct texture_op
{
    renderer_texture Texture;
    void *Data;
    b32 GenMipMaps;
    u32 TransferMemoryLastUsed;
    volatile texture_op_state State;
};

struct renderer_texture_queue
{
    u32 TransferMemoryCount;
    u32 TransferMemoryFirstUsed;
    u32 TransferMemoryLastUsed;
    u8 *TransferMemory;
    
    u32 OpCount;
    u32 FirstOpIndex;
    texture_op Ops[256];
};

struct platform_renderer_limits
{
    u32 MaxQuadCountPerFrame;
    u32 MaxTextureCount;
    u32 MaxSpecialTextureCount;
    u32 TextureTransferBufferSize;
};

struct platform_renderer
{
    renderer_texture_queue TextureQueue;
    
    umm TotalTextureMemory;
    umm TotalFramebufferMemory; // TODO(casey): Reenable this!
    u32 UsedMultisampleCount;
    
    // NOTE(casey): This is currently unused by us, but we've added it for
    // other platform maintainers so they can put some context pointers
    // in here for their own use.
    void *Platform;
};

struct lighting_box
{
    v3 BoxMin, BoxMax;
    
    // TODO(casey): In the future, this should be packed down to 64-bits total
    // {
    v3 RefC;
    f32 Emission;
    // }
};

enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_textured_quads,
    RenderGroupEntryType_render_entry_full_clear,
    RenderGroupEntryType_render_entry_depth_clear,
    RenderGroupEntryType_render_entry_begin_peels,
    RenderGroupEntryType_render_entry_end_peels,
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

struct fog_params
{
    // TODO(casey): Support v3 FogP!
    
    v3 Direction;
    f32 StartDistance;
    f32 EndDistance;
};

struct alpha_clip_params
{
    f32 dStartDistance;
    f32 dEndDistance;
};

struct render_setup
{
    rectangle2 ClipRect;
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
    u32 IndexArrayOffset; // NOTE(casey): Uses 6 indices per quad
    
    // NOTE(casey): Textures is 0 if using the default texture array / single batch render,
    // and an array of one texture per quad if not.
    renderer_texture *QuadTextures;
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

struct camera_transform
{
    b32 Orthographic;
    r32 FocalLength;
    v3 CameraP;
};

struct cube_uv_layout
{
    u16 Encoding;
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
    
    b32 LightingEnabled;
    //rectangle3 LightBounds;
    
#if HANDMADE_SLOW
    u32 DebugTag;
#endif
    
    u32 Flags;
    u32 MissingResourceCount;
    
    v3 WorldUp;
    render_setup LastSetup;
    render_transform GameXForm;
    render_transform DebugXForm;
    
    struct game_render_commands *Commands;
    
    render_entry_textured_quads *CurrentQuads;
    
    // NOTE(casey): The app must supply this, otherwise it will not be set
    renderer_texture WhiteTexture;
};

struct used_bitmap_dim
{
    v2 Size;
    v2 Align;
    v3 P;
};

struct push_buffer_result
{
    render_group_entry_header *Header;
};

struct textured_vertex
{
    v4 P;
    v3 N;
    v2 UV; // TODO(casey): Convert this down to 8-bits?
    u32 Color; // NOTE(casey): Packed RGBA in memory order (ABGR in little-endian)
    
    // TODO(casey): This doesn't need to be per-vertex, we could use a per-triangle stream
    u16 TextureIndex;
    u8 Emission; 
    u8 Tone; 
};

struct game_render_settings
{
    // NOTE(casey): The RenderDim is the actual size of all our render backbuffers
    v2u RenderDim;
    
    u32 DepthPeelCountHint;
    b32 MultisamplingDebug;
    b32 MultisamplingHint;
    b32 PixelationHint;
    b32 LightingDisabled;
    b32 RequestVSync;
    
    b32 NearestTexelFiltering;
    b32 NoMIPMaps;
    
    // NOTE(casey): SettingsVersion is a monotonically increasing value that
    // gets incremented whenever the renderer gets reloaded or another event
    // occurs that might want the renderer to reinitialize without actually
    // changing any of the settings above.
    u32 Version;
};

struct game_render_commands
{
    game_render_settings Settings;
    
    // NOTE(casey): OSWindowWidth/Height should NOT be used for anything drawing-related!
    // They are just there to let you know the shape of the container window into
    // which you are drawing.  DrawRegion contains the actual width/height you would
    // use for drawing.
    v2u OSWindowDim;
    rectangle2i OSDrawRegion; // NOTE(casey): This is the subsection of the window to which we will draw
    
    u32 MaxPushBufferSize;
    u8 *PushBufferBase;
    u8 *PushBufferDataAt;
    
    u32 MaxVertexCount;
    u32 VertexCount;
    textured_vertex *VertexArray;
    
    u32 MaxIndexCount;
    u32 IndexCount;
    u16 *IndexArray;
    
    v3 LightingVoxelMinCorner;
    v3 LightingVoxelInvTotalDim;
    
    light_atlas DiffuseLightAtlas;
    light_atlas SpecularLightAtlas;
    
    u32 MaxLightOccluderCount;
    u32 LightOccluderCount;
    lighting_box *LightOccluders;
    
    u32 MaxQuadTextureCount;
    u32 QuadTextureCount;
    renderer_texture *QuadTextures;
};

struct indexed_vertex_output
{
    textured_vertex *VertexAt;
    u16 *IndexAt;
    
    u16 BaseIndex;
};

enum render_group_flags
{
    Render_ClearColor = 0x1,
    Render_ClearDepth = 0x2,
    Render_HandleTransparency = 0x4,
    
    //
    
    Render_Default = (Render_ClearColor|Render_ClearDepth|Render_HandleTransparency),
};

inline b32x
AreEqual(game_render_settings *A, game_render_settings *B)
{
    b32x Result = MemoryIsEqual(sizeof(*A), A, B);
    return(Result);
}

internal b32x
IsValid(renderer_texture Texture)
{
    b32x Result = (Texture.Packed != 0);
    return(Result);
}

internal renderer_texture
ReferToTexture(u32 Index, u32 Width, u32 Height)
{
    renderer_texture Result;
    
    Result.Index = Index;
    Result.Width = (u16)Width;
    Result.Height = (u16)Height;
    
    Assert(Result.Index == Index);
    Assert(Result.Width == Width);
    Assert(Result.Height == Height);
    
    return(Result);
}

internal b32x
IsSpecialTexture(renderer_texture Texture)
{
    b32x Result = (Texture.Index & TEXTURE_SPECIAL_BIT);
    return(Result);
}

internal u32
SpecialTextureIndexFrom(u32 Index)
{
    Assert((Index & TEXTURE_SPECIAL_BIT) == 0);
    
    u32 Result = (Index | TEXTURE_SPECIAL_BIT);
    return(Result);
}

internal u32
TextureIndexFrom(renderer_texture Texture)
{
    u32 Result = (Texture.Index & ~TEXTURE_SPECIAL_BIT);
    return(Result);
}

internal void
InitTextureQueue(renderer_texture_queue *Queue, u32 RequestTransferBufferSize, void *Memory)
{
    Queue->TransferMemoryCount = RequestTransferBufferSize;
    Queue->TransferMemoryFirstUsed = 0;
    Queue->TransferMemoryLastUsed = 0;
    Queue->TransferMemory = (u8 *)Memory;
    
    Queue->OpCount = 0;
    Queue->FirstOpIndex = 0;
}