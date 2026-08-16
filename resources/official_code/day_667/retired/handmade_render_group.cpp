/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

inline void PushSetup(render_group *Group, render_setup *NewSetup);

struct transient_clip_rect
{
    transient_clip_rect(render_group *RenderGroupInit, rectangle2i NewClipRect)
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
    rectangle2i OldClipRect;
};

inline v3 GetRenderEntityBasisP(object_transform *ObjectTransform, v3 OriginalP)
{
    v3 P = OriginalP + ObjectTransform->OffsetP;
    return(P);
}

inline push_buffer_result
PushBuffer(render_group *RenderGroup, u32 DataSize)
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

inline void
PushSortBarrier(render_group *RenderGroup, b32 TurnOffSorting)
{
    // TODO(casey): Do we want the sort barrier again?
}

#define PushRenderElement(Group, type) (type *)PushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)
inline void *
PushRenderElement_(render_group *Group, uint32 Size, render_group_entry_type Type)
{
    game_render_commands *Commands = Group->Commands;

    void *Result = 0;

    Size += sizeof(render_group_entry_header);
    push_buffer_result Push = PushBuffer(Group, Size);
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

inline used_bitmap_dim
GetBitmapDim(render_group *Group, object_transform *ObjectTransform,
             loaded_bitmap *Bitmap, real32 Height, v3 Offset, r32 CAlign,
             v2 XAxis = V2(1, 0), v2 YAxis = V2(0, 1))
{
    used_bitmap_dim Dim;

#if 0
    Dim.Size = V2(Height*Bitmap->WidthOverHeight, Height);
    Dim.Align = CAlign*Hadamard(Bitmap->AlignPercentage, Dim.Size);
    Dim.P.z = Offset.z;
    Dim.P.xy = Offset.xy - Dim.Align.x*XAxis - Dim.Align.y*YAxis;
    Dim.BasisP = GetRenderEntityBasisP(ObjectTransform, Dim.P);
#else
    Dim.Size.x = Height*Bitmap->WidthOverHeight;
    Dim.Size.y = Height;
    Dim.Align.x = CAlign*Bitmap->AlignPercentage.x*Dim.Size.x;
    Dim.Align.y = CAlign*Bitmap->AlignPercentage.y*Dim.Size.y;
    Dim.P.z = Offset.z;
    Dim.P.x = Offset.x - Dim.Align.x*XAxis.x - Dim.Align.y*YAxis.x;
    Dim.P.y = Offset.y - Dim.Align.x*XAxis.y - Dim.Align.y*YAxis.y;
    Dim.BasisP.x = ObjectTransform->OffsetP.x + Dim.P.x;
    Dim.BasisP.y = ObjectTransform->OffsetP.y + Dim.P.y;
    Dim.BasisP.z = ObjectTransform->OffsetP.z + Dim.P.z;
#endif

    return(Dim);
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

inline render_entry_textured_quads *
GetCurrentQuads(render_group *Group, u32 QuadCount)
{
    game_render_commands *Commands = Group->Commands;

    if(!Group->CurrentQuads)
    {
        Group->CurrentQuads = (render_entry_textured_quads *)
            PushRenderElement_(Group, sizeof(render_entry_textured_quads), RenderGroupEntryType_render_entry_textured_quads);
        Group->CurrentQuads->QuadCount = 0;
        Group->CurrentQuads->VertexArrayOffset = Commands->VertexCount;
        Group->CurrentQuads->Setup = Group->LastSetup;
    }

    render_entry_textured_quads *Result = Group->CurrentQuads;
    if((Commands->VertexCount + 4*QuadCount) > Commands->MaxVertexCount)
    {
        Result = 0;
    }

    return(Result);
}

inline void
PushQuad(render_group *Group, loaded_bitmap *Bitmap,
         v4 P0, v2 UV0, u32 C0,
         v4 P1, v2 UV1, u32 C1,
         v4 P2, v2 UV2, u32 C2,
         v4 P3, v2 UV3, u32 C3,
         f32 Emission = 0.0f,
         u16 LightCount = 0,
         u16 LightIndex = 0)
{
    Assert(Emission >= 0.0f);
    Assert(Emission <= 1.0f);

    game_render_commands *Commands = Group->Commands;

    render_entry_textured_quads *Entry = Group->CurrentQuads;
    Assert(Entry);

    ++Entry->QuadCount;

    u32 VertIndex = Commands->VertexCount;
    Commands->VertexCount += 4;

    Commands->QuadBitmaps[VertIndex >> 2] = Bitmap;
    textured_vertex *Vert = Commands->VertexArray + VertIndex;

    // TODO(casey): Stop computing the normal, we could just pass it down directly!
    v3 N0, N1, N2, N3;

    v4 E10 = (P1 - P0);
    E10.z += E10.w;

    v4 E20 = (P2 - P0);
    E20.z += E20.w;

    v3 NormalDirection = Cross(E10.xyz, E20.xyz);
    v3 Normal = NOZ(NormalDirection);

    N0 = N1 = N2 = N3 = Normal;

    Vert[0].P = P3;
    Vert[0].N = N3;
    Vert[0].UV = UV3;
    Vert[0].Color = C3;
    Vert[0].LightIndex = LightIndex;

    Vert[1].P = P0;
    Vert[1].N = N0;
    Vert[1].UV = UV0;
    Vert[1].Color = C0;
    Vert[1].LightIndex = LightIndex;

    Vert[2].P = P2;
    Vert[2].N = N2;
    Vert[2].UV = UV2;
    Vert[2].Color = C2;
    Vert[2].LightIndex = LightIndex;

    Vert[3].P = P1;
    Vert[3].N = N1;
    Vert[3].UV = UV1;
    Vert[3].Color = C1;
    Vert[3].LightIndex = LightIndex;
}

inline void
PushLineSegment(render_group *Group, loaded_bitmap *Bitmap,
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

    PushQuad(Group, Bitmap,
             P0, UV0, C0,
             P1, UV1, C1,
             P2, UV2, C2,
             P3, UV3, C3);
#else
    // TODO(casey): Cylinder version here
#endif
}

inline void
PushQuad(render_group *Group, loaded_bitmap *Bitmap,
         v4 P0, v2 UV0, v4 C0,
         v4 P1, v2 UV1, v4 C1,
         v4 P2, v2 UV2, v4 C2,
         v4 P3, v2 UV3, v4 C3,
         f32 Emission, u16 LightCount = 0, u16 LightIndex = 0)
{
    PushQuad(Group, Bitmap,
             P0, UV0, RGBAPack4x8(255.0f*C0),
             P1, UV1, RGBAPack4x8(255.0f*C1),
             P2, UV2, RGBAPack4x8(255.0f*C2),
             P3, UV3, RGBAPack4x8(255.0f*C3),
             Emission, LightCount, LightIndex);
}

inline void
PushBitmap(render_group *Group, object_transform *ObjectTransform,
           loaded_bitmap *Bitmap, real32 Height, v3 Offset, v4 Color = V4(1, 1, 1, 1), r32 CAlign = 1.0f,
           v2 XAxis2 = V2(1, 0), v2 YAxis2 = V2(0, 1))
{
    if(Bitmap->Width && Bitmap->Height)
    {
        used_bitmap_dim Dim = GetBitmapDim(Group, ObjectTransform, Bitmap, Height, Offset, CAlign,
                                           XAxis2, YAxis2);

        v2 Size = Dim.Size;

        render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1);
        if(Entry)
        {
            v3 MinP = Dim.BasisP;
            f32 ZBias = 0.0f;
            v4 PremulColor = StoreColor(Color);
            v3 XAxis = Size.x*V3(XAxis2, 0.0f);
            v3 YAxis = Size.y*V3(YAxis2, 0.0f);

            if(ObjectTransform->Upright)
            {
                ZBias = 0.25f*Height;

                v3 XAxis0 = Size.x*V3(XAxis2.x, 0.0f, XAxis2.y);
                v3 YAxis0 = Size.y*V3(YAxis2.x, 0.0f, YAxis2.y);
                v3 XAxis1 = Size.x*(XAxis2.x*Group->GameXForm.X + XAxis2.y*Group->GameXForm.Y);
                v3 YAxis1 = Size.y*(YAxis2.x*Group->GameXForm.X + YAxis2.y*Group->GameXForm.Y);

                // XAxis = Lerp(XAxis0, 0.8f, XAxis1);
                // YAxis = Lerp(YAxis0, 0.8f, YAxis1);
                XAxis = XAxis1;
                YAxis = YAxis1;
                YAxis.z = Lerp(YAxis0.z, 0.8f, YAxis1.z);
            }

            r32 OneTexelU = 1.0f / (r32)Bitmap->Width;
            r32 OneTexelV = 1.0f / (r32)Bitmap->Height;
            v2 MinUV = V2(OneTexelU, OneTexelV);
            v2 MaxUV = V2(1.0f - OneTexelU, 1.0f - OneTexelV);

            u32 Color32 = RGBAPack4x8(255.0f*PremulColor);

            v4 MinXMinY = V4(MinP, 0.0f);
            v4 MinXMaxY = V4(MinP + YAxis, ZBias);
            v4 MaxXMinY = V4(MinP + XAxis, 0.0f);
            v4 MaxXMaxY = V4(MinP + XAxis + YAxis, ZBias);

            PushQuad(Group, Bitmap,
                     MinXMinY, V2(MinUV.x, MinUV.y), Color32,
                     MaxXMinY, V2(MaxUV.x, MinUV.y), Color32,
                     MaxXMaxY, V2(MaxUV.x, MaxUV.y), Color32,
                     MinXMaxY, V2(MinUV.x, MaxUV.y), Color32);
        }
    }
}

inline void
PushBitmap(render_group *Group, object_transform *ObjectTransform,
           bitmap_id ID, real32 Height, v3 Offset, v4 Color = V4(1, 1, 1, 1), r32 CAlign = 1.0f,
           v2 XAxis = V2(1, 0), v2 YAxis = V2(0, 1))
{

    loaded_bitmap *Bitmap = GetBitmap(Group->Assets, ID, Group->GenerationID);
    if(Bitmap)
    {
        PushBitmap(Group, ObjectTransform, Bitmap, Height, Offset, Color, CAlign, XAxis, YAxis);
    }
    else
    {
        LoadBitmap(Group->Assets, ID, false);
        ++Group->MissingResourceCount;
    }
}

inline void
PushCube(render_group *Group, loaded_bitmap *Bitmap, v3 P, v3 Radius,
         v4 Color = V4(1, 1, 1, 1), f32 Emission = 0.0f,
         lighting_point_state *LightStore = 0)
{
    Assert(Emission >= 0.0f);
    Assert(Emission <= 1.0f);

    game_render_commands *Commands = Group->Commands;
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 6);
    if(Entry)
    {
        if(!Group->LightingEnabled)
        {
            LightStore = 0;
        }

        f32 Nx = P.x - Radius.x;
        f32 Px = P.x + Radius.x;
        f32 Ny = P.y - Radius.y;
        f32 Py = P.y + Radius.y;
        f32 Nz = P.z - Radius.z;
        f32 Pz = P.z + Radius.z;

        v4 P0 = {Nx, Ny, Pz, 0};
        v4 P1 = {Px, Ny, Pz, 0};
        v4 P2 = {Px, Py, Pz, 0};
        v4 P3 = {Nx, Py, Pz, 0};
        v4 P4 = {Nx, Ny, Nz, 0};
        v4 P5 = {Px, Ny, Nz, 0};
        v4 P6 = {Px, Py, Nz, 0};
        v4 P7 = {Nx, Py, Nz, 0};

        v4 TopColor = StoreColor(Color);
        v4 BotColor = {0, 0, 0, TopColor.a};

        v4 CT = V4(0.75*TopColor.rgb, TopColor.a);
        v4 CB = V4(0.25*TopColor.rgb, TopColor.a);

        TopColor = BotColor = CT = CB = StoreColor(Color);

        v2 T0 = {0, 0};
        v2 T1 = {1, 0};
        v2 T2 = {1, 1};
        v2 T3 = {0, 1};

        u16 LightCount = 0;
        u16 LightIndex = 0;
        if(LightStore)
        {
            v3 MinCorner = V3(Nx, Ny, Nz);
            v3 MaxCorner = V3(Px, Py, Pz);
            rectangle3 CubeBounds = RectMinMax(MinCorner, MaxCorner);
            if(RectanglesIntersect(CubeBounds, Group->LightBounds))
            {
                LightCount = LIGHT_POINTS_PER_CHUNK / 6;
                LightIndex = Commands->LightPointIndex;
                Assert(LightIndex != 0);
                Commands->LightPointIndex += LIGHT_POINTS_PER_CHUNK;
                Assert(Commands->LightPointIndex <= LIGHT_DATA_WIDTH);

                lighting_box *LightBox =
                    (Group->Commands->LightBoxes + Group->Commands->LightBoxCount++);
                Assert(Group->Commands->LightBoxCount <= LIGHT_DATA_WIDTH);

                LightBox->P = 0.5f*(MaxCorner + MinCorner);
                LightBox->Radius = 0.5f*(MaxCorner - MinCorner);
                LightBox->Transparency = 0.0f;
                LightBox->Storage = LightStore;
                LightBox->Emission = Emission;
                LightBox->RefC = Color.rgb;
                ValidateTexel(LightBox->RefC);
                LightBox->LightIndex[0] = LightIndex;
                LightBox->LightIndex[1] = LightIndex + 4;
                LightBox->LightIndex[2] = LightIndex + 8;
                LightBox->LightIndex[3] = LightIndex + 12;
                LightBox->LightIndex[4] = LightIndex + 16;
                LightBox->LightIndex[5] = LightIndex + 20;
                LightBox->LightIndex[6] = LightIndex + 24;
                LightBox->ChildCount = 0;
            }
        }

        // NOTE(casey): -X
        PushQuad(Group, Bitmap,
                 P7, T0, CB,
                 P4, T1, CB,
                 P0, T2, CT,
                 P3, T3, CT, Emission, LightCount, LightIndex);
        LightIndex += LightCount;

        // NOTE(casey): +X
        PushQuad(Group, Bitmap,
                 P1, T0, CT,
                 P5, T1, CB,
                 P6, T2, CB,
                 P2, T3, CT, Emission, LightCount, LightIndex);
        LightIndex += LightCount;

        // NOTE(casey): -Y
        PushQuad(Group, Bitmap,
                 P4, T0, CB,
                 P5, T1, CB,
                 P1, T2, CT,
                 P0, T3, CT, Emission, LightCount, LightIndex);
        LightIndex += LightCount;

        // NOTE(casey): +Y
        PushQuad(Group, Bitmap,
                 P2, T0, CT,
                 P6, T1, CB,
                 P7, T2, CB,
                 P3, T3, CT, Emission, LightCount, LightIndex);
        LightIndex += LightCount;

        // NOTE(casey): -Z
        PushQuad(Group, Bitmap,
                 P7, T0, BotColor,
                 P6, T1, BotColor,
                 P5, T2, BotColor,
                 P4, T3, BotColor, Emission, LightCount, LightIndex);
        LightIndex += LightCount;

        // NOTE(casey): +Z
        PushQuad(Group, Bitmap,
                 P0, T0, TopColor,
                 P1, T1, TopColor,
                 P2, T2, TopColor,
                 P3, T3, TopColor, Emission, LightCount, LightIndex);
        LightIndex += LightCount;
    }
}

inline void
PushCube(render_group *Group, bitmap_id ID, v3 P, v3 Radius, v4 Color = V4(1, 1, 1, 1))
{
    loaded_bitmap *Bitmap = GetBitmap(Group->Assets, ID, Group->GenerationID);
    if(Bitmap)
    {
        PushCube(Group, Bitmap, P, Radius, Color);
    }
    else
    {
        LoadBitmap(Group->Assets, ID, false);
        ++Group->MissingResourceCount;
    }
}

inline void
PushLight(render_group *Group, v3 P, v3 Radius, v3 Color, f32 Emission, lighting_point_state *LightStore)
{
    PushCube(Group, Group->Commands->WhiteBitmap, P, Radius, V4(Color, 1.0f), Emission, LightStore);
}

inline void
PushVolumeOutline(render_group *Group, object_transform *ObjectTransform, rectangle3 Rectangle, v4 Color = V4(1, 1, 1, 1), f32 Thickness = 0.1f)
{
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 6);
    if(Entry)
    {
        loaded_bitmap *Bitmap = Group->Commands->WhiteBitmap;

        v3 OffsetP = ObjectTransform->OffsetP;

        f32 Nx = OffsetP.x + Rectangle.Min.x;
        f32 Px = OffsetP.x + Rectangle.Max.x;
        f32 Ny = OffsetP.y + Rectangle.Min.y;
        f32 Py = OffsetP.y + Rectangle.Max.y;
        f32 Nz = OffsetP.z + Rectangle.Min.z;
        f32 Pz = OffsetP.z + Rectangle.Max.z;

        v3 P0 = {Nx, Ny, Pz};
        v3 P1 = {Px, Ny, Pz};
        v3 P2 = {Px, Py, Pz};
        v3 P3 = {Nx, Py, Pz};
        v3 P4 = {Nx, Ny, Nz};
        v3 P5 = {Px, Ny, Nz};
        v3 P6 = {Px, Py, Nz};
        v3 P7 = {Nx, Py, Nz};

        v4 C = StoreColor(Color);

        PushLineSegment(Group, Bitmap, P0, C, P1, C, Thickness);
        PushLineSegment(Group, Bitmap, P0, C, P3, C, Thickness);
        PushLineSegment(Group, Bitmap, P0, C, P4, C, Thickness);

        PushLineSegment(Group, Bitmap, P2, C, P1, C, Thickness);
        PushLineSegment(Group, Bitmap, P2, C, P3, C, Thickness);
        PushLineSegment(Group, Bitmap, P2, C, P6, C, Thickness);

        PushLineSegment(Group, Bitmap, P5, C, P1, C, Thickness);
        PushLineSegment(Group, Bitmap, P5, C, P4, C, Thickness);
        PushLineSegment(Group, Bitmap, P5, C, P6, C, Thickness);

        PushLineSegment(Group, Bitmap, P7, C, P3, C, Thickness);
        PushLineSegment(Group, Bitmap, P7, C, P4, C, Thickness);
        PushLineSegment(Group, Bitmap, P7, C, P6, C, Thickness);
    }
}

inline loaded_font *
PushFont(render_group *Group, font_id ID)
{
    loaded_font *Font = GetFont(Group->Assets, ID, Group->GenerationID);
    if(Font)
    {
        // NOTE(casey): Nothing to do
    }
    else
    {
        LoadFont(Group->Assets, ID, false);
        ++Group->MissingResourceCount;
    }

    return(Font);
}

inline void
PushRect(render_group *Group, object_transform *ObjectTransform, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    v3 P = (Offset - V3(0.5f*Dim, 0));
    v3 BasisP = GetRenderEntityBasisP(ObjectTransform, P);

    v2 ScaledDim = Dim;

    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1);
    if(Entry)
    {
        v4 PremulColor = StoreColor(Color);
        u32 C = RGBAPack4x8(255.0f*PremulColor);

        v3 MinP = BasisP;
        v3 MaxP = MinP + V3(ScaledDim, 0);

        v2 MinUV = {0, 0};
        v2 MaxUV = {1, 1};

        loaded_bitmap *Bitmap = Group->Commands->WhiteBitmap;
        PushQuad(Group, Bitmap,
                 V4(MinP.x, MinP.y, MinP.z, 0), V2(MinUV.x, MinUV.y), C,
                 V4(MaxP.x, MinP.y, MinP.z, 0), V2(MaxUV.x, MinUV.y), C,
                 V4(MaxP.x, MaxP.y, MinP.z, 0), V2(MaxUV.x, MaxUV.y), C,
                 V4(MinP.x, MaxP.y, MinP.z, 0), V2(MinUV.x, MaxUV.y), C);
    }
}

inline void
PushRect(render_group *Group, object_transform *ObjectTransform, rectangle2 Rectangle, r32 Z, v4 Color = V4(1, 1, 1, 1))
{
    PushRect(Group, ObjectTransform, V3(GetCenter(Rectangle), Z), GetDim(Rectangle), Color);
}

inline void
PushRectOutline(render_group *Group, object_transform *ObjectTransform, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1), real32 Thickness = 0.1f)
{
    // NOTE(casey): Top and bottom
    PushRect(Group, ObjectTransform, Offset - V3(0, 0.5f*Dim.y, 0), V2(Dim.x-Thickness-0.01f, Thickness), Color);
    PushRect(Group, ObjectTransform, Offset + V3(0, 0.5f*Dim.y, 0), V2(Dim.x-Thickness-0.01f, Thickness), Color);

    // NOTE(casey): Left and right
    PushRect(Group, ObjectTransform, Offset - V3(0.5f*Dim.x, 0, 0), V2(Thickness, Dim.y + Thickness), Color);
    PushRect(Group, ObjectTransform, Offset + V3(0.5f*Dim.x, 0, 0), V2(Thickness, Dim.y + Thickness), Color);
}

inline void
PushRectOutline(render_group *Group, object_transform *ObjectTransform, rectangle2 Rectangle, r32 Z, v4 Color = V4(1, 1, 1, 1), real32 Thickness = 0.1f)
{
    PushRectOutline(Group, ObjectTransform, V3(GetCenter(Rectangle), Z), GetDim(Rectangle), Color, Thickness);
}

#if 0
inline void
CoordinateSystem(render_group *Group, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                 loaded_bitmap *Texture, loaded_bitmap *NormalMap,
                 environment_map *Top, environment_map *Middle, environment_map *Bottom)
{
    entity_basis_p_result Basis = GetRenderEntityBasisP(RenderGroup, &Entry->EntityBasis, ScreenDim);
    if(Basis.Valid)
    {
        render_entry_coordinate_system *Entry = PushRenderElement(Group, render_entry_coordinate_system);
        if(Entry)
        {
            Entry->Origin = Origin;
            Entry->XAxis = XAxis;
            Entry->YAxis = YAxis;
            Entry->Color = Color;
            Entry->Texture = Texture;
            Entry->NormalMap = NormalMap;
            Entry->Top = Top;
            Entry->Middle = Middle;
            Entry->Bottom = Bottom;
        }
    }
}
#endif

inline r32
FitCameraDistanceToHalfDim(f32 FocalLength, f32 MonitorHalfDimInMeters, f32 HalfDimInMeters)
{
    f32 Result = (FocalLength*HalfDimInMeters) / MonitorHalfDimInMeters;
    return(Result);
}

inline v2
FitCameraDistanceToHalfDim(f32 FocalLength, f32 MonitorHalfDimInMeters, v2 HalfDimInMeters)
{
    v2 Result =
    {
        FitCameraDistanceToHalfDim(FocalLength, MonitorHalfDimInMeters, HalfDimInMeters.x),
        FitCameraDistanceToHalfDim(FocalLength, MonitorHalfDimInMeters, HalfDimInMeters.y),
    };

    return(Result);
}

inline v3
Unproject(render_group *Group, render_transform *XForm,
          v2 PixelsXY, f32 WorldDistanceFromCameraZ)
{
    v4 ProbeZ = V4(XForm->P - WorldDistanceFromCameraZ*XForm->Z, 1.0f);
    ProbeZ = XForm->Proj.Forward*ProbeZ;

//    f32 ClipZ = ProbeZ.z / ProbeZ.w;

    v2 ScreenCenter = 0.5f*Group->ScreenDim;

    v2 ClipSpaceXY = (PixelsXY - ScreenCenter);
    ClipSpaceXY.x *= 2.0f / Group->ScreenDim.x;
    ClipSpaceXY.y *= 2.0f / Group->ScreenDim.y;

    ClipSpaceXY.x *= ProbeZ.w;
    ClipSpaceXY.y *= ProbeZ.w;

    v4 Clip = V4(ClipSpaceXY.x, ClipSpaceXY.y, ProbeZ.z, ProbeZ.w);

    v4 WorldP = XForm->Proj.Inverse*Clip;
    v3 Result = WorldP.xyz;

    return(Result);
}

inline rectangle3
GetCameraRectangleAtDistance(render_group *Group, real32 WorldDistanceFromCameraZ)
{
    v3 MinCorner = Unproject(Group, &Group->GameXForm, V2(0, 0), WorldDistanceFromCameraZ);
    v3 MaxCorner = Unproject(Group, &Group->GameXForm, Group->ScreenDim, WorldDistanceFromCameraZ);

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
GetScreenPoint(render_group *Group, object_transform *ObjectTransform, v3 WorldP)
{
    v4 P = Group->LastSetup.Proj*V4((WorldP + ObjectTransform->OffsetP), 1.0f);
    P.xyz /= P.w;

    P.x = 0.5f*Group->ScreenDim.x*(P.x + 1.0f);
    P.y = 0.5f*Group->ScreenDim.y*(P.y + 1.0f);

    return(P.xy);
}

inline rectangle2i
GetClipRect(render_group *Group, object_transform *ObjectTransform, v3 Offset, v2 Dim)
{
    v2 MinCorner = GetScreenPoint(Group, ObjectTransform, Offset);
    v2 MaxCorner = GetScreenPoint(Group, ObjectTransform, Offset + V3(Dim, 0));
    rectangle2i Result = RectMinMax(RoundReal32ToUInt32(MinCorner.x),
                                    RoundReal32ToUInt32(MinCorner.y),
                                    RoundReal32ToUInt32(MaxCorner.x),
                                    RoundReal32ToUInt32(MaxCorner.y));
    return(Result);
}

inline rectangle2i
GetClipRect(render_group *Group, object_transform *ObjectTransform, rectangle2 Rectangle, r32 Z)
{
    rectangle2i Result = GetClipRect(Group, ObjectTransform, V3(Rectangle.Min, Z),
                                     GetDim(Rectangle));
    return(Result);
}

inline void
SetCameraTransform(render_group *RenderGroup, u32 Flags, f32 FocalLength,
                   v3 CameraX = V3(1, 0, 0),
                   v3 CameraY = V3(0, 1, 0),
                   v3 CameraZ = V3(0, 0, 1),
                   v3 CameraP = V3(0, 0, 0),
                   f32 NearClipPlane = 0.1f,
                   f32 FarClipPlane = 100.0f,
                   b32 Fog = false,
                   v3 DebugLightP = V3(0, 0, 0))
{
    b32x Ortho = (Flags & Camera_IsOrthographic);
    b32x IsDebug = (Flags & Camera_IsDebug);

    f32 b = SafeRatio1((r32)RenderGroup->Commands->Settings.Width, (r32)RenderGroup->Commands->Settings.Height);

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

    if(Fog)
    {
        NewSetup.FogDirection = -CameraZ;
        NewSetup.FogStartDistance = 8.0f;
        NewSetup.FogEndDistance = 20.0f;
        NewSetup.ClipAlphaStartDistance = NearClipPlane + 2.0f;
        NewSetup.ClipAlphaEndDistance = NearClipPlane + 2.25f;
    }
    else
    {
        NewSetup.FogDirection = V3(0, 0, 0);
        NewSetup.ClipAlphaStartDistance = NearClipPlane - 100.0f;
        NewSetup.ClipAlphaEndDistance = NearClipPlane - 99.0f;
    }

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

inline u32
PushRenderTarget(render_group *RenderGroup, u32 RenderTargetIndex)
{
    render_setup NewSetup = RenderGroup->LastSetup;
    NewSetup.RenderTargetIndex = RenderTargetIndex;
    PushSetup(RenderGroup, &NewSetup);
}

inline render_group
BeginRenderGroup(game_assets *Assets, game_render_commands *Commands, u32 GenerationID,
                 int32 PixelWidth, int32 PixelHeight)
{
    render_group Result = {};

    Result.Assets = Assets;
    Result.MissingResourceCount = 0;
    Result.GenerationID = GenerationID;
    Result.Commands = Commands;
    Result.ScreenDim = V2i(PixelWidth, PixelHeight);

    render_setup InitialSetup = {};
    InitialSetup.FogStartDistance = 0.0f;
    InitialSetup.FogEndDistance = 1.0f;
    InitialSetup.ClipAlphaStartDistance = 0.0f;
    InitialSetup.ClipAlphaEndDistance = 1.0f;
    InitialSetup.ClipRect = RectMinDim(0, 0, PixelWidth, PixelHeight);
    InitialSetup.Proj = Identity();
    PushSetup(&Result, &InitialSetup);

    return(Result);
}

inline void
EndRenderGroup(render_group *RenderGroup)
{
    // TODO(casey):
    // RenderGroup->Commands->MissingResourceCount += RenderGroup->MissingResourceCount;
}

inline void
PushDepthClear(render_group *Group)
{
    PushRenderElement_(Group, 0, RenderGroupEntryType_render_entry_depth_clear);
}

inline void
PushFullClear(render_group *Group, v4 Color)
{
    render_entry_full_clear *Entry = PushRenderElement(Group, render_entry_full_clear);
    if(Entry)
    {
        Entry->ClearColor = Color;
        Group->LastSetup.FogColor = V3(Square(Color.r), Square(Color.g), Square(Color.b));
    }
}

inline void
BeginDepthPeel(render_group *Group, v4 Color)
{
    render_entry_begin_peels *Entry = PushRenderElement(Group, render_entry_begin_peels);
    if(Entry)
    {
        Entry->ClearColor = Color;
        Group->LastSetup.FogColor = V3(Square(Color.r), Square(Color.g), Square(Color.b));
    }
}

inline void
EndDepthPeel(render_group *Group)
{
    PushRenderElement_(Group, 0, RenderGroupEntryType_render_entry_end_peels);
}
