/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include <windows.h>
#include <gl/gl.h>
#include <stdio.h>
#include <tchar.h>

#include "handmade_types.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_shared.h"

// TODO(casey): Maybe try to compute spheres at run-time instead, so they
// can be created based on the desired octahedral sampling density at
// run-time?
// {
#include "handmade_simd.h"
#include "handmade_light_atlas.h"
#include "handmade_renderer.h"
#include "handmade_image.h"
// }

#include "win32_handmade_renderer.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB           0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             0x2093
#define WGL_CONTEXT_FLAGS_ARB                   0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB               0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                  0x2001
#define WGL_ACCELERATION_ARB                    0x2003
#define WGL_SUPPORT_OPENGL_ARB                  0x2010
#define WGL_DOUBLE_BUFFER_ARB                   0x2011
#define WGL_PIXEL_TYPE_ARB                      0x2013

#define WGL_TYPE_RGBA_ARB                       0x202B
#define WGL_FULL_ACCELERATION_ARB               0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB        0x20A9

#define WGL_RED_BITS_ARB                        0x2015
#define WGL_GREEN_BITS_ARB                      0x2017
#define WGL_BLUE_BITS_ARB                       0x2019
#define WGL_ALPHA_BITS_ARB                      0x201B
#define WGL_DEPTH_BITS_ARB                      0x2022

typedef HGLRC WINAPI wgl_create_context_attribs_arb(HDC hDC, HGLRC hShareContext,
                                                    const int *attribList);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_iv_arb(HDC hdc,
                                                       int iPixelFormat,
                                                       int iLayerPlane,
                                                       UINT nAttributes,
                                                       const int *piAttributes,
                                                       int *piValues);

typedef BOOL WINAPI wgl_get_pixel_format_attrib_fv_arb(HDC hdc,
                                                       int iPixelFormat,
                                                       int iLayerPlane,
                                                       UINT nAttributes,
                                                       const int *piAttributes,
                                                       FLOAT *pfValues);

typedef BOOL WINAPI wgl_choose_pixel_format_arb(HDC hdc,
                                                const int *piAttribIList,
                                                const FLOAT *pfAttribFList,
                                                UINT nMaxFormats,
                                                int *piFormats,
                                                UINT *nNumFormats);

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

typedef void WINAPI type_glTexImage2DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void WINAPI type_glBindFramebuffer(GLenum target, GLuint framebuffer);
typedef void WINAPI type_glGenFramebuffers(GLsizei n, GLuint *framebuffers);
typedef void WINAPI type_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef GLenum WINAPI type_glCheckFramebufferStatus(GLenum target);
typedef void WINAPI type_glBlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void WINAPI type_glAttachShader(GLuint program, GLuint shader);
typedef void WINAPI type_glCompileShader(GLuint shader);
typedef GLuint WINAPI type_glCreateProgram(void);
typedef GLuint WINAPI type_glCreateShader(GLenum type);
typedef void WINAPI type_glLinkProgram(GLuint program);
typedef void WINAPI type_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void WINAPI type_glUseProgram(GLuint program);
typedef void WINAPI type_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI type_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void WINAPI type_glValidateProgram(GLuint program);
typedef void WINAPI type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef GLint WINAPI type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void WINAPI type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI type_glUniform1i(GLint location, GLint v0);

typedef void WINAPI type_glUniform1f(GLint location, GLfloat v0);
typedef void WINAPI type_glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);

typedef void WINAPI type_glEnableVertexAttribArray(GLuint index);
typedef void WINAPI type_glDisableVertexAttribArray(GLuint index);
typedef GLint WINAPI type_glGetAttribLocation(GLuint program, const GLchar *name);
typedef void WINAPI type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void WINAPI type_glVertexAttribIPointer (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer);
typedef void WINAPI type_glBindVertexArray(GLuint array);
typedef void WINAPI type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void WINAPI type_glBindBuffer (GLenum target, GLuint buffer);
typedef void WINAPI type_glGenBuffers (GLsizei n, GLuint *buffers);
typedef void WINAPI type_glBufferData (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void WINAPI type_glActiveTexture (GLenum texture);
typedef void WINAPI type_glDeleteProgram (GLuint program);
typedef void WINAPI type_glDeleteShader (GLuint shader);
typedef void WINAPI type_glDeleteFramebuffers (GLsizei n, const GLuint *framebuffers);
typedef void WINAPI type_glDrawBuffers (GLsizei n, const GLenum *bufs);
typedef void WINAPI type_glTexImage3D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels);
typedef void WINAPI type_glTexSubImage3D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels);

typedef void WINAPI type_glDrawElementsBaseVertex (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex);

#define GL_DEBUG_CALLBACK(Name) void WINAPI Name(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
typedef GL_DEBUG_CALLBACK(GLDEBUGPROC);
typedef void WINAPI type_glDebugMessageCallbackARB(GLDEBUGPROC *callback, const void *userParam);

typedef const GLubyte * WINAPI type_glGetStringi(GLenum name, GLuint index);

typedef BOOL WINAPI wgl_swap_interval_ext(int interval);
typedef const char * WINAPI wgl_get_extensions_string_ext(void);

global wgl_create_context_attribs_arb *wglCreateContextAttribsARB;
global wgl_choose_pixel_format_arb *wglChoosePixelFormatARB;
global wgl_swap_interval_ext *wglSwapIntervalEXT;
global wgl_get_extensions_string_ext *wglGetExtensionsStringEXT;

#include "handmade_renderer_opengl.h"
#include "handmade_renderer_opengl.cpp"
#include "handmade_image.cpp"
#include "handmade_light_atlas.cpp"

global int Win32OpenGLAttribs[] =
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB
#if HANDMADE_INTERNAL
        |WGL_CONTEXT_DEBUG_BIT_ARB
#endif
        ,
#if 0
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
    0,
};

internal void
Win32SetPixelFormat(open_gl *OpenGL, HDC WindowDC)
{
    int SuggestedPixelFormatIndex = 0;
    GLuint ExtendedPick = 0;
    if(wglChoosePixelFormatARB)
    {
        int IntAttribList[] =
        {
            WGL_DRAW_TO_WINDOW_ARB, GL_TRUE, // 0
            WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB, // 1
            WGL_SUPPORT_OPENGL_ARB, GL_TRUE, // 2
            WGL_DOUBLE_BUFFER_ARB, GL_TRUE, // 3
            WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB, // 4
            WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE, // 5
            0,
        };
        
        if(!OpenGL->SupportsSRGBFramebuffer)
        {
            IntAttribList[10] = 0;
        }
        
        wglChoosePixelFormatARB(WindowDC, IntAttribList, 0, 1,
                                &SuggestedPixelFormatIndex, &ExtendedPick);
    }
    
    if(!ExtendedPick)
    {
        // TODO(casey): Hey Raymond Chen - what's the deal here?
        // Is cColorBits ACTUALLY supposed to exclude the alpha bits, like MSDN says, or not?
        PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
        DesiredPixelFormat.nSize = sizeof(DesiredPixelFormat);
        DesiredPixelFormat.nVersion = 1;
        DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
        DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
        DesiredPixelFormat.cColorBits = 32;
        DesiredPixelFormat.cAlphaBits = 8;
        DesiredPixelFormat.cDepthBits = 24;
        DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
        
        SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);
    }
    
    PIXELFORMATDESCRIPTOR SuggestedPixelFormat;
    // NOTE(casey): Technically you do not need to call DescribePixelFormat here,
    // as SetPixelFormat doesn't actually need it to be filled out properly.
    DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex,
                        sizeof(SuggestedPixelFormat), &SuggestedPixelFormat);
    SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestedPixelFormat);
}

internal void
Win32LoadWGLExtensions(open_gl *OpenGL)
{
    WNDCLASSA WindowClass = {};
    
    WindowClass.lpfnWndProc = DefWindowProcA;
    WindowClass.hInstance = GetModuleHandle(0);
    WindowClass.lpszClassName = "HandmadeWGLLoader";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Handmade Hero",
            0,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            WindowClass.hInstance,
            0);
        
        HDC WindowDC = GetDC(Window);
        Win32SetPixelFormat(OpenGL, WindowDC);
        HGLRC OpenGLRC = wglCreateContext(WindowDC);
        if(wglMakeCurrent(WindowDC, OpenGLRC))
        {
            wglChoosePixelFormatARB =
                (wgl_choose_pixel_format_arb *)wglGetProcAddress("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB =
                (wgl_create_context_attribs_arb *)wglGetProcAddress("wglCreateContextAttribsARB");
            wglGetExtensionsStringEXT = (wgl_get_extensions_string_ext *)wglGetProcAddress("wglGetExtensionsStringEXT");
            
            if(wglGetExtensionsStringEXT)
            {
                char *Extensions = (char *)wglGetExtensionsStringEXT();
                char *At = Extensions;
                while(*At)
                {
                    while(IsWhitespace(*At)) {++At;}
                    char *End = At;
                    while(*End && !IsWhitespace(*End)) {++End;}
                    
                    umm Count = End - At;
                    
                    if(0) {}
                    else if(StringsAreEqual(Count, At, "WGL_EXT_framebuffer_sRGB")) {OpenGL->SupportsSRGBFramebuffer = true;}
                    else if(StringsAreEqual(Count, At, "WGL_ARB_framebuffer_sRGB")) {OpenGL->SupportsSRGBFramebuffer = true;}
                    
                    At = End;
                }
            }
            
            wglMakeCurrent(0, 0);
        }
        
        wglDeleteContext(OpenGLRC);
        ReleaseDC(Window, WindowDC);
        DestroyWindow(Window);
    }
}

internal void *
Win32RendererAlloc(umm Size)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return(Result);
}

internal void
PlatformOpenGLSetVSync(open_gl *OpenGL, b32x VSyncEnabled)
{
    if(!wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT = (wgl_swap_interval_ext *)wglGetProcAddress("wglSwapIntervalEXT");
    }
    
    if(wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT(VSyncEnabled ? 1 : 0);
    }
}

RENDERER_BEGIN_FRAME(Win32BeginFrame)
{
    game_render_commands *Result =
        OpenGLBeginFrame((open_gl *)Renderer, OSWindowDim, RenderDim, DrawRegion);
    return(Result);
}

RENDERER_END_FRAME(Win32EndFrame)
{
    OpenGLEndFrame((open_gl *)Renderer, Frame);
    SwapBuffers(wglGetCurrentDC());
}

internal open_gl *
Win32InitOpenGL(HDC WindowDC, platform_renderer_limits *Limits)
{
    open_gl *OpenGL = (open_gl *)Win32RendererAlloc(sizeof(open_gl));
    InitTextureQueue(&OpenGL->Header.TextureQueue, Limits->TextureTransferBufferSize,
                     Win32RendererAlloc(Limits->TextureTransferBufferSize));
    Win32LoadWGLExtensions(OpenGL);
    Win32SetPixelFormat(OpenGL, WindowDC);
    
    u32 MaxVertexCount = Limits->MaxQuadCountPerFrame*4;
    u32 MaxIndexCount = Limits->MaxQuadCountPerFrame*6;
    OpenGL->MaxTextureCount = Limits->MaxTextureCount;
    OpenGL->MaxVertexCount = MaxVertexCount;
    OpenGL->MaxIndexCount = MaxIndexCount;
    OpenGL->MaxSpecialTextureCount = Limits->MaxSpecialTextureCount;
    
    // NOTE(casey): This is wayyyyyy overkill because you would not render all your quads
    // as separate textures, so this wastes a ton of memory.  At some point we may want
    // to restrict the number of these you could draw with separate textures.
    OpenGL->MaxQuadTextureCount = Limits->MaxQuadCountPerFrame;
    
    OpenGL->VertexArray = (textured_vertex *)Win32RendererAlloc(MaxVertexCount*sizeof(textured_vertex));
    OpenGL->IndexArray = (u16 *)Win32RendererAlloc(MaxIndexCount*sizeof(u16));
    OpenGL->BitmapArray = (renderer_texture *)Win32RendererAlloc(OpenGL->MaxQuadTextureCount*sizeof(renderer_texture));
    OpenGL->SpecialTextureHandles = (GLuint *)Win32RendererAlloc(Limits->MaxSpecialTextureCount*sizeof(GLuint));
    
    v3s LightAtlasVoxelDim = V3S(LIGHT_LOOKUP_VOXEL_DIM_X, LIGHT_LOOKUP_VOXEL_DIM_Y, LIGHT_LOOKUP_VOXEL_DIM_Z);
    
    u32 LIGHT_COLOR_LOOKUP_SQUARE_DIM = (LIGHTING_OCTAHEDRAL_MAP_DIM+2);
    v2u LightAtlasTileDim = V2U(LIGHT_COLOR_LOOKUP_SQUARE_DIM, LIGHT_COLOR_LOOKUP_SQUARE_DIM);
    OpenGL->DiffuseLightAtlas = MakeLightAtlas(LightAtlasVoxelDim, LightAtlasTileDim);
    SetLightAtlasTexels(&OpenGL->DiffuseLightAtlas,
                        Win32RendererAlloc(GetLightAtlasSize(&OpenGL->DiffuseLightAtlas)));
    OpenGL->SpecularLightAtlas = MakeLightAtlas(LightAtlasVoxelDim, LightAtlasTileDim);
    SetLightAtlasTexels(&OpenGL->SpecularLightAtlas,
                        Win32RendererAlloc(GetLightAtlasSize(&OpenGL->SpecularLightAtlas)));
    
    b32 ModernContext = true;
    HGLRC OpenGLRC = 0;
    if(wglCreateContextAttribsARB)
    {
        OpenGLRC = wglCreateContextAttribsARB(WindowDC, 0, Win32OpenGLAttribs);
    }
    
    if(!OpenGLRC)
    {
        ModernContext = false;
        OpenGLRC = wglCreateContext(WindowDC);
    }
    
    if(wglMakeCurrent(WindowDC, OpenGLRC))
    {
        
#define Win32GetOpenGLFunction(Name) OpenGL->Name = (type_##Name *)wglGetProcAddress(#Name)
        
        Win32GetOpenGLFunction(glTexImage2DMultisample);
        Win32GetOpenGLFunction(glBlitFramebuffer);
        Win32GetOpenGLFunction(glAttachShader);
        Win32GetOpenGLFunction(glCompileShader);
        Win32GetOpenGLFunction(glCreateProgram);
        Win32GetOpenGLFunction(glCreateShader);
        Win32GetOpenGLFunction(glLinkProgram);
        Win32GetOpenGLFunction(glShaderSource);
        Win32GetOpenGLFunction(glUseProgram);
        Win32GetOpenGLFunction(glGetProgramInfoLog);
        Win32GetOpenGLFunction(glGetShaderInfoLog);
        Win32GetOpenGLFunction(glValidateProgram);
        Win32GetOpenGLFunction(glGetProgramiv);
        Win32GetOpenGLFunction(glGetUniformLocation);
        Win32GetOpenGLFunction(glUniform4fv);
        Win32GetOpenGLFunction(glUniformMatrix4fv);
        Win32GetOpenGLFunction(glUniform1i);
        Win32GetOpenGLFunction(glEnableVertexAttribArray);
        Win32GetOpenGLFunction(glDisableVertexAttribArray);
        Win32GetOpenGLFunction(glGetAttribLocation);
        Win32GetOpenGLFunction(glVertexAttribPointer);
        Win32GetOpenGLFunction(glVertexAttribIPointer);
        Win32GetOpenGLFunction(glDebugMessageCallbackARB);
        Win32GetOpenGLFunction(glBindVertexArray);
        Win32GetOpenGLFunction(glGenVertexArrays);
        Win32GetOpenGLFunction(glBindBuffer);
        Win32GetOpenGLFunction(glGenBuffers);
        Win32GetOpenGLFunction(glBufferData);
        Win32GetOpenGLFunction(glActiveTexture);
        Win32GetOpenGLFunction(glGetStringi);
        Win32GetOpenGLFunction(glDeleteProgram);
        Win32GetOpenGLFunction(glDeleteShader);
        Win32GetOpenGLFunction(glDeleteFramebuffers);
        Win32GetOpenGLFunction(glDrawBuffers);
        Win32GetOpenGLFunction(glTexImage3D);
        Win32GetOpenGLFunction(glTexSubImage3D);
        Win32GetOpenGLFunction(glDrawElementsBaseVertex);
        Win32GetOpenGLFunction(glUniform1f);
        Win32GetOpenGLFunction(glUniform2fv);
        Win32GetOpenGLFunction(glUniform3fv);
        
        opengl_info Info = OpenGLGetInfo(OpenGL, ModernContext);
        if(Info.OpenGL_ARB_framebuffer_object)
        {
            Win32GetOpenGLFunction(glBindFramebuffer);
            Win32GetOpenGLFunction(glGenFramebuffers);
            Win32GetOpenGLFunction(glFramebufferTexture2D);
            Win32GetOpenGLFunction(glCheckFramebufferStatus);
        }
        
        PlatformOpenGLSetVSync(OpenGL, true);
        OpenGLInit(OpenGL, Info, OpenGL->SupportsSRGBFramebuffer);
    }
    
    return(OpenGL);
}

WIN32_LOAD_RENDERER_ENTRY()
{
    platform_renderer *Result = (platform_renderer *)Win32InitOpenGL(WindowDC, Limits);
    
    return(Result);
}

#if 0
internal void
Win32DisplayBufferInWindow(platform_work_queue *RenderQueue, game_render_commands *Commands,
                           HDC DeviceContext, rectangle2i DrawRegion, u32 WindowWidth, u32 WindowHeight,
                           memory_arena *TempArena)
{
    temporary_memory TempMem = BeginTemporaryMemory(TempArena);
    
    /*  TODO(casey): Do we want to check for resources like before?  Probably?
        if(AllResourcesPresent(RenderGroup))
        {
            RenderToOutput(TranState->HighPriorityQueue, RenderGroup, &DrawBuffer, &TranState->TranArena);
        }
    */
    
    if(GlobalSoftwareRendering)
    {
        software_texture OutputTarget;
        OutputTarget.Memory = GlobalBackbuffer.Memory;
        OutputTarget.Width = GlobalBackbuffer.Width;
        OutputTarget.Height = GlobalBackbuffer.Height;
        OutputTarget.Pitch = GlobalBackbuffer.Pitch;
        
        SoftwareRenderCommands(RenderQueue, Commands, &OutputTarget, TempArena);
        
        // TODO(casey): Track clears so we clear the backbuffer to the right color?
        v4 ClearColor = {};
        OpenGLDisplayBitmap(GlobalBackbuffer.Width, GlobalBackbuffer.Height, GlobalBackbuffer.Memory,
                            GlobalBackbuffer.Pitch, DrawRegion, ClearColor,
                            OpenGL.ReservedBlitTexture);
    }
    else
    {
        BEGIN_BLOCK("OpenGLRenderCommands");
        OpenGLRenderCommands(Commands, DrawRegion, WindowWidth, WindowHeight);
        END_BLOCK();
    }
    
    EndTemporaryMemory(TempMem);
}
#endif
