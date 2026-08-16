/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal void
WriteQuad(geometry_output *Out,
          v4 P0, v3 N0, v2 UV0, u32 C0,
          v4 P1, v3 N1, v2 UV1, u32 C1,
          v4 P2, v3 N2, v2 UV2, u32 C2,
          v4 P3, v3 N3, v2 UV3, u32 C3,
          u16 TextureIndex)
{
    textured_vertex *Vert = Out->V;
    u16 *Index = Out->I;
    
    Vert[0].P = P3;
    Vert[0].N = N3;
    Vert[0].UV = UV3;
    Vert[0].Color = C3;
    Vert[0].TextureIndex = TextureIndex;
    Vert[0].Emission = 0;
    
    Vert[1].P = P0;
    Vert[1].N = N0;
    Vert[1].UV = UV0;
    Vert[1].Color = C0;
    Vert[1].TextureIndex = TextureIndex;
    Vert[1].Emission = 0;
    
    Vert[2].P = P2;
    Vert[2].N = N2;
    Vert[2].UV = UV2;
    Vert[2].Color = C2;
    Vert[2].TextureIndex = TextureIndex;
    Vert[2].Emission = 0;
    
    Vert[3].P = P1;
    Vert[3].N = N1;
    Vert[3].UV = UV1;
    Vert[3].Color = C1;
    Vert[3].TextureIndex = TextureIndex;
    Vert[3].Emission = 0;
    
    u16 VI = Out->BaseIndex;
    Index[0] = VI + 0;
    Index[1] = VI + 1;
    Index[2] = VI + 2;
    Index[3] = VI + 1;
    Index[4] = VI + 3;
    Index[5] = VI + 2;
    
    Out->V += 4;
    Out->I += 6;
    Out->BaseIndex += 4;
}

internal v2
WorldDimFromWorldHeight(hha_bitmap *Info, f32 Height)
{
    f32 WidthOverHeight = 1.0f;
    if(Info->OrigDim[1])
    {
        WidthOverHeight = (f32)Info->OrigDim[0] / (f32)Info->OrigDim[1];
    }
    
    v2 Result =
    {
        Height*WidthOverHeight,
        Height,
    };
    
    return(Result);
}

internal sprite_values
SpriteValuesForUpright(v3 WorldUp, v3 CameraUp, v3 XAxisH, v2 WorldDim, v2 AlignP,
                       v2 XAxis2 = V2(1, 0), v2 YAxis2 = V2(0, 1),
                       f32 tCameraUp = 0.65f)
{
    v3 YAxisH = NOZ(Lerp(WorldUp, tCameraUp, CameraUp));
    // f32 ZBias = tCameraUp*Inner(WorldUp, CameraUp)*WorldDim.y;
    
    v3 XAxis = WorldDim.x*(XAxis2.x*XAxisH + XAxis2.y*YAxisH);
    v3 YAxis = WorldDim.y*(YAxis2.x*XAxisH + YAxis2.y*YAxisH);
    
    v3 MinP = -AlignP.x*XAxis - AlignP.y*YAxis;
    
    sprite_values Result;
    Result.MinP = MinP;
    Result.ScaledXAxis = XAxis;
    Result.ScaledYAxis = YAxis;
    
    // TODO(casey): We need to assess this calculation and perhaps introduce
    // a selectable alternate version for the scenario where we have spinning
    // entities.
    // Result.ZDisplacement = AlignP.y*WorldDim.y*YAxisH.z;
    Result.ZDisplacement = AlignP.y*YAxis.z;
    
    return(Result);
}

internal sprite_values
SpriteValuesForUpright(render_group *RenderGroup, v2 WorldDim, v2 AlignP,
                       v2 XAxis2 = V2(1, 0), v2 YAxis2 = V2(0, 1),
                       f32 tCameraUp = 0.65f)
{
    v3 WorldUp = RenderGroup->WorldUp;
    v3 CameraUp = RenderGroup->GameXForm.Y;
    v3 XAxisH = RenderGroup->GameXForm.X;
    
    sprite_values Result = SpriteValuesForUpright(WorldUp, CameraUp, XAxisH, WorldDim, AlignP, XAxis2, YAxis2, tCameraUp);
    return(Result);
}

internal v3
WorldPFromAlignP(sprite_values *Values, v2 AlignP)
{
    v3 Result = (Values->MinP +
                 AlignP.x*Values->ScaledXAxis +
                 AlignP.y*Values->ScaledYAxis);
    
    return(Result);
}
