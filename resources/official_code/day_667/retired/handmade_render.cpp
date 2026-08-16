/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

global b32 Global_Renderer_ShowLightingSamples = false;

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

internal rectangle2i
AspectRatioFit(u32 RenderWidth, u32 RenderHeight,
               u32 WindowWidth, u32 WindowHeight)
{
    rectangle2i Result = {};
    
    if((RenderWidth > 0) &&
       (RenderHeight > 0) &&
       (WindowWidth > 0) &&
       (WindowHeight > 0))
    {
        r32 OptimalWindowWidth = (r32)WindowHeight * ((r32)RenderWidth / (r32)RenderHeight);
        r32 OptimalWindowHeight = (r32)WindowWidth * ((r32)RenderHeight / (r32)RenderWidth);
        
        if(OptimalWindowWidth > (r32)WindowWidth)
        {
            // NOTE(casey): Width-constrained display - top and bottom black bars
            Result.MinX = 0;
            Result.MaxX = WindowWidth;
            
            r32 Empty = (r32)WindowHeight - OptimalWindowHeight;
            s32 HalfEmpty = RoundReal32ToInt32(0.5f*Empty);            
            s32 UseHeight = RoundReal32ToInt32(OptimalWindowHeight);
            
            Result.MinY = HalfEmpty;
            Result.MaxY = Result.MinY + UseHeight;
        }
        else
        {
            // NOTE(casey): Height-constrained display - left and right black bars
            Result.MinY = 0;
            Result.MaxY = WindowHeight;
            
            r32 Empty = (r32)WindowWidth - OptimalWindowWidth;
            s32 HalfEmpty = RoundReal32ToInt32(0.5f*Empty);            
            s32 UseWidth = RoundReal32ToInt32(OptimalWindowWidth);
            
            Result.MinX = HalfEmpty;
            Result.MaxX = Result.MinX + UseWidth;
        }
    }
    
    return(Result);
}


