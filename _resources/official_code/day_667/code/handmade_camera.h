/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct camera
{
    v3 Offset;
    v3 Shift;
    
    f32 Pitch;
    f32 Orbit;
    f32 Dolly;

    f32 FocalLength;
    f32 NearClipPlane;
    f32 FarClipPlane;
    
    f32 FogStart;
    f32 FogEnd;
    
    f32 ClipAlphaStart;
    f32 ClipAlphaEnd;
};
