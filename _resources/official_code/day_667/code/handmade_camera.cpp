/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

internal camera
GetStandardCamera(void)
{
    camera Result;

    Result.Pitch = 0.3f*Pi32; // Tilt of the camera
    Result.Orbit = 0; // Rotation of the camera around the subject
    Result.Dolly = 20.0f; // Distance away from the subject
    
    Result.FocalLength = 3.0f; // Amount of perspective foreshortening
    Result.NearClipPlane = 0.2f; // Closest you can be to the camera and still be seen
    Result.FarClipPlane = 1000.0f; // Furthest you can be from the camera and still be seen
    
    Result.Offset = {0, 0, 0};
    Result.Shift = {0, 0, -1.0f}; // Amount to drop the camera down from the center of the subject
    
    Result.FogStart = 8.0f;
    Result.FogEnd = 20.0f;
    
    Result.ClipAlphaStart = 2.0f;
    Result.ClipAlphaEnd = 2.25f;
    
    return(Result);
}

internal m4x4
BuildCameraObjectMatrix(v3 Offset, f32 Orbit, f32 Pitch, f32 Dolly)
{
    m4x4 Result = (Translation(Offset) *
                   ZRotation(Orbit) *
                   XRotation(Pitch) *
                   Translation(V3(0, 0, Dolly)));
    
    return(Result);
}

internal m4x4
BuildCameraObjectMatrix(camera *Camera)
{
    m4x4 Result = BuildCameraObjectMatrix(Camera->Offset + Camera->Shift, Camera->Orbit, Camera->Pitch, Camera->Dolly);
    return(Result);
}

internal void
ViewFromCamera(render_group *Group, camera *Camera)
{
    m4x4 CameraM = BuildCameraObjectMatrix(Camera);
    
    v3 CameraZ = GetColumn(CameraM, 2);
    fog_params Fog;
    Fog.Direction = -CameraZ;
    Fog.StartDistance = Camera->FogStart;
    Fog.EndDistance = Camera->FogEnd;
    
    alpha_clip_params AlphaClip;
    
    AlphaClip.dStartDistance = Camera->ClipAlphaStart;
    AlphaClip.dEndDistance = Camera->ClipAlphaEnd;
    
    SetCameraTransform(Group,
                       0,
                       Camera->FocalLength,
                       GetColumn(CameraM, 0),
                       GetColumn(CameraM, 1),
                       CameraZ,
                       GetColumn(CameraM, 3),
                       Camera->NearClipPlane,
                       Camera->FarClipPlane,
                       &Fog, &AlphaClip);
}
