/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define WIN32_LOAD_RENDERER(name) platform_renderer *name(HDC WindowDC, platform_renderer_limits *Limits)
typedef WIN32_LOAD_RENDERER(win32_load_renderer);
#define WIN32_LOAD_RENDERER_ENTRY() WIN32_LOAD_RENDERER(Win32LoadRenderer)

struct win32_renderer_function_table
{
    win32_load_renderer *LoadRenderer;
    renderer_begin_frame *BeginFrame;
    renderer_end_frame *EndFrame;
};
global char *Win32RendererFunctionTableNames[] = 
{
    "Win32LoadRenderer",
    "Win32BeginFrame",
    "Win32EndFrame",
};
