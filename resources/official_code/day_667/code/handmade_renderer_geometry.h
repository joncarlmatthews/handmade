/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct geometry_output
{
    textured_vertex *V;
    u16 *I;
    
    u16 BaseIndex;
};

struct sprite_values
{
    v3 MinP;
    v3 ScaledXAxis;
    v3 ScaledYAxis;
    f32 ZDisplacement;
};

internal void WriteQuad(geometry_output *Out,
                        v4 P0, v3 N0, v2 UV0, u32 C0,
                        v4 P1, v3 N1, v2 UV1, u32 C1,
                        v4 P2, v3 N2, v2 UV2, u32 C2,
                        v4 P3, v3 N3, v2 UV3, u32 C3,
                        u16 TextureIndex);

internal v2 WorldDimFromWorldHeight(hha_bitmap *Info, f32 Height);
internal v3 WorldPFromAlignP(sprite_values *Values, v2 AlignP);
