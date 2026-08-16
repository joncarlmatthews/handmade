/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

global b32 Global_Renderer_ShowLightingSamples = false;
inline void PushSetup(render_group *Group, render_setup *NewSetup);

struct transient_clip_rect
{
    transient_clip_rect(render_group *RenderGroupInit, rectangle2 NewClipRect)
    {
        RenderGroup = RenderGroupInit;
        
        render_setup NewSetup = RenderGroup->LastSetup;
        OldClipRect = NewSetup.ClipRect;
        NewSetup.ClipRect = NewClipRect;
        PushSetup(RenderGroup, &NewSetup);
    }
    
    transient_clip_rect(render_group *RenderGroupInit)
    {
        RenderGroup = RenderGroupInit;
        OldClipRect = RenderGroup->LastSetup.ClipRect;
    }
    
    ~transient_clip_rect(void)
    {
        render_setup NewSetup = RenderGroup->LastSetup;
        NewSetup.ClipRect = OldClipRect;
        PushSetup(RenderGroup, &NewSetup);
    }
    
    render_group *RenderGroup;
    rectangle2 OldClipRect;
};

internal void
Clear(renderer_texture *Texture)
{
    Texture->Index = 0;
    Texture->Width = 0;
    Texture->Height = 0;
}

inline v4
UnscaleAndBiasNormal(v4 Normal)
{
    v4 Result;
    
    real32 Inv255 = 1.0f / 255.0f;
    
    Result.x = -1.0f + 2.0f*(Inv255*Normal.x);
    Result.y = -1.0f + 2.0f*(Inv255*Normal.y);
    Result.z = -1.0f + 2.0f*(Inv255*Normal.z);
    
    Result.w = Inv255*Normal.w;
    
    return(Result);
}

inline push_buffer_result
PushRenderBuffer(render_group *RenderGroup, u32 DataSize)
{
    game_render_commands *Commands = RenderGroup->Commands;
    
    push_buffer_result Result = {};
    
    u8 *PushBufferEnd = Commands->PushBufferBase + Commands->MaxPushBufferSize;
    if((Commands->PushBufferDataAt + DataSize) <= PushBufferEnd)
    {
        Result.Header = (render_group_entry_header *)Commands->PushBufferDataAt;
        Commands->PushBufferDataAt += DataSize;
    }
    else
    {
        InvalidCodePath;
    }
    
    return(Result);
}

#define PushRenderElement(Group, type) (type *)PushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)
inline void *
PushRenderElement_(render_group *Group, uint32 Size, render_group_entry_type Type)
{
    game_render_commands *Commands = Group->Commands;
    
    void *Result = 0;
    
    Size += sizeof(render_group_entry_header);
    push_buffer_result Push = PushRenderBuffer(Group, Size);
    if(Push.Header)
    {
        render_group_entry_header *Header = Push.Header;
        Header->Type = (u16)Type;
        Result = (uint8 *)Header + sizeof(*Header);
#if HANDMADE_SLOW
        Header->DebugTag = Group->DebugTag;
#endif
    }
    else
    {
        InvalidCodePath;
    }
    
    Group->CurrentQuads = 0;
    
    return(Result);
}

inline v4
StoreColor(v4 Source)
{
    v4 Dest;
    Dest.a = Source.a;
    Dest.r = Dest.a*Source.r;
    Dest.g = Dest.a*Source.g;
    Dest.b = Dest.a*Source.b;
    
    return(Dest);
}

internal u32
FinalizeColor(v4 Unpremultiplied)
{
    v4 PremulColor = StoreColor(Unpremultiplied);
    u32 Result = RGBAPack4x8(255.0f*PremulColor);
    return(Result);
}

internal u32
FinalizeColor(v3 Unpremultiplied)
{
    u32 Result = FinalizeColor(V4(Unpremultiplied, 1.0f));
    return(Result);
}

inline render_entry_textured_quads *
GetCurrentQuads(render_group *Group, u32 QuadCount, renderer_texture Texture)
{
    game_render_commands *Commands = Group->Commands;
    
    u32 VertexCount = QuadCount*4;
    u32 IndexCount = QuadCount*6;
    
    u32 MaxQuadsPerBatch = (U16Max - 1)/4;
    
    if(Group->CurrentQuads && ((Group->CurrentQuads->QuadCount + QuadCount) > MaxQuadsPerBatch))
    {
        Group->CurrentQuads = 0;
    }
    
    b32x ThisTextures = IsSpecialTexture(Texture);
    if(Group->CurrentQuads)
    {
        b32x CurrentTextures = (Group->CurrentQuads->QuadTextures != 0);
        if(CurrentTextures != ThisTextures)
        {
            Group->CurrentQuads = 0;
        }
    }
    
    if(!Group->CurrentQuads)
    {
        Group->CurrentQuads = (render_entry_textured_quads *)
            PushRenderElement_(Group, sizeof(render_entry_textured_quads), RenderGroupEntryType_render_entry_textured_quads);
        Group->CurrentQuads->QuadCount = 0;
        Group->CurrentQuads->VertexArrayOffset = Commands->VertexCount;
        Group->CurrentQuads->IndexArrayOffset = Commands->IndexCount;
        Group->CurrentQuads->Setup = Group->LastSetup;
        Group->CurrentQuads->QuadTextures = 0;
        if(ThisTextures)
        {
            Group->CurrentQuads->QuadTextures =
                (Commands->QuadTextures + Commands->QuadTextureCount);
        }
    }
    
    render_entry_textured_quads *Result = Group->CurrentQuads;
    if(((Commands->VertexCount + VertexCount) > Commands->MaxVertexCount) &&
       ((Commands->IndexCount + IndexCount) > Commands->MaxIndexCount) &&
       (ThisTextures &&
        ((Commands->QuadTextureCount + QuadCount) > Commands->MaxQuadTextureCount)))
    {
        Result = 0;
        InvalidCodePath;
    }
    
    return(Result);
}

internal v2
GetUVScaleForRegularTexture(renderer_texture Texture)
{
    v2 Result =
    {
        (f32)Texture.Width / (f32)TEXTURE_ARRAY_DIM,
        (f32)Texture.Height / (f32)TEXTURE_ARRAY_DIM
    };
    
    return(Result);
}

inline void
PushQuad(render_group *Group, renderer_texture Texture,
         v4 P0, v2 UV0, u32 C0,
         v4 P1, v2 UV1, u32 C1,
         v4 P2, v2 UV2, u32 C2,
         v4 P3, v2 UV3, u32 C3,
         f32 Emission = 0.0f)
{
    Assert(Emission >= 0.0f);
    Assert(Emission <= 1.0f);
    
    game_render_commands *Commands = Group->Commands;
    
    render_entry_textured_quads *Entry = Group->CurrentQuads;
    Assert(Entry);
    
    ++Entry->QuadCount;
    
    v2 InvUV = GetUVScaleForRegularTexture(Texture);
    
    u32 TextureIndex32 = TextureIndexFrom(Texture);
    u16 TextureIndex = (u16)TextureIndex32;
    Assert(TextureIndex == TextureIndex32);
    if(IsSpecialTexture(Texture))
    {
        InvUV.x = 1.0f;
        InvUV.y = 1.0f;
        Commands->QuadTextures[Commands->QuadTextureCount++] = Texture;
        TextureIndex = 0;
    }
    
    u32 VertIndex = Commands->VertexCount;
    u32 IndexIndex = Commands->IndexCount;
    Commands->VertexCount += 4;
    Commands->IndexCount += 6;
    Assert(Commands->VertexCount <= Commands->MaxVertexCount);
    Assert(Commands->IndexCount <= Commands->MaxIndexCount);
    Assert(Commands->QuadTextureCount <= Commands->MaxQuadTextureCount);
    
    textured_vertex *Vert = Commands->VertexArray + VertIndex;
    u16 *Index = Commands->IndexArray + IndexIndex;
    
    UV0 = Hadamard(InvUV, UV0);
    UV1 = Hadamard(InvUV, UV1);
    UV2 = Hadamard(InvUV, UV2);
    UV3 = Hadamard(InvUV, UV3);
    
    // TODO(casey): Stop computing the normal, we could just pass it down directly!
    v3 N0, N1, N2, N3;
    
    v4 E10 = (P1 - P0);
    E10.z += E10.w;
    
    v4 E20 = (P2 - P0);
    E20.z += E20.w;
    
    v3 NormalDirection = Cross(E10.xyz, E20.xyz);
    v3 Normal = NOZ(NormalDirection);
    
    u8 VertTone = 255;
    if(!Group->LightingEnabled)
    {
        Emission = 1.0f;
        VertTone = 0;
    }
    u8 VertEmission = (u8)RoundReal32ToInt32(Clamp01(Emission)*255.0f);
    
    N0 = N1 = N2 = N3 = Normal;
    
    Vert[0].P = P3;
    Vert[0].N = N3;
    Vert[0].UV = UV3;
    Vert[0].Color = C3;
    Vert[0].TextureIndex = TextureIndex;
    Vert[0].Emission = VertEmission;
    Vert[0].Tone = VertTone;
    
    Vert[1].P = P0;
    Vert[1].N = N0;
    Vert[1].UV = UV0;
    Vert[1].Color = C0;
    Vert[1].TextureIndex = TextureIndex;
    Vert[1].Emission = VertEmission;
    Vert[1].Tone = VertTone;
    
    Vert[2].P = P2;
    Vert[2].N = N2;
    Vert[2].UV = UV2;
    Vert[2].Color = C2;
    Vert[2].TextureIndex = TextureIndex;
    Vert[2].Emission = VertEmission;
    Vert[2].Tone = VertTone;
    
    Vert[3].P = P1;
    Vert[3].N = N1;
    Vert[3].UV = UV1;
    Vert[3].Color = C1;
    Vert[3].TextureIndex = TextureIndex;
    Vert[3].Emission = VertEmission;
    Vert[3].Tone = VertTone;
    
    u32 BaseIndex = (VertIndex - Entry->VertexArrayOffset);
    u16 VI = (u16)BaseIndex;
    Assert((u32)VI == BaseIndex);
    
    Index[0] = VI + 0;
    Index[1] = VI + 1;
    Index[2] = VI + 2;
    Index[3] = VI + 1;
    Index[4] = VI + 3;
    Index[5] = VI + 2;
}

inline void
PushLineSegment(render_group *Group, renderer_texture Texture,
                v3 FromP, v4 FromCF, v3 ToP, v4 ToCF, f32 Thickness)
{
    u32 FromC = RGBAPack4x8(255.0f*FromCF);
    u32 ToC = RGBAPack4x8(255.0f*ToCF);
    
    v3 Line = ToP - FromP;
    
#if 1
    v3 CamZ = Group->DebugXForm.Z;
    Line -= Inner(CamZ, Line)*CamZ;
    v3 LinePerp = Cross(CamZ, Line);
    f32 LinePerpLen = Length(LinePerp);
#if 0
    if(LinePerpLen < Thickness)
    {
        LinePerp = Group->DebugXForm.Y;
    }
    else
#endif
    {
        LinePerp /= LinePerpLen;
    }
    
    LinePerp *= Thickness;
    
    f32 ZBias = 0.01f;
    
    v4 P0 = V4(FromP - LinePerp, ZBias);
    v2 UV0 = {0, 0};
    u32 C0 = FromC;
    
    v4 P1 = V4(ToP - LinePerp, ZBias);
    v2 UV1 = {1, 0};
    u32 C1 = ToC;
    
    v4 P2 = V4(ToP + LinePerp, ZBias);
    v2 UV2 = {1, 1};
    u32 C2 = ToC;
    
    v4 P3 = V4(FromP + LinePerp, ZBias);
    v2 UV3 = {0, 1};
    u32 C3 = FromC;
    
    PushQuad(Group, Texture,
             P0, UV0, C0,
             P1, UV1, C1,
             P2, UV2, C2,
             P3, UV3, C3,
             1.0f);
#else
    // TODO(casey): Cylinder version here
#endif
}

inline void
PushLineSegment(render_group *Group,
                v3 FromP, v4 FromCF, v3 ToP, v4 ToCF, f32 Thickness)
{
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1, Group->WhiteTexture);
    PushLineSegment(Group, Group->WhiteTexture, FromP, FromCF, ToP, ToCF, Thickness);
}

inline void
PushQuad(render_group *Group, renderer_texture Texture,
         v4 P0, v2 UV0, v4 C0,
         v4 P1, v2 UV1, v4 C1,
         v4 P2, v2 UV2, v4 C2,
         v4 P3, v2 UV3, v4 C3,
         f32 Emission)
{
    PushQuad(Group, Texture,
             P0, UV0, RGBAPack4x8(255.0f*C0),
             P1, UV1, RGBAPack4x8(255.0f*C1),
             P2, UV2, RGBAPack4x8(255.0f*C2),
             P3, UV3, RGBAPack4x8(255.0f*C3),
             Emission);
}

internal cube_uv_layout
EncodeCubeUVLayout(u32x BottomIndex,
                   u32x BottomFacingIndex,  // NOTE(casey): 0 = 0 degrees, 1 = 90 ... 3 = 270
                   u32x SideIndex0, // NOTE(casey): +X This is the EAST side
                   u32x SideIndex1, // NOTE(casey): +Y NORTH
                   u32x SideIndex2, // NOTE(casey): -X WEST
                   u32x SideIndex3, // NOTE(casey): -Y SOUTH
                   u32x TopIndex,
                   u32x TopFacingIndex)
{
    Assert(BottomIndex <= 3);
    Assert(BottomFacingIndex <= 3);
    Assert(SideIndex0 <= 3);
    Assert(SideIndex1 <= 3);
    Assert(SideIndex2 <= 3);
    Assert(SideIndex3 <= 3);
    Assert(TopIndex <= 3);
    Assert(TopFacingIndex <= 3);
    
    cube_uv_layout Result;
    Result.Encoding = (u16)((BottomIndex << 0) |
                            (BottomFacingIndex << 2) |
                            (SideIndex0  << 4) |
                            (SideIndex1  << 6) |
                            (SideIndex2  << 8) |
                            (SideIndex3  << 10) |
                            (TopIndex    << 12) |
                            (TopFacingIndex << 14));
    
    return(Result);
}

internal cube_uv_layout
DefaultCubeUVLayout(void)
{
    cube_uv_layout Result = {};
    return(Result);
}

inline void
PushLightingElement(render_group *Group, v3 P, v3 Radius, v3 Color, f32 Emission)
{
    game_render_commands *Commands = Group->Commands;
    if(Group->LightingEnabled)
    {
        Assert(Commands->LightOccluderCount < Commands->MaxLightOccluderCount);
        lighting_box *Box = Commands->LightOccluders + Commands->LightOccluderCount++;
        Box->BoxMin = P - Radius;
        Box->BoxMax = P + Radius;
        Box->RefC = Color;
        VALIDATE_TEXEL(Box->RefC);
        Box->Emission = Emission;
        
        //PushVolumeOutline(Group, RectMinMax(Box->BoxMin, Box->BoxMax));
    }
}

inline void
PushCube(render_group *Group, renderer_texture Texture, v3 P, v3 Radius,
         v4 Color = V4(1, 1, 1, 1),
         cube_uv_layout UVLayout = DefaultCubeUVLayout(),
         f32 Emission = 0.0f, f32 ZBias = 0.0f)
{
    Assert(Emission >= 0.0f);
    Assert(Emission <= 1.0f);
    
    game_render_commands *Commands = Group->Commands;
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 6, Texture);
    if(Entry)
    {
        f32 Nx = P.x - Radius.x;
        f32 Px = P.x + Radius.x;
        f32 Ny = P.y - Radius.y;
        f32 Py = P.y + Radius.y;
        f32 Nz = P.z - Radius.z;
        f32 Pz = P.z + Radius.z;
        
        v4 P0 = {Nx, Ny, Pz, ZBias};
        v4 P1 = {Px, Ny, Pz, ZBias};
        v4 P2 = {Px, Py, Pz, ZBias};
        v4 P3 = {Nx, Py, Pz, ZBias};
        v4 P4 = {Nx, Ny, Nz, ZBias};
        v4 P5 = {Px, Ny, Nz, ZBias};
        v4 P6 = {Px, Py, Nz, ZBias};
        v4 P7 = {Nx, Py, Nz, ZBias};
        
        v4 TopColor = StoreColor(Color);
        v4 BotColor = {0, 0, 0, TopColor.a};
        
        v4 CT = V4(0.75*TopColor.rgb, TopColor.a);
        v4 CB = V4(0.25*TopColor.rgb, TopColor.a);
        
        TopColor = BotColor = CT = CB = StoreColor(Color);
        
        // TODO(casey): If we eventually have mesh shaders in the renderer, we can actually
        // save a ton of bandwidth by just encoding the 16-bit cube layout directly and
        // expanding in the shader.
        
        u32 BotFace = ((UVLayout.Encoding >> 2) & 0x3);
        u32 TopFace = ((UVLayout.Encoding >> 14) & 0x3);
        f32 BotX  = (f32)((UVLayout.Encoding >>  0) & 0x3) * 0.25f;
        f32 TopX  = (f32)((UVLayout.Encoding >> 12) & 0x3) * 0.25f;
        f32 MidX0 = (f32)((UVLayout.Encoding >>  4) & 0x3) * 0.25f;
        f32 MidX1 = (f32)((UVLayout.Encoding >>  6) & 0x3) * 0.25f;
        f32 MidX2 = (f32)((UVLayout.Encoding >>  8) & 0x3) * 0.25f;
        f32 MidX3 = (f32)((UVLayout.Encoding >> 10) & 0x3) * 0.25f;
        
        /*
        X - xor both bits!
        00 0 - 0.00f
        01 1 - 0.25f
        10 2 - 0.25f
        11 3 - 0.00f
        
        Y - second bit!
        00 0 - 0.00f
        01 1 - 0.00f
        10 2 - 0.25f
        11 3 - 0.25f
        */
        
#define CubeDecodeX(Face, Offset) (f32)(((Offset - Face) ^ ((Offset - Face) >> 1)) & 0x1) * 0.25f
#define CubeDecodeY(Face, Offset) (f32)(((Offset - Face) >> 1) & 0x1) * 0.25f
        
        // TODO(casey): Bottom face orientation is _probably_ incorrect?  We
        // could check this if we cared...
        v2 BotT0 = V2(BotX + CubeDecodeX(BotFace, 0), CubeDecodeY(BotFace, 0));
        v2 BotT1 = V2(BotX + CubeDecodeX(BotFace, 1), CubeDecodeY(BotFace, 1));
        v2 BotT2 = V2(BotX + CubeDecodeX(BotFace, 2), CubeDecodeY(BotFace, 2));
        v2 BotT3 = V2(BotX + CubeDecodeX(BotFace, 3), CubeDecodeY(BotFace, 3));
        
        v2 MidT0_0 = {MidX0 + 0, 0.25f};
        v2 MidT1_0 = {MidX0 + 0.25f, 0.25f};
        v2 MidT2_0 = {MidX0 + 0.25f, 0.75f};
        v2 MidT3_0 = {MidX0 + 0, 0.75f};
        
        v2 MidT0_1 = {MidX1 + 0, 0.25f};
        v2 MidT1_1 = {MidX1 + 0.25f, 0.25f};
        v2 MidT2_1 = {MidX1 + 0.25f, 0.75f};
        v2 MidT3_1 = {MidX1 + 0, 0.75f};
        
        v2 MidT0_2 = {MidX2 + 0, 0.25f};
        v2 MidT1_2 = {MidX2 + 0.25f, 0.25f};
        v2 MidT2_2 = {MidX2 + 0.25f, 0.75f};
        v2 MidT3_2 = {MidX2 + 0, 0.75f};
        
        v2 MidT0_3 = {MidX3 + 0, 0.25f};
        v2 MidT1_3 = {MidX3 + 0.25f, 0.25f};
        v2 MidT2_3 = {MidX3 + 0.25f, 0.75f};
        v2 MidT3_3 = {MidX3 + 0, 0.75f};
        
        v2 TopT0 = V2(TopX + CubeDecodeX(TopFace, 0), 0.75f + CubeDecodeY(TopFace, 0));
        v2 TopT1 = V2(TopX + CubeDecodeX(TopFace, 1), 0.75f + CubeDecodeY(TopFace, 1));
        v2 TopT2 = V2(TopX + CubeDecodeX(TopFace, 2), 0.75f + CubeDecodeY(TopFace, 2));
        v2 TopT3 = V2(TopX + CubeDecodeX(TopFace, 3), 0.75f + CubeDecodeY(TopFace, 3));
        
        // NOTE(casey): -X
        PushQuad(Group, Texture,
                 P7, MidT0_2, CB,
                 P4, MidT1_2, CB,
                 /**/ P0, MidT2_2, CT,
                 /**/ P3, MidT3_2, CT, Emission);
        
        // NOTE(casey): +X
        PushQuad(Group, Texture,
                 /**/ P1, MidT3_0, CT,
                 P5, MidT0_0, CB,
                 P6, MidT1_0, CB,
                 /**/ P2, MidT2_0, CT, Emission);
        
        // NOTE(casey): -Y
        PushQuad(Group, Texture,
                 P4, MidT0_3, CB,
                 P5, MidT1_3, CB,
                 /**/ P1, MidT2_3, CT,
                 /**/ P0, MidT3_3, CT, Emission);
        
        // NOTE(casey): +Y
        PushQuad(Group, Texture,
                 /**/ P2, MidT3_1, CT,
                 P6, MidT0_1, CB,
                 P7, MidT1_1, CB,
                 /**/ P3, MidT2_1, CT, Emission);
        
        // NOTE(casey): -Z
        PushQuad(Group, Texture,
                 P7, BotT0, BotColor,
                 P6, BotT1, BotColor,
                 P5, BotT2, BotColor,
                 P4, BotT3, BotColor, Emission);
        
        // NOTE(casey): +Z
        PushQuad(Group, Texture,
                 P0, TopT0, TopColor,
                 P1, TopT1, TopColor,
                 P2, TopT2, TopColor,
                 P3, TopT3, TopColor, Emission);
    }
}

inline void
PushVolumeOutline(render_group *Group, rectangle3 Rectangle, v4 Color = V4(1, 1, 1, 1), f32 Thickness = 0.1f)
{
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 12, Group->WhiteTexture);
    if(Entry)
    {
        renderer_texture Texture = Group->WhiteTexture;
        
        f32 Nx = Rectangle.Min.x;
        f32 Px = Rectangle.Max.x;
        f32 Ny = Rectangle.Min.y;
        f32 Py = Rectangle.Max.y;
        f32 Nz = Rectangle.Min.z;
        f32 Pz = Rectangle.Max.z;
        
        v3 P0 = {Nx, Ny, Pz};
        v3 P1 = {Px, Ny, Pz};
        v3 P2 = {Px, Py, Pz};
        v3 P3 = {Nx, Py, Pz};
        v3 P4 = {Nx, Ny, Nz};
        v3 P5 = {Px, Ny, Nz};
        v3 P6 = {Px, Py, Nz};
        v3 P7 = {Nx, Py, Nz};
        
        v4 C = StoreColor(Color);
        
        PushLineSegment(Group, Texture, P0, C, P1, C, Thickness);
        PushLineSegment(Group, Texture, P0, C, P3, C, Thickness);
        PushLineSegment(Group, Texture, P0, C, P4, C, Thickness);
        
        PushLineSegment(Group, Texture, P2, C, P1, C, Thickness);
        PushLineSegment(Group, Texture, P2, C, P3, C, Thickness);
        PushLineSegment(Group, Texture, P2, C, P6, C, Thickness);
        
        PushLineSegment(Group, Texture, P5, C, P1, C, Thickness);
        PushLineSegment(Group, Texture, P5, C, P4, C, Thickness);
        PushLineSegment(Group, Texture, P5, C, P6, C, Thickness);
        
        PushLineSegment(Group, Texture, P7, C, P3, C, Thickness);
        PushLineSegment(Group, Texture, P7, C, P4, C, Thickness);
        PushLineSegment(Group, Texture, P7, C, P6, C, Thickness);
    }
}

inline void
PushRect(render_group *Group, v3 P, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    v3 BasisP = (P - V3(0.5f*Dim, 0));
    
    v2 ScaledDim = Dim;
    
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1, Group->WhiteTexture);
    if(Entry)
    {
        v4 PremulColor = StoreColor(Color);
        u32 C = RGBAPack4x8(255.0f*PremulColor);
        
        v3 MinP = BasisP;
        v3 MaxP = MinP + V3(ScaledDim, 0);
        
        v2 MinUV = {0, 0};
        v2 MaxUV = {1, 1};
        
        PushQuad(Group, Group->WhiteTexture,
                 V4(MinP.x, MinP.y, MinP.z, 0), V2(MinUV.x, MinUV.y), C,
                 V4(MaxP.x, MinP.y, MinP.z, 0), V2(MaxUV.x, MinUV.y), C,
                 V4(MaxP.x, MaxP.y, MinP.z, 0), V2(MaxUV.x, MaxUV.y), C,
                 V4(MinP.x, MaxP.y, MinP.z, 0), V2(MinUV.x, MaxUV.y), C);
    }
}

inline void
PushRect(render_group *Group, rectangle2 Rectangle, v3 Offset, v4 Color = V4(1, 1, 1, 1))
{
    PushRect(Group, Offset + V3(GetCenter(Rectangle), 0), GetDim(Rectangle), Color);
}

inline void
PushRectOutline(render_group *Group, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1), real32 Thickness = 0.1f)
{
    // NOTE(casey): Top and bottom
    PushRect(Group, Offset - V3(0, 0.5f*Dim.y, 0), V2(Dim.x-Thickness-0.01f, Thickness), Color);
    PushRect(Group, Offset + V3(0, 0.5f*Dim.y, 0), V2(Dim.x-Thickness-0.01f, Thickness), Color);
    
    // NOTE(casey): Left and right
    PushRect(Group, Offset - V3(0.5f*Dim.x, 0, 0), V2(Thickness, Dim.y + Thickness), Color);
    PushRect(Group, Offset + V3(0.5f*Dim.x, 0, 0), V2(Thickness, Dim.y + Thickness), Color);
}

inline void
PushRectOutline(render_group *Group, rectangle2 Rectangle, v3 Offset, v4 Color = V4(1, 1, 1, 1), real32 Thickness = 0.1f)
{
    PushRectOutline(Group, V3(GetCenter(Rectangle), 0) + Offset, GetDim(Rectangle), Color, Thickness);
}

inline void
PushUpright(render_group *Group, renderer_texture Texture,
            v3 GroundP, v2 Size,
            v4 Color = V4(1, 1, 1, 1),
            v2 XAxis2 = V2(1, 0), v2 YAxis2 = V2(0, 1),
            v2 MinUV = V2(0, 0),
            v2 MaxUV = V2(1, 1),
            f32 tCameraUp = 0.5f)
{
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1, Texture);
    if(Entry)
    {
        v4 PremulColor = StoreColor(Color);
        
        v3 WorldUp = Group->WorldUp;
        v3 CameraUp = Group->GameXForm.Y;
        v3 XAxisH = Group->GameXForm.X;
        v3 YAxisH = NOZ(Lerp(WorldUp, tCameraUp, CameraUp));
        f32 ZBias = tCameraUp*Inner(WorldUp, CameraUp)*Size.y;
        
        v3 XAxis = Size.x*(XAxis2.x*XAxisH + XAxis2.y*YAxisH);
        v3 YAxis = Size.y*(YAxis2.x*XAxisH + YAxis2.y*YAxisH);
        
        u32 Color32 = RGBAPack4x8(255.0f*PremulColor);
        
        v3 MinP = GroundP - 0.5f*XAxis;
        v4 MinXMinY = V4(MinP, 0.0f);
        v4 MinXMaxY = V4(MinP + YAxis, ZBias);
        v4 MaxXMinY = V4(MinP + XAxis, 0.0f);
        v4 MaxXMaxY = V4(MinP + XAxis + YAxis, ZBias);
        
        PushQuad(Group, Texture,
                 MinXMinY, V2(MinUV.x, MinUV.y), Color32,
                 MaxXMinY, V2(MaxUV.x, MinUV.y), Color32,
                 MaxXMaxY, V2(MaxUV.x, MaxUV.y), Color32,
                 MinXMaxY, V2(MinUV.x, MaxUV.y), Color32);
    }
}

inline void
PushSprite(render_group *Group, renderer_texture Texture,
           v3 MinP,
           v3 ScaledXAxis = V3(1, 0, 0),
           v3 ScaledYAxis = V3(0, 1, 0),
           v4 Color = V4(1, 1, 1, 1),
           v2 MinUV = V2(0, 0),
           v2 MaxUV = V2(1, 1))
{
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1, Texture);
    if(Entry)
    {
        u32 Color32 = FinalizeColor(Color);
        
        v4 MinXMinY = V4(MinP, 0.0f);
        v4 MinXMaxY = V4(MinP + ScaledYAxis, 0.0f);
        v4 MaxXMinY = V4(MinP + ScaledXAxis, 0.0f);
        v4 MaxXMaxY = V4(MinP + ScaledXAxis + ScaledYAxis, 0.0f);
        
        PushQuad(Group, Texture,
                 MinXMinY, V2(MinUV.x, MinUV.y), Color32,
                 MaxXMinY, V2(MaxUV.x, MinUV.y), Color32,
                 MaxXMaxY, V2(MaxUV.x, MaxUV.y), Color32,
                 MinXMaxY, V2(MinUV.x, MaxUV.y), Color32);
    }
}

internal v2
ClipSpaceFromPixelSpace(f32 PixelDimX, f32 PixelDimY, v2 PixelSpaceXY)
{
    v2 PixelDim = V2(PixelDimX, PixelDimY);
    v2 ScreenCenter = 0.5f*PixelDim;
    
    v2 ClipSpaceXY = (PixelSpaceXY- ScreenCenter);
    ClipSpaceXY.x *= 2.0f / PixelDim.x;
    ClipSpaceXY.y *= 2.0f / PixelDim.y;
    
    return(ClipSpaceXY);
}

inline v3
Unproject(render_group *Group, render_transform *XForm,
          v2 ClipSpaceXY, f32 WorldDistanceFromCameraZ)
{
    v4 ProbeZ = V4(XForm->P - WorldDistanceFromCameraZ*XForm->Z, 1.0f);
    ProbeZ = XForm->Proj.Forward*ProbeZ;
    
    ClipSpaceXY.x *= ProbeZ.w;
    ClipSpaceXY.y *= ProbeZ.w;
    
    v4 Clip = V4(ClipSpaceXY.x, ClipSpaceXY.y, ProbeZ.z, ProbeZ.w);
    
    v4 WorldP = XForm->Proj.Inverse*Clip;
    v3 Result = WorldP.xyz;
    
    return(Result);
}

inline rectangle3
GetCameraRectangleAtDistance(render_group *Group, f32 WorldDistanceFromCameraZ)
{
    v3 MinCorner = Unproject(Group, &Group->GameXForm, V2(-1, -1), WorldDistanceFromCameraZ);
    v3 MaxCorner = Unproject(Group, &Group->GameXForm, V2(1, 1), WorldDistanceFromCameraZ);
    
    rectangle3 Result = RectMinMax(MinCorner, MaxCorner);
    
    return(Result);
}

inline rectangle3
GetCameraRectangleAtTarget(render_group *Group, f32 Z)
{
    rectangle3 Result = GetCameraRectangleAtDistance(Group, Z);
    
    return(Result);
}

inline bool32
AllResourcesPresent(render_group *Group)
{
    bool32 Result = (Group->MissingResourceCount == 0);
    
    return(Result);
}

inline void
PushSetup(render_group *Group, render_setup *NewSetup)
{
    Group->LastSetup = *NewSetup;
    Group->CurrentQuads = 0;
}

inline v2
GetClipSpacePoint(render_group *Group, v3 WorldP)
{
    v4 P = Group->LastSetup.Proj*V4(WorldP, 1.0f);
    P.xyz /= P.w;
    
    return(P.xy);
}

inline rectangle2
GetClipRect(render_group *Group, v3 Offset, v2 Dim)
{
    v2 MinCorner = GetClipSpacePoint(Group, Offset);
    v2 MaxCorner = GetClipSpacePoint(Group, Offset + V3(Dim, 0));
    rectangle2 Result = RectMinMax(MinCorner, MaxCorner);
    
    return(Result);
}

inline rectangle2
GetClipRect(render_group *Group, rectangle2 Rectangle, r32 Z)
{
    rectangle2 Result = GetClipRect(Group, V3(Rectangle.Min, Z), GetDim(Rectangle));
    return(Result);
}

global fog_params DefaultFogParams;
global alpha_clip_params DefaultAlphaClipParams
{
    -100.0f,
    -99.0f,
};

internal void
SetLightBounds(render_group *RenderGroup, v3 VoxelMinCorner, v3 VoxelInvTotalDim)
{
    game_render_commands *Commands = RenderGroup->Commands;
    Commands->LightingVoxelMinCorner = VoxelMinCorner;
    Commands->LightingVoxelInvTotalDim = VoxelInvTotalDim;
}

inline void
SetCameraTransform(render_group *RenderGroup, u32 Flags, f32 FocalLength,
                   v3 CameraX = V3(1, 0, 0),
                   v3 CameraY = V3(0, 1, 0),
                   v3 CameraZ = V3(0, 0, 1),
                   v3 CameraP = V3(0, 0, 0),
                   f32 NearClipPlane = 0.1f,
                   f32 FarClipPlane = 100.0f,
                   fog_params *Fog = &DefaultFogParams,
                   alpha_clip_params *AlphaClip = &DefaultAlphaClipParams)
{
    b32x Ortho = (Flags & Camera_IsOrthographic);
    b32x IsDebug = (Flags & Camera_IsDebug);
    
    f32 b = SafeRatio1((r32)RenderGroup->Commands->Settings.RenderDim.Width,
                       (r32)RenderGroup->Commands->Settings.RenderDim.Height);
    
    render_transform *XForm = IsDebug ? &RenderGroup->DebugXForm : &RenderGroup->GameXForm;
    
    render_setup NewSetup = RenderGroup->LastSetup;
    
    m4x4_inv Proj;
    if(Ortho)
    {
        Proj = OrthographicProjection(b, NearClipPlane, FarClipPlane);
        
    }
    else
    {
        Proj = PerspectiveProjection(b, FocalLength, NearClipPlane, FarClipPlane);
    }
    
    NewSetup.FogDirection = Fog->Direction;
    NewSetup.FogStartDistance = Fog->StartDistance;
    NewSetup.FogEndDistance = Fog->EndDistance;
    NewSetup.ClipAlphaStartDistance = AlphaClip->dStartDistance;
    NewSetup.ClipAlphaEndDistance = AlphaClip->dEndDistance;
    
    XForm->X = CameraX;
    XForm->Y = CameraY;
    XForm->Z = CameraZ;
    XForm->P = CameraP;
    
    NewSetup.CameraP = CameraP;
    
    m4x4_inv CameraC = CameraTransform(CameraX, CameraY, CameraZ, CameraP);
    XForm->Proj.Forward = Proj.Forward*CameraC.Forward;
    XForm->Proj.Inverse = CameraC.Inverse*Proj.Inverse;
    
#if HANDMADE_SLOW
    m4x4 I = XForm->Proj.Inverse*XForm->Proj.Forward;
#endif
    NewSetup.Proj = XForm->Proj.Forward;
    
    PushSetup(RenderGroup, &NewSetup);
    
    if(!IsDebug)
    {
        RenderGroup->DebugXForm = RenderGroup->GameXForm;
    }
}

inline void
BeginDepthPeel_(render_group *Group, v4 Color)
{
    render_entry_begin_peels *Entry = PushRenderElement(Group, render_entry_begin_peels);
    if(Entry)
    {
        Entry->ClearColor = Color;
        Group->LastSetup.FogColor = V3(Square(Color.r), Square(Color.g), Square(Color.b));
    }
}

inline void
EndDepthPeel_(render_group *Group)
{
    PushRenderElement_(Group, 0, RenderGroupEntryType_render_entry_end_peels);
}

inline void
PushDepthClear_(render_group *Group)
{
    PushRenderElement_(Group, 0, RenderGroupEntryType_render_entry_depth_clear);
}

inline void
PushFullClear_(render_group *Group, v4 Color)
{
    render_entry_full_clear *Entry = PushRenderElement(Group, render_entry_full_clear);
    if(Entry)
    {
        Entry->ClearColor = Color;
        Group->LastSetup.FogColor = V3(Square(Color.r), Square(Color.g), Square(Color.b));
    }
}

inline render_group
BeginRenderGroup(game_assets *Assets, game_render_commands *Commands,
                 enum32(render_group_flags) Flags = Render_Default,
                 v4 ClearColor = V4(0, 0, 0, 0))
{
    render_group Result = {};
    
    Result.Assets = Assets;
    Result.Flags = Flags;
    Result.MissingResourceCount = 0;
    Result.Commands = Commands;
    Result.WorldUp = V3(0, 0, 1);
    
    render_setup InitialSetup = {};
    InitialSetup.FogStartDistance = 0.0f;
    InitialSetup.FogEndDistance = 1.0f;
    InitialSetup.ClipAlphaStartDistance = 0.0f;
    InitialSetup.ClipAlphaEndDistance = 1.0f;
    InitialSetup.ClipRect = RectMinMax(V2(-1, -1), V2(1, 1));
    InitialSetup.Proj = Identity();
    PushSetup(&Result, &InitialSetup);
    
    if(Flags & Render_HandleTransparency)
    {
        Assert(Flags & Render_ClearColor);
        Assert(Flags & Render_ClearDepth);
        
        BeginDepthPeel_(&Result, ClearColor);
    }
    else
    {
        if(Flags & Render_ClearColor)
        {
            Assert(Flags & Render_ClearDepth);
            PushFullClear_(&Result, ClearColor);
        }
        else if(Flags & Render_ClearDepth)
        {
            PushDepthClear_(&Result);
        }
    }
    
    return(Result);
}

inline void
EnableLighting(render_group *RenderGroup, b32x LightingEnabled)
{
    RenderGroup->LightingEnabled = LightingEnabled;
}

inline void
EndRenderGroup(render_group *Group)
{
    if(Group->Flags & Render_HandleTransparency)
    {
        EndDepthPeel_(Group);
    }
    
    // TODO(casey):
    // RenderGroup->Commands->MissingResourceCount += RenderGroup->MissingResourceCount;
}

internal texture_op *
BeginTextureOp(renderer_texture_queue *Queue, u32 SizeRequested)
{
    texture_op *Result = 0;
    if(Queue->OpCount < ArrayCount(Queue->Ops))
    {
        u32 SizeAvailable = 0;
        
        u32 MemoryAt = Queue->TransferMemoryLastUsed;
        if(Queue->TransferMemoryLastUsed == Queue->TransferMemoryFirstUsed)
        {
            // NOTE(casey): The used space is either the entire buffer or none of the buffer,
            // and we disambiguate between those two by just checking if there are any
            // ops outstanding
            if(Queue->OpCount == 0)
            {
                MemoryAt = 0;
                SizeAvailable = Queue->TransferMemoryCount;
            }
        }
        else if(Queue->TransferMemoryLastUsed < Queue->TransferMemoryFirstUsed)
        {
            // NOTE(casey): The used space wraps around, one continuous usable space
            SizeAvailable = (Queue->TransferMemoryFirstUsed - MemoryAt);
        }
        else
        {
            // NOTE(casey): The used space doesn't wrap, two usable spaces on either side
            SizeAvailable = (Queue->TransferMemoryCount - MemoryAt);
            if(SizeAvailable < SizeRequested)
            {
                MemoryAt = 0;
                SizeAvailable = (Queue->TransferMemoryFirstUsed - MemoryAt);
            }
        }
        
        if(SizeRequested <= SizeAvailable)
        {
            u32 OpIndex = Queue->FirstOpIndex + Queue->OpCount++;
            Result = Queue->Ops + (OpIndex % ArrayCount(Queue->Ops));
            
            Result->Data = Queue->TransferMemory + MemoryAt;
            Result->TransferMemoryLastUsed = MemoryAt + SizeRequested;
            Result->State = TextureOp_PendingLoad;
            
#if 0
            for(u32 Index = 0;
                Index < Result->TransferMemoryCount;
                ++Index)
            {
                ((u8 *)Result->Data)[Index] = 0xFF;
            }
#endif
            
            Queue->TransferMemoryLastUsed = Result->TransferMemoryLastUsed;
            
            Assert(MemoryAt < Queue->TransferMemoryCount);
            Assert(Result->TransferMemoryLastUsed <= Queue->TransferMemoryCount);
        }
    }
    
    return(Result);
}

internal void
CompleteTextureOp(renderer_texture_queue *Queue, texture_op *Op)
{
    Op->State = TextureOp_ReadyToTransfer;
}

internal void
CancelTextureOp(renderer_texture_queue *Queue, texture_op *Op)
{
    Op->State = TextureOp_Empty;
}

internal indexed_vertex_output
OutputQuads(render_group *RenderGroup, u32 VertexCount, u32 IndexCount)
{
    indexed_vertex_output Result = {};
    if((VertexCount > 0) || (IndexCount > 0))
    {
        renderer_texture NullTexture = {};
        render_entry_textured_quads *Entry = GetCurrentQuads(RenderGroup, (IndexCount / 6), NullTexture);
        
        game_render_commands *Commands = RenderGroup->Commands;
        
        u32 VertIndex = Commands->VertexCount;
        u32 IndexIndex = Commands->IndexCount;
        
        Commands->VertexCount += VertexCount;
        Commands->IndexCount += IndexCount;
        Assert(Commands->VertexCount <= Commands->MaxVertexCount);
        Assert(Commands->IndexCount <= Commands->MaxIndexCount);
        
        textured_vertex *Vert = Commands->VertexArray + VertIndex;
        u16 *Index = Commands->IndexArray + IndexIndex;
        
        Result.VertexAt = Commands->VertexArray + VertIndex;
        Result.IndexAt = Commands->IndexArray + IndexIndex;
        Result.BaseIndex = SafeTruncateToU16(VertIndex - Entry->VertexArrayOffset);
        
        Entry->QuadCount += (IndexCount / 6);
    }
    
    return(Result);
}

internal indexed_vertex_output
OutputQuads(render_group *RenderGroup, u32 QuadCount)
{
    indexed_vertex_output Result = OutputQuads(RenderGroup, QuadCount*4, QuadCount*6);
    return(Result);
}

internal void
VertexOut(indexed_vertex_output *Out, u32 VertexOffset, v3 P, v3 N, v2 UV, u32 Color, u16 TextureIndex)
{
    Out->VertexAt[VertexOffset].P.xyz = P;
    Out->VertexAt[VertexOffset].P.w = 0.0f;
    Out->VertexAt[VertexOffset].N = N;
    Out->VertexAt[VertexOffset].UV = UV;
    Out->VertexAt[VertexOffset].Color = Color;
    Out->VertexAt[VertexOffset].TextureIndex = TextureIndex;
    Out->VertexAt[VertexOffset].Emission = 0;
    Out->VertexAt[VertexOffset].Tone = 255;
}

internal void
QuadIndexOut(indexed_vertex_output *Out, u32 IndexOffset)
{
    Out->IndexAt[IndexOffset + 0] = Out->BaseIndex + 0;
    Out->IndexAt[IndexOffset + 1] = Out->BaseIndex + 1;
    Out->IndexAt[IndexOffset + 2] = Out->BaseIndex + 2;
    Out->IndexAt[IndexOffset + 3] = Out->BaseIndex + 0;
    Out->IndexAt[IndexOffset + 4] = Out->BaseIndex + 2;
    Out->IndexAt[IndexOffset + 5] = Out->BaseIndex + 3;
}

internal void
Advance(indexed_vertex_output *Out, u32 VertexCount, u32 IndexCount)
{
    Assert(((u32)Out->BaseIndex + VertexCount) <= U16Max);
    
    Out->VertexAt += VertexCount;
    Out->IndexAt += IndexCount;
    Out->BaseIndex += (u16)VertexCount;
}

internal void
Advance(indexed_vertex_output *Out, u32 QuadCount)
{
    Advance(Out, QuadCount*4, QuadCount*6);
}
