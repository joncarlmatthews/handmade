/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

//
// NOTE(casey): Handmade Hero Specific
//

inline used_bitmap_dim
GetBitmapDim(render_group *Group, renderer_texture TextureHandle, real32 Height, v3 Offset, 
             v2 AlignPercentage, v2 XAxis = V2(1, 0), v2 YAxis = V2(0, 1))
{
    used_bitmap_dim Dim;
    
    f32 WidthOverHeight = 1.0f;
    if(TextureHandle.Height)
    {
        WidthOverHeight = (f32)TextureHandle.Width / (f32)TextureHandle.Height;
    }
    
    Dim.Size.x = Height*WidthOverHeight;
    Dim.Size.y = Height;
    Dim.Align.x = AlignPercentage.x*Dim.Size.x;
    Dim.Align.y = AlignPercentage.y*Dim.Size.y;
    Dim.P.z = Offset.z;
    Dim.P.x = Offset.x - Dim.Align.x*XAxis.x - Dim.Align.y*YAxis.x;
    Dim.P.y = Offset.y - Dim.Align.x*XAxis.y - Dim.Align.y*YAxis.y;
    
    return(Dim);
}

inline void
PushBitmap(render_group *Group, b32x Upright, used_bitmap_dim *Dim,
           renderer_texture TextureHandle, real32 Height, v3 Offset, v2 AlignPercentage, 
           v4 Color = V4(1, 1, 1, 1), v2 XAxis2 = V2(1, 0), v2 YAxis2 = V2(0, 1))
{
    render_entry_textured_quads *Entry = GetCurrentQuads(Group, 1, TextureHandle);
    if(Entry)
    {
        u32 BitmapWidth = TextureHandle.Width;
        u32 BitmapHeight = TextureHandle.Height;
        
        v2 Size = Dim->Size;
        v3 MinP = Dim->P;
        f32 ZBias = 0.0f;
        v4 PremulColor = StoreColor(Color);
        v3 XAxis = Size.x*V3(XAxis2, 0.0f);
        v3 YAxis = Size.y*V3(YAxis2, 0.0f);
        
        if(Upright)
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
        
        r32 OneTexelU = 1.0f / (r32)BitmapWidth;
        r32 OneTexelV = 1.0f / (r32)BitmapHeight;
        v2 MinUV = V2(OneTexelU, OneTexelV);
        v2 MaxUV = V2(1.0f - OneTexelU, 1.0f - OneTexelV);
        
        u32 Color32 = RGBAPack4x8(255.0f*PremulColor);
        
        v4 MinXMinY = V4(MinP, 0.0f);
        v4 MinXMaxY = V4(MinP + YAxis, ZBias);
        v4 MaxXMinY = V4(MinP + XAxis, 0.0f);
        v4 MaxXMaxY = V4(MinP + XAxis + YAxis, ZBias);
        
        PushQuad(Group, TextureHandle,
                 MinXMinY, V2(MinUV.x, MinUV.y), Color32,
                 MaxXMinY, V2(MaxUV.x, MinUV.y), Color32,
                 MaxXMaxY, V2(MaxUV.x, MaxUV.y), Color32,
                 MinXMaxY, V2(MinUV.x, MaxUV.y), Color32);
    }
}

inline void
PushBitmap(render_group *Group, renderer_texture TextureHandle,
           real32 Height, v3 Offset, v2 AlignPercentage, 
           v4 Color = V4(1, 1, 1, 1), v2 XAxis2 = V2(1, 0), v2 YAxis2 = V2(0, 1))
{
    u32 BitmapWidth = TextureHandle.Width;
    u32 BitmapHeight = TextureHandle.Height;
    if(BitmapWidth && BitmapHeight)
    {
        used_bitmap_dim Dim = GetBitmapDim(Group, TextureHandle, Height, Offset, AlignPercentage,
                                           XAxis2, YAxis2);
        PushBitmap(Group, false, &Dim, TextureHandle, Height, Offset, AlignPercentage,
                   Color, XAxis2, YAxis2);
    }
}

inline void
PushBitmap(render_group *Group,
           bitmap_id ID, real32 Height, v3 Offset, v2 AlignPercentage, v4 Color = V4(1, 1, 1, 1),
           v2 XAxis = V2(1, 0), v2 YAxis = V2(0, 1))
{
    renderer_texture TextureHandle = GetBitmap(Group->Assets, ID);
    if(IsValid(TextureHandle))
    {
        PushBitmap(Group, TextureHandle, Height, Offset, AlignPercentage, Color, XAxis, YAxis);
    }
    else
    {
        LoadBitmap(Group->Assets, ID);
        ++Group->MissingResourceCount;
    }
}

inline void
PushCube(render_group *Group, bitmap_id ID, v3 P, v3 Radius,
         v4 Color = V4(1, 1, 1, 1),
         cube_uv_layout UVLayout = DefaultCubeUVLayout(),
         f32 Emission = 0.0f)
{
    renderer_texture TextureHandle = GetBitmap(Group->Assets, ID);
    if(IsValid(TextureHandle))
    {
        PushCube(Group, TextureHandle, P, Radius, Color, UVLayout, Emission);
    }
    else
    {
        LoadBitmap(Group->Assets, ID);
        ++Group->MissingResourceCount;
    }
}

inline loaded_font *
PushFont(render_group *Group, font_id ID)
{
    loaded_font *Font = GetFont(Group->Assets, ID);
    if(Font)
    {
        // NOTE(casey): Nothing to do
    }
    else
    {
        LoadFont(Group->Assets, ID);
        ++Group->MissingResourceCount;
    }
    
    return(Font);
}
