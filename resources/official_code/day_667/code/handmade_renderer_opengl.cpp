/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2015 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define GL_NUM_EXTENSIONS                 0x821D

#define GL_MAX_COLOR_ATTACHMENTS          0x8CDF
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D

#define GL_TEXTURE_3D                     0x806F

#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#define GL_TEXTURE2                       0x84C2
#define GL_TEXTURE3                       0x84C3
#define GL_TEXTURE4                       0x84C4
#define GL_TEXTURE5                       0x84C5
#define GL_TEXTURE6                       0x84C6
#define GL_TEXTURE7                       0x84C7

#define GL_DEBUG_SEVERITY_HIGH            0x9146
#define GL_DEBUG_SEVERITY_MEDIUM          0x9147
#define GL_DEBUG_SEVERITY_LOW             0x9148
#define GL_DEBUG_TYPE_MARKER              0x8268
#define GL_DEBUG_TYPE_PUSH_GROUP          0x8269
#define GL_DEBUG_TYPE_POP_GROUP           0x826A
#define GL_DEBUG_SEVERITY_NOTIFICATION    0x826B

#define GL_DEBUG_OUTPUT_SYNCHRONOUS       0x8242
#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_STREAM_DRAW                    0x88E0
#define GL_STREAM_READ                    0x88E1
#define GL_STREAM_COPY                    0x88E2
#define GL_STATIC_DRAW                    0x88E4
#define GL_STATIC_READ                    0x88E5
#define GL_STATIC_COPY                    0x88E6
#define GL_DYNAMIC_DRAW                   0x88E8
#define GL_DYNAMIC_READ                   0x88E9
#define GL_DYNAMIC_COPY                   0x88EA

#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_TEXTURE_MIN_LOD                0x813A
#define GL_TEXTURE_MAX_LOD                0x813B
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_TEXTURE_WRAP_R                 0x8072

#define GL_FRAMEBUFFER_SRGB               0x8DB9
#define GL_SRGB8_ALPHA8                   0x8C43

#define GL_SHADING_LANGUAGE_VERSION       0x8B8C
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_VALIDATE_STATUS                0x8B83

#define GL_TEXTURE_2D_ARRAY               0x8C1A

#define GL_FRAMEBUFFER                    0x8D40
#define GL_READ_FRAMEBUFFER               0x8CA8
#define GL_DRAW_FRAMEBUFFER               0x8CA9
#define GL_COLOR_ATTACHMENT0              0x8CE0
#define GL_COLOR_ATTACHMENT1              0x8CE1
#define GL_COLOR_ATTACHMENT2              0x8CE2
#define GL_COLOR_ATTACHMENT3              0x8CE3
#define GL_COLOR_ATTACHMENT4              0x8CE4
#define GL_COLOR_ATTACHMENT5              0x8CE5
#define GL_COLOR_ATTACHMENT6              0x8CE6
#define GL_COLOR_ATTACHMENT7              0x8CE7
#define GL_COLOR_ATTACHMENT8              0x8CE8
#define GL_COLOR_ATTACHMENT9              0x8CE9
#define GL_COLOR_ATTACHMENT10             0x8CEA
#define GL_COLOR_ATTACHMENT11             0x8CEB
#define GL_COLOR_ATTACHMENT12             0x8CEC
#define GL_COLOR_ATTACHMENT13             0x8CED
#define GL_COLOR_ATTACHMENT14             0x8CEE
#define GL_COLOR_ATTACHMENT15             0x8CEF
#define GL_COLOR_ATTACHMENT16             0x8CF0
#define GL_COLOR_ATTACHMENT17             0x8CF1
#define GL_COLOR_ATTACHMENT18             0x8CF2
#define GL_COLOR_ATTACHMENT19             0x8CF3
#define GL_COLOR_ATTACHMENT20             0x8CF4
#define GL_COLOR_ATTACHMENT21             0x8CF5
#define GL_COLOR_ATTACHMENT22             0x8CF6
#define GL_COLOR_ATTACHMENT23             0x8CF7
#define GL_COLOR_ATTACHMENT24             0x8CF8
#define GL_COLOR_ATTACHMENT25             0x8CF9
#define GL_COLOR_ATTACHMENT26             0x8CFA
#define GL_COLOR_ATTACHMENT27             0x8CFB
#define GL_COLOR_ATTACHMENT28             0x8CFC
#define GL_COLOR_ATTACHMENT29             0x8CFD
#define GL_COLOR_ATTACHMENT30             0x8CFE
#define GL_COLOR_ATTACHMENT31             0x8CFF
#define GL_DEPTH_ATTACHMENT               0x8D00
#define GL_FRAMEBUFFER_COMPLETE           0x8CD5

#define GL_DEPTH_COMPONENT16              0x81A5
#define GL_DEPTH_COMPONENT24              0x81A6
#define GL_DEPTH_COMPONENT32              0x81A7
#define GL_DEPTH_COMPONENT32F             0x8CAC

#define GL_RED_INTEGER                    0x8D94
#define GL_GREEN_INTEGER                  0x8D95
#define GL_BLUE_INTEGER                   0x8D96

#define GL_RGBA32F                        0x8814
#define GL_RGB32F                         0x8815
#define GL_RGBA16F                        0x881A
#define GL_RGB16F                         0x881B
#define GL_R8                             0x8229
#define GL_R16                            0x822A
#define GL_RG8                            0x822B
#define GL_RG16                           0x822C
#define GL_R16F                           0x822D
#define GL_R32F                           0x822E
#define GL_RG16F                          0x822F
#define GL_RG32F                          0x8230
#define GL_R8I                            0x8231
#define GL_R8UI                           0x8232
#define GL_R16I                           0x8233
#define GL_R16UI                          0x8234
#define GL_R32I                           0x8235
#define GL_R32UI                          0x8236
#define GL_RG8I                           0x8237
#define GL_RG8UI                          0x8238
#define GL_RG16I                          0x8239
#define GL_RG16UI                         0x823A
#define GL_RG32I                          0x823B
#define GL_RG32UI                         0x823C
#define GL_R11F_G11F_B10F                 0x8C3A


#define GL_MULTISAMPLE                    0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE            0x809F
#define GL_SAMPLE_COVERAGE                0x80A0
#define GL_SAMPLE_BUFFERS                 0x80A8
#define GL_SAMPLES                        0x80A9
#define GL_SAMPLE_COVERAGE_VALUE          0x80AA
#define GL_SAMPLE_COVERAGE_INVERT         0x80AB
#define GL_TEXTURE_2D_MULTISAMPLE         0x9100
#define GL_MAX_SAMPLES                    0x8D57
#define GL_MAX_COLOR_TEXTURE_SAMPLES      0x910E
#define GL_MAX_DEPTH_TEXTURE_SAMPLES      0x910F

global GLenum OpenGLAllColorAttachments[] = {
    GL_COLOR_ATTACHMENT0,
    GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3,
    GL_COLOR_ATTACHMENT4,
    GL_COLOR_ATTACHMENT5,
    GL_COLOR_ATTACHMENT6,
    GL_COLOR_ATTACHMENT7,
    GL_COLOR_ATTACHMENT8,
    GL_COLOR_ATTACHMENT9,
    GL_COLOR_ATTACHMENT10,
    GL_COLOR_ATTACHMENT11,
    GL_COLOR_ATTACHMENT12,
    GL_COLOR_ATTACHMENT13,
    GL_COLOR_ATTACHMENT14,
    GL_COLOR_ATTACHMENT15,
    GL_COLOR_ATTACHMENT16,
    GL_COLOR_ATTACHMENT17,
    GL_COLOR_ATTACHMENT18,
    GL_COLOR_ATTACHMENT19,
    GL_COLOR_ATTACHMENT20,
    GL_COLOR_ATTACHMENT21,
    GL_COLOR_ATTACHMENT22,
    GL_COLOR_ATTACHMENT23,
    GL_COLOR_ATTACHMENT24,
    GL_COLOR_ATTACHMENT25,
    GL_COLOR_ATTACHMENT26,
    GL_COLOR_ATTACHMENT27,
    GL_COLOR_ATTACHMENT28,
    GL_COLOR_ATTACHMENT29,
    GL_COLOR_ATTACHMENT30,
    GL_COLOR_ATTACHMENT31,
};

GL_DEBUG_CALLBACK(OpenGLDebugCallback)
{
    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        char *ErrorMessage = (char *)message;
#if 0
        OutputDebugStringA("OPENGL: ");
        OutputDebugStringA(ErrorMessage);
        OutputDebugStringA("\n");
#endif
        Assert(!"OpenGL Error encountered");
    }
}

internal opengl_info
OpenGLGetInfo(open_gl *OpenGL, b32 ModernContext)
{
    opengl_info Result = {};

    Result.ModernContext = ModernContext;
    Result.Vendor = (char *)glGetString(GL_VENDOR);
    Result.Renderer = (char *)glGetString(GL_RENDERER);
    Result.Version = (char *)glGetString(GL_VERSION);
    if(Result.ModernContext)
    {
        Result.ShadingLanguageVersion = (char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    }
    else
    {
        Result.ShadingLanguageVersion = "(none)";
    }

    if(OpenGL->glGetStringi)
    {
        GLint ExtensionCount = 0;
        glGetIntegerv(GL_NUM_EXTENSIONS, &ExtensionCount);
        for(GLint ExtensionIndex = 0;
            ExtensionIndex < ExtensionCount;
            ++ExtensionIndex)
        {
            char *ExtName = (char *)OpenGL->glGetStringi(GL_EXTENSIONS, ExtensionIndex);

            if(0) {}
            else if(StringsAreEqual(ExtName, "GL_EXT_texture_sRGB")) {Result.OpenGL_EXT_texture_sRGB=true;}
            else if(StringsAreEqual(ExtName, "GL_EXT_framebuffer_sRGB")) {Result.OpenGL_EXT_framebuffer_sRGB=true;}
            else if(StringsAreEqual(ExtName, "GL_ARB_framebuffer_sRGB")) {Result.OpenGL_EXT_framebuffer_sRGB=true;}
            else if(StringsAreEqual(ExtName, "GL_ARB_framebuffer_object")) {Result.OpenGL_ARB_framebuffer_object=true;}
            // TODO(casey): Is there some kind of ARB string to look for that indicates GL_EXT_texture_sRGB?
        }
    }

    char *MajorAt = Result.Version;
    char *MinorAt = 0;
    for(char *At = Result.Version;
        *At;
        ++At)
    {
        if(At[0] == '.')
        {
            MinorAt = At + 1;
            break;
        }
    }

    s32 Major = 1;
    s32 Minor = 0;
    if(MinorAt)
    {
        Major = S32FromZ(MajorAt);
        Minor = S32FromZ(MinorAt);
    }

    if((Major > 2) || ((Major == 2) && (Minor >= 1)))
    {
        // NOTE(casey): We _believe_ we have sRGB textures in 2.1 and above automatically.
        Result.OpenGL_EXT_texture_sRGB = true;
    }

    if(Major >= 3)
    {
        // NOTE(casey): We _believe_ we have framebuffer objects in 3.0 and above automatically.
        Result.OpenGL_ARB_framebuffer_object=true;
    }


    return(Result);
}

internal void
OpenGLBindFramebuffer(open_gl *OpenGL, opengl_framebuffer *Framebuffer, u32 RenderWidth, u32 RenderHeight)
{
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, Framebuffer ? Framebuffer->FramebufferHandle : 0);
    glViewport(0, 0, RenderWidth, RenderHeight);
}

inline void
OpenGLLineVertices(v3 MinP, v3 MaxP)
{
    f32 Z = MaxP.z;

    glVertex3f(MinP.x, MinP.y, Z);
    glVertex3f(MaxP.x, MinP.y, Z);

    glVertex3f(MaxP.x, MinP.y, Z);
    glVertex3f(MaxP.x, MaxP.y, Z);

    glVertex3f(MaxP.x, MaxP.y, Z);
    glVertex3f(MinP.x, MaxP.y, Z);

    glVertex3f(MinP.x, MaxP.y, Z);
    glVertex3f(MinP.x, MinP.y, Z);
}

inline void
GLQuad(v3 P0, v2 T0, v4 C0,
       v3 P1, v2 T1, v4 C1,
       v3 P2, v2 T2, v4 C2,
       v3 P3, v2 T3, v4 C3)
{
    // NOTE(casey): Lower triangle
    glColor4fv(C0.E);
    glTexCoord2fv(T0.E);
    glVertex3fv(P0.E);

    glColor4fv(C1.E);
    glTexCoord2fv(T1.E);
    glVertex3fv(P1.E);

    glColor4fv(C2.E);
    glTexCoord2fv(T2.E);
    glVertex3fv(P2.E);

    // NOTE(casey): Upper triangle
    glColor4fv(C0.E);
    glTexCoord2fv(T0.E);
    glVertex3fv(P0.E);

    glColor4fv(C2.E);
    glTexCoord2fv(T2.E);
    glVertex3fv(P2.E);

    glColor4fv(C3.E);
    glTexCoord2fv(T3.E);
    glVertex3fv(P3.E);
}

inline void
OpenGLRectangle(v3 MinP, v3 MaxP, v4 PremulColor, v2 MinUV = V2(0, 0), v2 MaxUV = V2(1, 1))
{
    glBegin(GL_TRIANGLES);

    glColor4f(PremulColor.r, PremulColor.g, PremulColor.b, PremulColor.a);

    // NOTE(casey): Lower triangle
    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex3f(MinP.x, MinP.y, MinP.z);

    glTexCoord2f(MaxUV.x, MinUV.y);
    glVertex3f(MaxP.x, MinP.y, MinP.z);

    glTexCoord2f(MaxUV.x, MaxUV.y);
    glVertex3f(MaxP.x, MaxP.y, MinP.z);

    // NOTE(casey): Upper triangle
    glTexCoord2f(MinUV.x, MinUV.y);
    glVertex3f(MinP.x, MinP.y, MinP.z);

    glTexCoord2f(MaxUV.x, MaxUV.y);
    glVertex3f(MaxP.x, MaxP.y, MinP.z);

    glTexCoord2f(MinUV.x, MaxUV.y);
    glVertex3f(MinP.x, MaxP.y, MinP.z);

    glEnd();
}

inline void
OpenGLDisplayBitmap(open_gl *OpenGL, s32 Width, s32 Height, void *Memory, int Pitch,
                    rectangle2i DrawRegion, v4 ClearColor, GLuint BlitTexture)
{
    Assert(Pitch == (Width*4));
    OpenGLBindFramebuffer(OpenGL, 0, GetWidth(DrawRegion), GetHeight(DrawRegion));

    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glBindTexture(GL_TEXTURE_2D, BlitTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, Width, Height, 0,
                 GL_BGRA_EXT, GL_UNSIGNED_BYTE, Memory);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glEnable(GL_TEXTURE_2D);

    glClearColor(ClearColor.r, ClearColor.g, ClearColor.b, ClearColor.a);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //NotImplemented;

    // TODO(casey): This needs to be worked out specifically for doing the full-screen
    // draw!
    glMatrixMode(GL_PROJECTION);
    r32 a = SafeRatio1(2.0f, 1.0f);
    r32 b = SafeRatio1(2.0f*(r32)Width, (r32)Height);
    r32 Proj[] =
    {
        a,  0,  0,  0,
        0,  b,  0,  0,
        0,  0,  1,  0,
        0,  0,  0,  1,
    };
    glLoadMatrixf(Proj);

    v3 MinP = {0, 0, 0};
    v3 MaxP = {(r32)Width, (r32)Height, 0.0f};
    v4 Color = {1, 1, 1, 1};

    // TODO(casey): Decide how we want to handle aspect ratio - black bars or crop?

    OpenGLRectangle(MinP, MaxP, Color);

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_BLEND);
}

internal GLuint
GetSpecialTextureHandleFor(open_gl *OpenGL, renderer_texture Texture)
{
    u32 Index = TextureIndexFrom(Texture);
    Assert(Index < OpenGL->MaxSpecialTextureCount);
    GLuint Result = OpenGL->SpecialTextureHandles[Index];
    return(Result);
}

internal GLuint
OpenGLCreateProgram(open_gl *OpenGL, char *Defines, char *HeaderCode, char *VertexCode, char *FragmentCode,
                    opengl_program_common *Result)
{
    GLuint VertexShaderID = OpenGL->glCreateShader(GL_VERTEX_SHADER);
    GLchar *VertexShaderCode[] =
    {
        Defines,
        HeaderCode,
        VertexCode,
    };
    OpenGL->glShaderSource(VertexShaderID, ArrayCount(VertexShaderCode), VertexShaderCode, 0);
    OpenGL->glCompileShader(VertexShaderID);

    GLuint FragmentShaderID = OpenGL->glCreateShader(GL_FRAGMENT_SHADER);
    GLchar *FragmentShaderCode[] =
    {
        Defines,
        HeaderCode,
        FragmentCode,
    };
    OpenGL->glShaderSource(FragmentShaderID, ArrayCount(FragmentShaderCode), FragmentShaderCode, 0);
    OpenGL->glCompileShader(FragmentShaderID);

    GLuint ProgramID = OpenGL->glCreateProgram();
    OpenGL->glAttachShader(ProgramID, VertexShaderID);
    OpenGL->glAttachShader(ProgramID, FragmentShaderID);
    OpenGL->glLinkProgram(ProgramID);

    OpenGL->glValidateProgram(ProgramID);
    GLint Linked = false;
    OpenGL->glGetProgramiv(ProgramID, GL_LINK_STATUS, &Linked);
    if(!Linked)
    {
        GLsizei Ignored;
        char VertexErrors[4096];
        char FragmentErrors[4096];
        char ProgramErrors[4096];
        OpenGL->glGetShaderInfoLog(VertexShaderID, sizeof(VertexErrors), &Ignored, VertexErrors);
        OpenGL->glGetShaderInfoLog(FragmentShaderID, sizeof(FragmentErrors), &Ignored, FragmentErrors);
        OpenGL->glGetProgramInfoLog(ProgramID, sizeof(ProgramErrors), &Ignored, ProgramErrors);

        Assert(!"Shader validation failed");
    }

    OpenGL->glDeleteShader(VertexShaderID);
    OpenGL->glDeleteShader(FragmentShaderID);

    Result->ProgHandle = ProgramID;
    Result->VertPID = OpenGL->glGetAttribLocation(ProgramID, "VertP");
    Result->VertNID = OpenGL->glGetAttribLocation(ProgramID, "VertN");
    Result->VertUVID = OpenGL->glGetAttribLocation(ProgramID, "VertUV");
    Result->VertColorID = OpenGL->glGetAttribLocation(ProgramID, "VertColor");
    Result->VertTextureIndex = OpenGL->glGetAttribLocation(ProgramID, "VertTextureIndex");
    Result->VertEmission = OpenGL->glGetAttribLocation(ProgramID, "VertEmission");
    Result->VertTone = OpenGL->glGetAttribLocation(ProgramID, "VertTone");
    Result->SamplerCount = 0;

    return(ProgramID);
}

internal void
OpenGLLinkSamplers(open_gl *OpenGL,
                   opengl_program_common *Prog,
                   char *Sampler0 = 0,
                   char *Sampler1 = 0,
                   char *Sampler2 = 0,
                   char *Sampler3 = 0,
                   char *Sampler4 = 0,
                   char *Sampler5 = 0,
                   char *Sampler6 = 0,
                   char *Sampler7 = 0,
                   char *Sampler8 = 0,
                   char *Sampler9 = 0,
                   char *Sampler10 = 0,
                   char *Sampler11 = 0,
                   char *Sampler12 = 0,
                   char *Sampler13 = 0,
                   char *Sampler14 = 0,
                   char *Sampler15 = 0
                   )
{
    char *SamplerNames[] =
    {
        Sampler0,
        Sampler1,
        Sampler2,
        Sampler3,
        Sampler4,
        Sampler5,
        Sampler6,
        Sampler7,
        Sampler8,
        Sampler9,
        Sampler10,
        Sampler11,
        Sampler12,
        Sampler13,
        Sampler14,
        Sampler15,
    };

    for(u32 SamplerIndex = 0;
        SamplerIndex < ArrayCount(SamplerNames);
        ++SamplerIndex)
    {
        char *Name = SamplerNames[SamplerIndex];
        if(Name)
        {
            GLuint SamplerID = OpenGL->glGetUniformLocation(Prog->ProgHandle, Name);
            Assert(Prog->SamplerCount < ArrayCount(Prog->Samplers));
            Prog->Samplers[Prog->SamplerCount++] = SamplerID;
        }
    }
}

internal b32
IsValidArray(GLuint Index)
{
    b32 Result = (Index != -1);
    return(Result);
}

internal void
UseProgramBegin(open_gl *OpenGL, opengl_program_common *Prog)
{
    OpenGL->glUseProgram(Prog->ProgHandle);

    GLuint PArray = Prog->VertPID;
    GLuint NArray = Prog->VertNID;
    GLuint CArray = Prog->VertColorID;
    GLuint UVArray = Prog->VertUVID;
    GLuint TextureIndex = Prog->VertTextureIndex;
    GLuint Emission = Prog->VertEmission;
    GLuint Tone = Prog->VertTone;
    
    if(IsValidArray(PArray))
    {
        OpenGL->glEnableVertexAttribArray(PArray);
        OpenGL->glVertexAttribPointer(PArray, 4, GL_FLOAT, false, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, P));
    }

    if(IsValidArray(NArray))
    {
        OpenGL->glEnableVertexAttribArray(NArray);
        OpenGL->glVertexAttribPointer(NArray, 3, GL_FLOAT, false, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, N));
    }

    if(IsValidArray(CArray))
    {
        OpenGL->glEnableVertexAttribArray(CArray);
        OpenGL->glVertexAttribPointer(CArray, 4, GL_UNSIGNED_BYTE, true, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, Color));
    }

    if(IsValidArray(UVArray))
    {
        OpenGL->glEnableVertexAttribArray(UVArray);
        OpenGL->glVertexAttribPointer(UVArray, 2, GL_FLOAT, false, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, UV));
    }

    if(IsValidArray(TextureIndex))
    {
        OpenGL->glEnableVertexAttribArray(TextureIndex);
        OpenGL->glVertexAttribIPointer(TextureIndex, 1, GL_UNSIGNED_SHORT, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, TextureIndex));
    }

    if(IsValidArray(Emission))
    {
        OpenGL->glEnableVertexAttribArray(Emission);
        OpenGL->glVertexAttribPointer(Emission, 1, GL_UNSIGNED_BYTE, true, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, Emission));
    }
    
    if(IsValidArray(Tone))
    {
        OpenGL->glEnableVertexAttribArray(Tone);
        OpenGL->glVertexAttribPointer(Tone, 1, GL_UNSIGNED_BYTE, true, sizeof(textured_vertex), (void *)OffsetOf(textured_vertex, Tone));
    }
    
    for(u32 SamplerIndex = 0;
        SamplerIndex < Prog->SamplerCount;
        ++SamplerIndex)
    {
        OpenGL->glUniform1i(Prog->Samplers[SamplerIndex], SamplerIndex);
    }
}

internal void
UseProgramEnd(open_gl *OpenGL, opengl_program_common *Prog)
{
    OpenGL->glUseProgram(0);

    GLuint PArray = Prog->VertPID;
    GLuint NArray = Prog->VertNID;
    GLuint CArray = Prog->VertColorID;
    GLuint UVArray = Prog->VertUVID;
    GLuint TextureIndex = Prog->VertTextureIndex;
    GLuint Emission = Prog->VertEmission;
    GLuint Tone = Prog->VertTone;
    
    if(IsValidArray(PArray))
    {
        OpenGL->glDisableVertexAttribArray(PArray);
    }

    if(IsValidArray(NArray))
    {
        OpenGL->glDisableVertexAttribArray(NArray);
    }

    if(IsValidArray(CArray))
    {
        OpenGL->glDisableVertexAttribArray(CArray);
    }

    if(IsValidArray(UVArray))
    {
        OpenGL->glDisableVertexAttribArray(UVArray);
    }

    if(IsValidArray(TextureIndex))
    {
        OpenGL->glDisableVertexAttribArray(TextureIndex);
    }

    if(IsValidArray(Emission))
    {
        OpenGL->glDisableVertexAttribArray(Emission);
    }
    
    if(IsValidArray(Tone))
    {
        OpenGL->glDisableVertexAttribArray(Tone);
    }
}

internal void
UseProgramBegin(open_gl *OpenGL, zbias_program *Prog, game_render_commands *Commands, render_setup *Setup, f32 AlphaThreshold)
{
    UseProgramBegin(OpenGL, &Prog->Common);

    OpenGL->glUniformMatrix4fv(Prog->TransformID, 1, GL_TRUE, Setup->Proj.E[0]);
    OpenGL->glUniform3fv(Prog->CameraP, 1, Setup->CameraP.E);
    OpenGL->glUniform3fv(Prog->FogDirection, 1, Setup->FogDirection.E);
    OpenGL->glUniform3fv(Prog->FogColor, 1, Setup->FogColor.E);
    OpenGL->glUniform1f(Prog->FogStartDistance, Setup->FogStartDistance);
    OpenGL->glUniform1f(Prog->FogEndDistance, Setup->FogEndDistance);
    OpenGL->glUniform1f(Prog->ClipAlphaStartDistance, Setup->ClipAlphaStartDistance);
    OpenGL->glUniform1f(Prog->ClipAlphaEndDistance, Setup->ClipAlphaEndDistance);
    OpenGL->glUniform1f(Prog->AlphaThreshold, AlphaThreshold);
    OpenGL->glUniform3fv(Prog->VoxelMinCorner, 1, Commands->LightingVoxelMinCorner.E);
    OpenGL->glUniform3fv(Prog->VoxelInvTotalDim, 1, Commands->LightingVoxelInvTotalDim.E);
}

internal void
UseProgramBegin(open_gl *OpenGL, resolve_multisample_program *Prog)
{
    UseProgramBegin(OpenGL, &Prog->Common);
}

// TODO(casey): Parameterize the type of depth buffer to use in the render parameters?
// It seems like we don't actually need floating point, 24-bit works fine.
#define OPENGL_DEPTH_COMPONENT_TYPE GL_DEPTH_COMPONENT32F
#define OPENGL_DEPTH_COMPONENT_BYTES_PER_SAMPLE 4
internal GLuint
FramebufferTexImage(open_gl *OpenGL, GLuint Slot, u32 Width, u32 Height, GLint FilterType, GLuint Format)
{
    GLuint Result = 0;

    glGenTextures(1, &Result);
    glBindTexture(Slot, Result);

    if(Slot == GL_TEXTURE_2D_MULTISAMPLE)
    {
        OpenGL->glTexImage2DMultisample(Slot, OpenGL->MaxMultiSampleCount, Format,
                                        Width, Height,
                                        GL_FALSE);
    }
    else
    {
        glTexImage2D(Slot, 0,
                     Format,
                     Width, Height, 0,
                     (Format == OPENGL_DEPTH_COMPONENT_TYPE) ? GL_DEPTH_COMPONENT : GL_BGRA_EXT,
                     GL_UNSIGNED_BYTE, 0);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterType);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterType);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return(Result);
}

internal opengl_framebuffer
CreateFramebuffer(open_gl *OpenGL, u32 Width, u32 Height, u32 Flags, u32 ColorBufferCount)
{
    Assert(glGetError() == GL_NO_ERROR);
    Assert((Width > 0) && (Height > 0));

    opengl_framebuffer Result = {};

    b32x Multisampled = Flags & OpenGLFramebuffer_Multisampled;
    b32x Filtered = Flags & OpenGLFramebuffer_Filtered;
    b32x HasDepth = Flags & OpenGLFramebuffer_Depth;
    b32x IsFloat = Flags & OpenGLFramebuffer_Float;

    OpenGL->glGenFramebuffers(1, &Result.FramebufferHandle);
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, Result.FramebufferHandle);

    GLuint Slot = Multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
    GLint FilterType = Filtered ? GL_LINEAR : GL_NEAREST;

    u32 SampleCount = Multisampled ? OpenGL->MaxMultiSampleCount : 1;

    Assert(ColorBufferCount <= ArrayCount(OpenGLAllColorAttachments));
    Assert(ColorBufferCount <= ArrayCount(Result.ColorHandle));
    for(u32 ColorIndex = 0;
        ColorIndex < ColorBufferCount;
        ++ColorIndex)
    {
        // TODO(casey): Pass the multisample count instead of the slot?
        Result.GPUMemoryUsed += Width * Height * (IsFloat ? 4*4 : 4) * SampleCount;
        Result.ColorHandle[ColorIndex] = FramebufferTexImage(OpenGL, Slot, Width, Height, FilterType,
                                                             (ColorIndex == 0) ? OpenGL->DefaultFramebufferTextureFormat : GL_RGBA8);
        OpenGL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + ColorIndex, Slot,
                                       Result.ColorHandle[ColorIndex], 0);
    }
    OpenGL->glDrawBuffers(ColorBufferCount, OpenGLAllColorAttachments);

    if(HasDepth)
    {
        Result.GPUMemoryUsed += Width * Height * OPENGL_DEPTH_COMPONENT_BYTES_PER_SAMPLE * SampleCount;
        Result.DepthHandle = FramebufferTexImage(OpenGL, Slot, Width, Height, FilterType,
                                                 OPENGL_DEPTH_COMPONENT_TYPE);
        OpenGL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, Slot, Result.DepthHandle, 0);
    }

    GLenum FrameBufferStatus = OpenGL->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    Assert(FrameBufferStatus == GL_FRAMEBUFFER_COMPLETE);

    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(Slot, 0);

    OpenGL->Header.TotalFramebufferMemory += Result.GPUMemoryUsed;

    return(Result);
}

global char *GlobalShaderHeaderCode = R"FOO(
#define m4x4 mat4x4
#define f32 float
#define s32 int
#define u32 int unsigned
#define v4 vec4
#define v3 vec3
#define v3s ivec3
#define v2 vec2
#define v2s ivec2
#define V4 vec4
#define V3 vec3
#define V3S ivec3
#define V2 vec2
#define V2S ivec2
#define Lerp(a, t, b) mix(a, b, t)
#define Clamp01(t) clamp(t, 0, 1)
#define Clamp0Inf(t) max(t, 0)
#define Clamp(min, t, max) clamp(t, min, max)
#define Inner(a, b) dot(a, b)
#define Length(a) length(a)
#define LengthSq(a) dot(a, a)
#define SquareRoot(a) sqrt(a)
#define Square(a) ((a)*(a))
#define Sign(a) sign(a)
#define MaxLightIntensity 10
#define NOZ(a) normalize(a)
#define AbsoluteValue(a) abs(a)

f32 Clamp01MapToRange(f32 Min, f32 t, f32 Max)
{
f32 Range = Max - Min;
f32 Result = Clamp01((t - Min) / Range);

return(Result);
}

v2 PackNormal2(v2 N)
{
v2 Result;
Result.x = 0.5f + 0.5f*N.x;
Result.y = 0.5f + 0.5f*N.y;
return(Result);
}

v2 UnpackNormal2(v2 N)
{
v2 Result;
Result.x = -1.0f + 2.0f*N.x;
Result.y = -1.0f + 2.0f*N.y;
return(Result);
}

v3 ExtendNormalZ(v2 N)
{
v3 Result = V3(N, sqrt(1 - N.x*N.x - N.y*N.y));
return(Result);
}

v3 UnpackNormal3(v2 N)
{
v3 Result = ExtendNormalZ(UnpackNormal2(N));
return(Result);
}

// NOTE(casey): This is from the ACES standard exposure curve, we did not
// invent it ourselves!
v3 ACESFilm(v3 x)
{
    f32 a = 2.51;
    f32 b = 0.03;
    f32 c = 2.43;
    f32 d = 0.59;
    f32 e = 0.14;
    return Clamp01((x*(a*x+b))/(x*(c*x+d)+e));
}

f32 Mestre(f32 x)
{
    return x*x*(3.0 - 2.0*x);
}

)FOO";

internal void
CompileZBiasProgram(open_gl *OpenGL, zbias_program *Result, b32x DepthPeel, v3s VoxelDim)
{
    char Defines[1024];
    FormatString(sizeof(Defines), Defines,
                 "#version 330\n"
                 "#define ShaderSimTexWriteSRGB %d\n"
                 "#define ShaderSimTexReadSRGB %d\n"
                 "#define DepthPeel %d\n"
                 "#define MAX_LIGHT_INTENSITY %f\n"
                 "#define LIGHT_LOOKUP_VOXEL_DIM V3(%u,%u,%u)\n"
                 "#define DIFFUSE_TILE_DIM_X %u\n"
                 "#define DIFFUSE_TILE_DIM_Y %u\n"
                 "#define SPECULAR_TILE_DIM_X %u\n"
                 "#define SPECULAR_TILE_DIM_Y %u\n"
                 ,
                 OpenGL->ShaderSimTexWriteSRGB,
                 OpenGL->ShaderSimTexReadSRGB,
                 DepthPeel,
                 MAX_LIGHT_INTENSITY,
                 VoxelDim.x, VoxelDim.y, VoxelDim.z,
                 OpenGL->DiffuseLightAtlas.TileDim.x,
                 OpenGL->DiffuseLightAtlas.TileDim.y,
                 OpenGL->SpecularLightAtlas.TileDim.x,
                 OpenGL->SpecularLightAtlas.TileDim.y);

    char *VertexCode = R"FOO(
    // Vertex code
    uniform m4x4 Transform;

    uniform v3 CameraP;
    uniform v3 FogDirection;

    in v4 VertP;
    in v3 VertN;
    in v2 VertUV;
    in v4 VertColor;
    in int VertTextureIndex;
    in f32 VertEmission;
    in f32 VertTone;

    smooth out v2 FragUV;
    smooth out v4 FragColor;
    smooth out f32 FogDistance;
    smooth out v3 WorldP;
    smooth out v3 WorldN;
    smooth out f32 FragEmission;
    smooth out f32 FragTone;

        flat out int FragTextureIndex;

    void main(void)
    {
    v4 InVertex = V4(VertP.xyz, 1.0);
    f32 ZBias = VertP.w;

    v4 ZVertex = InVertex;
    ZVertex.z += ZBias;

    v4 ZMinTransform = Transform*InVertex;
    v4 ZMaxTransform = Transform*ZVertex;

    f32 ModifiedZ = (ZMinTransform.w / ZMaxTransform.w)*ZMaxTransform.z;

    gl_Position = vec4(ZMinTransform.x, ZMinTransform.y, ModifiedZ, ZMinTransform.w);

    FragUV = VertUV.xy;
    FragColor = VertColor;

    FragEmission = VertEmission;
    FragTone = VertTone;;

    FogDistance = Inner(ZVertex.xyz - CameraP, FogDirection);
    WorldP = ZVertex.xyz;
    WorldN = VertN;

        FragTextureIndex = VertTextureIndex;
    }
    )FOO";

    /* TODO(casey): Put this into the fragment shader
            v3 LightC = Hadamard(Source->RefC, Source->FrontEmitC);

            ToLight *= 1.0f / LightDistance;
            v3 RefN = Dest->N;
            f32 DiffuseC = 1.0f;
            f32 SpecularC = 1.0f - DiffuseC; // TODO(casey): How should this really be encoded?
            f32 SpecularPower = 1.0f + (15.0);
            f32 DistanceFalloff = 1.0f / (1.0f + Square(LightDistance));
            f32 DiffuseInner = Clamp01(Inner(ToLight, RefN));
            f32 DiffuseContrib = DistanceFalloff*DiffuseC*DiffuseInner;
            v3 DiffuseContrib3 = V3(DiffuseContrib, DiffuseContrib, DiffuseContrib);
            v3 DiffuseLight = Hadamard(DiffuseContrib3, LightC);

            v3 RefVec = -ToCamera + 2*Inner(RefN, ToCamera)*RefN;
            f32 SpecInner = Clamp01(Inner(ToLight, RefVec));
            //            SpecInner = pow(SpecInner, SpecularPower);
            f32 SpecularContrib = SpecularC*SpecInner;
            v3 SpecularContrib3 = V3(SpecularContrib, SpecularContrib, SpecularContrib);
            v3 SpecularLight = Hadamard(SpecularContrib3, LightC);

            v3 TotalLight = DiffuseLight + SpecularLight;
            AccumC += TotalLight;
        }
        */

    char *FragmentCode = R"FOO(
    // Fragment code

    //uniform v4 FogColor;
    uniform sampler2DArray TextureSampler;
    #if DepthPeel
    uniform sampler2D DepthSampler;
    #endif
    uniform v3 FogColor;
    uniform f32 AlphaThreshold;
    uniform f32 FogStartDistance;
    uniform f32 FogEndDistance;
    uniform f32 ClipAlphaStartDistance;
    uniform f32 ClipAlphaEndDistance;
    uniform v3 CameraP;

    uniform v3 VoxelMinCorner;
    uniform v3 VoxelInvTotalDim;

    uniform sampler2D DiffuseSampler;
    uniform sampler2D SpecularSampler;

    smooth in vec2 FragUV;
    smooth in vec4 FragColor;
    smooth in f32 FogDistance;
    smooth in vec3 WorldP;
    smooth in vec3 WorldN;
    smooth in f32 FragEmission;
    smooth in f32 FragTone;

    flat in int FragTextureIndex;

    out v4 BlendUnitColor;

    v2 SignOf(v2 V)
    {
        v2 Result;

        Result.x = (V.x < 0) ? -1.0 : 1.0;
        Result.y = (V.y < 0) ? -1.0 : 1.0;

        return(Result);
    }

    v3 SampleLighting(v2 DiffOct, v2 SpecOct, v3 VoxI, v3 dVoxI)
    {
        f32 VoxXYStride = (LIGHT_LOOKUP_VOXEL_DIM.x*LIGHT_LOOKUP_VOXEL_DIM.y);
        f32 VoxZStride = LIGHT_LOOKUP_VOXEL_DIM.z;

        v2 OctExteriorUVDim;
        OctExteriorUVDim.x = (1.0 / VoxXYStride);
        OctExteriorUVDim.y = (1.0 / VoxZStride);

        v2 OctMapInteriorUVDimDiffuse;
        OctMapInteriorUVDimDiffuse.x = ((DIFFUSE_TILE_DIM_X - 2.0) / (VoxXYStride * DIFFUSE_TILE_DIM_X));
        OctMapInteriorUVDimDiffuse.y = ((DIFFUSE_TILE_DIM_Y - 2.0) / (VoxZStride * DIFFUSE_TILE_DIM_Y));

        v2 OctMapInteriorUVDimSpecular;
        OctMapInteriorUVDimSpecular.x = ((SPECULAR_TILE_DIM_X - 2.0) / (VoxXYStride * SPECULAR_TILE_DIM_X));
        OctMapInteriorUVDimSpecular.y = ((SPECULAR_TILE_DIM_Y - 2.0) / (VoxZStride * SPECULAR_TILE_DIM_Y));

        v3 I = Clamp(V3(0,0,0), VoxI + dVoxI, LIGHT_LOOKUP_VOXEL_DIM);
        v2 OctMapCenterUV;
        OctMapCenterUV.x = (LIGHT_LOOKUP_VOXEL_DIM.x*I.y + I.x)*OctExteriorUVDim.x;
        OctMapCenterUV.y = I.z*OctExteriorUVDim.y;
        OctMapCenterUV += 0.5*OctExteriorUVDim;

        v2 SampleDiffuseUV = OctMapCenterUV + 0.5*OctMapInteriorUVDimDiffuse*DiffOct;
        v2 SampleSpecularUV = OctMapCenterUV + 0.5*OctMapInteriorUVDimSpecular*SpecOct;

        v3 Diff = texture(DiffuseSampler, SampleDiffuseUV).rgb;
        v3 Spec = texture(SpecularSampler, SampleSpecularUV).rgb;

        f32 Material = 0.0;
        v3 Result = Lerp(Diff, Material, Spec);

        return Result;
    }

    v2 OctFrom(v3 R)
    {
        f32 OneNorm = AbsoluteValue(R.x) + AbsoluteValue(R.y) + AbsoluteValue(R.z);
        v2 Oct = (1.0 / OneNorm)*R.xy;
        if(R.z < 0)
        {
             Oct.xy = SignOf(R.xy) * (1.0 - AbsoluteValue(Oct.yx));
        }

        return Oct;
    }

    v3 SumLight()
    {
        v3 I = WorldP - CameraP;
        v3 Reflected = NOZ(I - 2*Inner(I, WorldN)*WorldN);

        v3 VoxRelP = WorldP - VoxelMinCorner;
        v3 VoxUVW = VoxelInvTotalDim*VoxRelP;
        v3 VoxR = (LIGHT_LOOKUP_VOXEL_DIM-V3(1, 1, 1))*VoxUVW; // TODO(casey): The -1 is suspicious here - did we do something unaligned in the original?
        v3 VoxI = floor(VoxR);
        VoxR -= VoxI;

        v2 DiffOct = OctFrom(WorldN);
        v2 SpecOct = OctFrom(Reflected);

        v3 S000 = SampleLighting(DiffOct, SpecOct, VoxI, V3(0, 0, 0));
        v3 S100 = SampleLighting(DiffOct, SpecOct, VoxI, V3(1, 0, 0));
        v3 S010 = SampleLighting(DiffOct, SpecOct, VoxI, V3(0, 1, 0));
        v3 S110 = SampleLighting(DiffOct, SpecOct, VoxI, V3(1, 1, 0));
        v3 S001 = SampleLighting(DiffOct, SpecOct, VoxI, V3(0, 0, 1));
        v3 S101 = SampleLighting(DiffOct, SpecOct, VoxI, V3(1, 0, 1));
        v3 S011 = SampleLighting(DiffOct, SpecOct, VoxI, V3(0, 1, 1));
        v3 S111 = SampleLighting(DiffOct, SpecOct, VoxI, V3(1, 1, 1));

/*
        VoxR.x = Mestre(VoxR.x);
        VoxR.y = Mestre(VoxR.y);
        VoxR.z = Mestre(VoxR.z);
*/

        v3 S00 = Lerp(S000, VoxR.z, S001);
        v3 S10 = Lerp(S100, VoxR.z, S101);
        v3 S01 = Lerp(S010, VoxR.z, S011);
        v3 S11 = Lerp(S110, VoxR.z, S111);

        v3 S0 = Lerp(S00, VoxR.y, S01);
        v3 S1 = Lerp(S10, VoxR.y, S11);

        v3 S = Lerp(S0, VoxR.x, S1);

        v3 Result = S;

        return Result;
    }

    void main(void)
    {
        #if DepthPeel
        f32 ClipDepth = texelFetch(DepthSampler, ivec2(gl_FragCoord.xy), 0).r;
        if(gl_FragCoord.z <= ClipDepth)
        {
            discard;
        }
        #endif

        vec3 ArrayUV = vec3(FragUV.x, FragUV.y, f32(FragTextureIndex));
        vec4 TexSample = texture(TextureSampler, ArrayUV);
        #if ShaderSimTexReadSRGB
            TexSample.rgb *= TexSample.rgb;
        #endif

        f32 FogAmount = Clamp01MapToRange(FogStartDistance, FogDistance, FogEndDistance);
        f32 AlphaAmount = Clamp01MapToRange(ClipAlphaStartDistance, FogDistance, ClipAlphaEndDistance);
        v4 ModColor = AlphaAmount*FragColor*TexSample;
        if(ModColor.a > AlphaThreshold)
        {
            v3 SurfaceReflect = ModColor.rgb;

            v3 L = Lerp(SumLight(), FragEmission, V3(1, 1, 1));
            SurfaceReflect *= L;
            SurfaceReflect = Lerp(SurfaceReflect, FogAmount, FogColor.rgb*ModColor.a);

            v3 MappedColor = Clamp0Inf(100.0*SurfaceReflect);
            MappedColor = ACESFilm(MappedColor);

            v3 FinalColor = Lerp(SurfaceReflect, FragTone, MappedColor);

            #if ShaderSimTexWriteSRGB
            FinalColor = sqrt(FinalColor);
            #endif

            BlendUnitColor.rgb = FinalColor;
            BlendUnitColor.a = ModColor.a;
        }
        else
        {
            discard;
        }
    }
    )FOO";

    GLuint Prog = OpenGLCreateProgram(OpenGL, Defines, GlobalShaderHeaderCode, VertexCode, FragmentCode, &Result->Common);
    OpenGLLinkSamplers(OpenGL, &Result->Common,
                       "TextureSampler",
                       "DepthSampler",
                       "DiffuseSampler",
                       "SpecularSampler");

    Result->TransformID = OpenGL->glGetUniformLocation(Prog, "Transform");
    Result->CameraP = OpenGL->glGetUniformLocation(Prog, "CameraP");
    Result->FogDirection = OpenGL->glGetUniformLocation(Prog, "FogDirection");
    Result->FogColor = OpenGL->glGetUniformLocation(Prog, "FogColor");
    Result->FogStartDistance = OpenGL->glGetUniformLocation(Prog, "FogStartDistance");
    Result->FogEndDistance = OpenGL->glGetUniformLocation(Prog, "FogEndDistance");
    Result->ClipAlphaStartDistance = OpenGL->glGetUniformLocation(Prog, "ClipAlphaStartDistance");
    Result->ClipAlphaEndDistance = OpenGL->glGetUniformLocation(Prog, "ClipAlphaEndDistance");
    Result->AlphaThreshold = OpenGL->glGetUniformLocation(Prog, "AlphaThreshold");

    Result->VoxelMinCorner = OpenGL->glGetUniformLocation(Prog, "VoxelMinCorner");
    Result->VoxelInvTotalDim = OpenGL->glGetUniformLocation(Prog, "VoxelInvTotalDim");
}

internal void
CompilePeelComposite(open_gl *OpenGL, opengl_program_common *Result)
{
    char Defines[1024];
    FormatString(sizeof(Defines), Defines,
                 "#version 330\n"
                 "#define ShaderSimTexWriteSRGB %d\n"
                 "#define ShaderSimTexReadSRGB %d\n"
                 "#define DepthPeel %d\n",
                 OpenGL->ShaderSimTexWriteSRGB,
                 OpenGL->ShaderSimTexReadSRGB,
                 false);

    char *VertexCode = R"FOO(
    // Vertex code
    in v4 VertP;
    in v4 VertColor;
    in v2 VertUV;
    smooth out v2 FragUV;
    smooth out v4 FragColor;
    void main(void)
    {
    gl_Position = VertP;
    FragUV = VertUV;
    FragColor = VertColor;
    }
    )FOO";

    char *FragmentCode = R"FOO(
    // Fragment code
    uniform sampler2D Peel0Sampler;
    uniform sampler2D Peel1Sampler;
    uniform sampler2D Peel2Sampler;
    uniform sampler2D Peel3Sampler;

    smooth in vec2 FragUV;
    smooth in vec4 FragColor;

    out vec4 BlendUnitColor;

    void main(void)
    {
    vec4 Peel0 = texture(Peel0Sampler, FragUV);
    vec4 Peel1 = texture(Peel1Sampler, FragUV);
    vec4 Peel2 = texture(Peel2Sampler, FragUV);
    vec4 Peel3 = texture(Peel3Sampler, FragUV);

    #if ShaderSimTexReadSRGB
    Peel0.rgb *= Peel0.rgb;
    Peel1.rgb *= Peel1.rgb;
    Peel2.rgb *= Peel2.rgb;
    Peel3.rgb *= Peel3.rgb;
    #endif

    #if 0
    Peel3.rgb *= (1.0f / Peel3.a);
    #endif

    #if 0
    Peel0.rgb = Peel0.a*V3(0, 0, 1);
    Peel1.rgb = Peel1.a*V3(0, 1, 0);
    Peel2.rgb = Peel2.a*V3(1, 0, 0);
    Peel3.rgb = V3(0, 0, 0);
    #endif

    BlendUnitColor.rgb = Peel3.rgb;
    BlendUnitColor.rgb = Peel2.rgb + (1 - Peel2.a)*BlendUnitColor.rgb;
    BlendUnitColor.rgb = Peel1.rgb + (1 - Peel1.a)*BlendUnitColor.rgb;
    BlendUnitColor.rgb = Peel0.rgb + (1 - Peel0.a)*BlendUnitColor.rgb;

    #if ShaderSimTexWriteSRGB
    BlendUnitColor.rgb = sqrt(BlendUnitColor.rgb);
    #endif
    }
    )FOO";

    GLuint Prog = OpenGLCreateProgram(OpenGL, Defines, GlobalShaderHeaderCode, VertexCode, FragmentCode, Result);
    OpenGLLinkSamplers(OpenGL, Result, "Peel0Sampler", "Peel1Sampler", "Peel2Sampler", "Peel3Sampler");
}

internal void
CompileResolveMultisample(open_gl *OpenGL, resolve_multisample_program *Result)
{
    char Defines[1024];
    FormatString(sizeof(Defines), Defines,
                 "#version 330\n"
                 "#define ShaderSimTexWriteSRGB %d\n"
                 "#define ShaderSimTexReadSRGB %d\n"
                 "#define DepthPeel %d\n"
                 "#define MultisamplingDebug %d\n",
                 OpenGL->ShaderSimTexWriteSRGB,
                 OpenGL->ShaderSimTexReadSRGB,
                 false,
                 OpenGL->CurrentSettings.MultisamplingDebug);

    char *VertexCode = R"FOO(
    // Vertex code
    in v4 VertP;
    void main(void)
    {
    gl_Position = VertP;
    }
    )FOO";

    char FragmentCode[16000];
    FormatString(sizeof(FragmentCode), FragmentCode,
                 R"FOO(
                 /* Fragment code
                 TODO(casey): Depth is non-linear - can we do something here that is based on ratio? */

                 #define DepthThreshold 0.001f
                 #define SampleCount %u
                 #define InvSampleCount %f

                 uniform sampler2DMS DepthSampler;
                 uniform sampler2DMS ColorSampler;
                 uniform sampler2D MaskSampler;

                 out v4 BlendUnitColor;
                 void main(void)
                 {

                 #if !MultisamplingDebug

                 f32 Mask = texelFetch(MaskSampler, ivec2(gl_FragCoord.xy), 0).a;
                 if(Mask < 1.0)
                 {
                 f32 DepthMax = 0.0f;
                 f32 DepthMin = 1.0f;
                 v4 CombinedColor = V4(0, 0, 0, 0);
                 for(s32 SampleIndex = 0;
                 SampleIndex < SampleCount;
                 ++SampleIndex)
                 {
                 f32 Depth = texelFetch(DepthSampler, ivec2(gl_FragCoord.xy), SampleIndex).r;
                 DepthMin = min(DepthMin, Depth);
                 DepthMax = max(DepthMax, Depth);

                 v4 Color = texelFetch(ColorSampler, ivec2(gl_FragCoord.xy), SampleIndex);
                 #if ShaderSimTexReadSRGB
                 Color.rgb *= Color.rgb;
                 #endif
                 CombinedColor += Color;
                 }

                 v4 SurfaceReflect = InvSampleCount*CombinedColor;
                 #if ShaderSimTexWriteSRGB
                 SurfaceReflect.rgb = sqrt(SurfaceReflect.rgb);
                 #endif

                 gl_FragDepth = 0.5f*(DepthMin+DepthMax);
                 BlendUnitColor = SurfaceReflect;
                 }
                 else
                 {
                 /*gl_FragDepth = texelFetch(DepthSampler, ivec2(gl_FragCoord.xy), 0).r;
                 BlendUnitColor = texelFetch(ColorSampler, ivec2(gl_FragCoord.xy), 0);*/
                 BlendUnitColor = V4(0, 0, 0, 1);
                 gl_FragDepth = 1.0;
                 }

                 #else

                 s32 UniqueCount = 1;
                 for(s32 IndexA = 1;
                 IndexA < SampleCount;
                 ++IndexA)
                 {
                 s32 Unique = 1;
                 f32 DepthA = texelFetch(DepthSampler, ivec2(gl_FragCoord.xy), IndexA).r;
                 for(s32 IndexB = 0;
                 IndexB < IndexA;
                 ++IndexB)
                 {
                 f32 DepthB = texelFetch(DepthSampler, ivec2(gl_FragCoord.xy), IndexB).r;
                 if((DepthA == 1.0) ||
                 (DepthB == 1.0) ||
                 (DepthA == DepthB))
                 {
                 Unique = 0;
                 break;
                 }
                 }
                 if(Unique == 1)
                 {
                 ++UniqueCount;
                 }
                 }

                 BlendUnitColor.a = 1;
                 if(UniqueCount == 1)
                 {
                 BlendUnitColor.rgb = V3(0, 0, 0);
                 }
                 if(UniqueCount == 2)
                 {
                 BlendUnitColor.rgb = V3(0, 1, 0);
                 }
                 if(UniqueCount == 3)
                 {
                 BlendUnitColor.rgb = V3(1, 1, 0);
                 }
                 if(UniqueCount >= 4)
                 {
                 BlendUnitColor.rgb = V3(1, 0, 0);
                 }
                 #endif

                 if(BlendUnitColor.a == 1.0)
                 {
                 gl_FragDepth = 1.0f;
                 }

                 }
                 )FOO",
                 OpenGL->MaxMultiSampleCount,
                 1.0f / f32(OpenGL->MaxMultiSampleCount));

    GLuint Prog = OpenGLCreateProgram(OpenGL, Defines, GlobalShaderHeaderCode, VertexCode, FragmentCode, &Result->Common);
    OpenGLLinkSamplers(OpenGL, &Result->Common, "DepthSampler", "ColorSampler", "MaskSampler");
}

internal void
CompileFinalStretch(open_gl *OpenGL, opengl_program_common *Result)
{
    char Defines[1024];
    FormatString(sizeof(Defines), Defines,
                 "#version 330\n"
                 "#define ShaderSimTexWriteSRGB %d\n"
                 "#define ShaderSimTexReadSRGB %d\n",
                 OpenGL->ShaderSimTexWriteSRGB,
                 OpenGL->ShaderSimTexReadSRGB);

    char *VertexCode = R"FOO(
    // Vertex code
    in v4 VertP;
    in v2 VertUV;

    smooth out v2 FragUV;

    void main(void)
    {
    gl_Position = VertP;
    FragUV = VertUV;
    }
    )FOO";

    char *FragmentCode = R"FOO(
    // Fragment code
    uniform sampler2D Image;

    smooth in vec2 FragUV;

    out vec4 BlendUnitColor;

    void main(void)
    {
    BlendUnitColor = texture(Image, FragUV);
    }
    )FOO";

    GLuint Prog = OpenGLCreateProgram(OpenGL, Defines, GlobalShaderHeaderCode, VertexCode, FragmentCode, Result);
    OpenGLLinkSamplers(OpenGL, Result, "Image");
}

#define ALLOW_GPU_SRGB 0
internal void
OpenGLInit(open_gl *OpenGL, opengl_info Info, b32 FramebufferSupportsSRGB)
{
    OpenGL->CurrentSettings.DepthPeelCountHint = 4;
    OpenGL->CurrentSettings.MultisamplingHint = true;
    OpenGL->CurrentSettings.PixelationHint = false;
    OpenGL->CurrentSettings.MultisamplingDebug = false;
#if HANDMADE_INTERNAL
    OpenGL->CurrentSettings.RequestVSync = false;
#else
    OpenGL->CurrentSettings.RequestVSync = true;
#endif

    OpenGL->ShaderSimTexReadSRGB = true;
    OpenGL->ShaderSimTexWriteSRGB = true;

    glGenTextures(1, &OpenGL->ReservedBlitTexture);

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &OpenGL->MaxColorAttachments);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &OpenGL->MaxSamplersPerShader);

    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &OpenGL->MaxMultiSampleCount);
    if(OpenGL->MaxMultiSampleCount > 16)
    {
        OpenGL->MaxMultiSampleCount = 16;
    }

    OpenGL->DefaultSpriteTextureFormat = GL_RGBA8;
    OpenGL->DefaultFramebufferTextureFormat = GL_RGBA8;

#if ALLOW_GPU_SRGB
    if(Info.OpenGL_EXT_texture_sRGB)
    {
        OpenGL->DefaultSpriteTextureFormat = GL_SRGB8_ALPHA8;
        OpenGL->ShaderSimTexReadSRGB = false;
    }

    if(FramebufferSupportsSRGB && Info.OpenGL_EXT_framebuffer_sRGB)
    {
#if 0
        GLuint TestTexture;
        glGenTextures(1, &TestTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, TestTexture);
        glGetError();
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, OpenGL->MaxMultiSampleCount,
                                GL_SRGB8_ALPHA8,
                                1920, 1080,
                                GL_FALSE);
        if(glGetError() == GL_NO_ERROR)
#endif
        {
            OpenGL->DefaultFramebufferTextureFormat = GL_SRGB8_ALPHA8;
            glEnable(GL_FRAMEBUFFER_SRGB);
            OpenGL->ShaderSimTexWriteSRGB = false;
        }

#if 0
        glDeleteTextures(1, &TestTexture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
#endif
    }
#endif

#if HANDMADE_INTERNAL
    if(OpenGL->glDebugMessageCallbackARB)
    {
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        OpenGL->glDebugMessageCallbackARB(OpenGLDebugCallback, 0);
    }
#endif

    GLuint DummyVertexArray;
    OpenGL->glGenVertexArrays(1, &DummyVertexArray);
    OpenGL->glBindVertexArray(DummyVertexArray);

    OpenGL->glGenBuffers(1, &OpenGL->VertexBuffer);
    OpenGL->glGenBuffers(1, &OpenGL->IndexBuffer);

    OpenGL->glGenBuffers(1, &OpenGL->ScreenFillVertexBuffer);
    OpenGL->glGenBuffers(1, &OpenGL->LightMapDebugBuffer);

    u32 AllZeroes = 0;
    glGenTextures(1, &OpenGL->SinglePixelAllZeroesTexture);
    glBindTexture(GL_TEXTURE_2D, OpenGL->SinglePixelAllZeroesTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &AllZeroes);

    for(u32 Y = 0; Y < 4; ++Y)
    {
        for(u32 X = 0; X < 4; ++X)
        {
            OpenGL->White[Y][X] = 0xFFFFFFFF;
        }
    }

    glGenTextures(1, &OpenGL->TextureArray);
    glBindTexture(GL_TEXTURE_2D_ARRAY, OpenGL->TextureArray);

    for(mip_iterator MIP = IterateMIPs(TEXTURE_ARRAY_DIM, TEXTURE_ARRAY_DIM, 0);
        IsValid(&MIP);
        Advance(&MIP))
    {
        OpenGL->glTexImage3D(GL_TEXTURE_2D_ARRAY, MIP.Level, OpenGL->DefaultSpriteTextureFormat,
                             MIP.Image.Width, MIP.Image.Height, OpenGL->MaxTextureCount,
                             0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, 0);
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenTextures(OpenGL->MaxSpecialTextureCount, OpenGL->SpecialTextureHandles);
    for(u32 HandleIndex = 0;
        HandleIndex < OpenGL->MaxSpecialTextureCount;
        ++HandleIndex)
    {
        GLuint Handle = OpenGL->SpecialTextureHandles[HandleIndex];
        glBindTexture(GL_TEXTURE_2D_ARRAY, Handle);

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    glGenTextures(1, &OpenGL->DiffuseLightAtlasHandle);
    glGenTextures(1, &OpenGL->SpecularLightAtlasHandle);
    {
        GLuint Handle = OpenGL->DiffuseLightAtlasHandle;
        glBindTexture(GL_TEXTURE_2D, Handle);
        glTexImage2D(GL_TEXTURE_2D, 0, IsFloat(&OpenGL->DiffuseLightAtlas) ? GL_RGB32F : GL_RGB8, GetLightAtlasWidth(&OpenGL->DiffuseLightAtlas), GetLightAtlasHeight(&OpenGL->DiffuseLightAtlas),
                     0, GL_RGB, IsFloat(&OpenGL->DiffuseLightAtlas) ? GL_FLOAT : GL_UNSIGNED_BYTE, 0);

        Handle = OpenGL->SpecularLightAtlasHandle;
        glBindTexture(GL_TEXTURE_2D, Handle);
        glTexImage2D(GL_TEXTURE_2D, 0, IsFloat(&OpenGL->SpecularLightAtlas) ? GL_RGB32F : GL_RGB8, GetLightAtlasWidth(&OpenGL->SpecularLightAtlas), GetLightAtlasHeight(&OpenGL->SpecularLightAtlas),
                     0, GL_RGB, IsFloat(&OpenGL->SpecularLightAtlas) ? GL_FLOAT : GL_UNSIGNED_BYTE, 0);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

internal void
FreeFramebuffer(open_gl *OpenGL, opengl_framebuffer *Framebuffer)
{
    OpenGL->Header.TotalFramebufferMemory -= Framebuffer->GPUMemoryUsed;

    if(Framebuffer->FramebufferHandle)
    {
        OpenGL->glDeleteFramebuffers(1, &Framebuffer->FramebufferHandle);
        Framebuffer->FramebufferHandle = 0;
    }

    for(u32 ColorIndex = 0;
        ColorIndex < ArrayCount(Framebuffer->ColorHandle);
        ++ColorIndex)
    {
        if(Framebuffer->ColorHandle[ColorIndex])
        {
            glDeleteTextures(1, &Framebuffer->ColorHandle[ColorIndex]);
            Framebuffer->ColorHandle[ColorIndex] = 0;
        }
    }

    if(Framebuffer->DepthHandle)
    {
        glDeleteTextures(1, &Framebuffer->DepthHandle);
        Framebuffer->DepthHandle = 0;
    }
}

internal void
FreeProgram(open_gl *OpenGL, opengl_program_common *Program)
{
    OpenGL->glDeleteProgram(Program->ProgHandle);
    Program->ProgHandle = 0;
}

internal void
OpenGLChangeToSettings(open_gl *OpenGL, game_render_settings *Settings)
{
    //
    // NOTE(casey): Free all dynamic resources
    //

    // TODO(casey): Can we use a tool (on some other machine) to verify
    // that we are actually freeing our resources here?  Because it
    // looks like maybe we aren't, but it's hard to tell, and I'm not
    // sure what we're missing.

    FreeFramebuffer(OpenGL, &OpenGL->ResolveFramebuffer);
    FreeFramebuffer(OpenGL, &OpenGL->DepthPeelBuffer);
    for(u32x DepthPeelIndex = 0;
        DepthPeelIndex < OpenGL->DepthPeelCount;
        ++DepthPeelIndex)
    {
        FreeFramebuffer(OpenGL, &OpenGL->DepthPeelResolveBuffer[DepthPeelIndex]);
    }

#if 0
    for(u32x LightIndex = 0;
        LightIndex < OpenGL->LightBufferCount;
        ++LightIndex)
    {
        light_buffer *LightBuffer = OpenGL->LightBuffers + LightIndex;
        glDeleteFramebuffers(1, &LightBuffer->WriteAllFramebuffer);
        glDeleteFramebuffers(1, &LightBuffer->WriteEmitFramebuffer);
        glDeleteTextures(1, &LightBuffer->FrontEmitTex);
        glDeleteTextures(1, &LightBuffer->BackEmitTex);
        glDeleteTextures(1, &LightBuffer->SurfaceColorTex);
        glDeleteTextures(1, &LightBuffer->NPTex);

        light_buffer NullBuffer = {};
        *LightBuffer = NullBuffer;
    }
#endif

    FreeProgram(OpenGL, &OpenGL->ZBiasNoDepthPeel.Common);
    FreeProgram(OpenGL, &OpenGL->ZBiasDepthPeel.Common);
    FreeProgram(OpenGL, &OpenGL->PeelComposite);
    FreeProgram(OpenGL, &OpenGL->FinalStretch);
    FreeProgram(OpenGL, &OpenGL->ResolveMultisample.Common);

    //
    // NOTE(casey): Create new dynamic resources
    //

    OpenGL->CurrentSettings = *Settings;

    // TODO(casey): Fix the pipeline so that multisampling can be enabled to reduce jaggies
    OpenGL->Multisampling = Settings->MultisamplingHint;
    u32 ResolveFlags = 0;
    if(!Settings->PixelationHint)
    {
        ResolveFlags |= OpenGLFramebuffer_Filtered;
    }

    u32x RenderWidth = Settings->RenderDim.Width;
    u32x RenderHeight = Settings->RenderDim.Height;

    u32 DepthPeelFlags = OpenGLFramebuffer_Depth;
    if(OpenGL->Multisampling)
    {
        DepthPeelFlags |= OpenGLFramebuffer_Multisampled;
        OpenGL->Header.UsedMultisampleCount = OpenGL->MaxMultiSampleCount;
    }
    else
    {
        OpenGL->Header.UsedMultisampleCount = 1;
    }

    OpenGL->DepthPeelCount = Settings->DepthPeelCountHint;
    if(OpenGL->DepthPeelCount > ArrayCount(OpenGL->DepthPeelResolveBuffer))
    {
        OpenGL->DepthPeelCount = ArrayCount(OpenGL->DepthPeelResolveBuffer);
    }

    CompileZBiasProgram(OpenGL, &OpenGL->ZBiasNoDepthPeel, false, OpenGL->SpecularLightAtlas.VoxelDim);
    CompileZBiasProgram(OpenGL, &OpenGL->ZBiasDepthPeel, true, OpenGL->SpecularLightAtlas.VoxelDim);
    CompilePeelComposite(OpenGL, &OpenGL->PeelComposite);
    CompileFinalStretch(OpenGL, &OpenGL->FinalStretch);
    CompileResolveMultisample(OpenGL, &OpenGL->ResolveMultisample);

    OpenGL->ResolveFramebuffer = CreateFramebuffer(OpenGL, RenderWidth, RenderHeight, ResolveFlags, 1);

    OpenGL->DepthPeelBuffer =
        CreateFramebuffer(OpenGL, RenderWidth, RenderHeight, DepthPeelFlags, OpenGLColor_Count);
    for(u32 DepthPeelIndex = 0;
        DepthPeelIndex < OpenGL->DepthPeelCount;
        ++DepthPeelIndex)
    {
        if(OpenGL->Multisampling)
        {
            OpenGL->DepthPeelResolveBuffer[DepthPeelIndex] =
                CreateFramebuffer(OpenGL, RenderWidth, RenderHeight,
                                  DepthPeelFlags & ~OpenGLFramebuffer_Multisampled, OpenGLColor_Count);
        }
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, OpenGL->TextureArray);

    GLuint MinFilter = GL_LINEAR_MIPMAP_NEAREST;
    GLuint MagFilter = GL_LINEAR;
    if(Settings->NearestTexelFiltering)
    {
        MagFilter = GL_NEAREST;
        if(Settings->NoMIPMaps)
        {
            MinFilter = GL_NEAREST;
        }
        else
        {
            MinFilter = GL_NEAREST_MIPMAP_NEAREST;
        }
    }
    else if(Settings->NoMIPMaps)
    {
        MinFilter = GL_LINEAR;
    }

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, MinFilter);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, MagFilter);

    GLuint LightSampling = GL_LINEAR;

    glBindTexture(GL_TEXTURE_2D, OpenGL->DiffuseLightAtlasHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, LightSampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, LightSampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, OpenGL->SpecularLightAtlasHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, LightSampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, LightSampling);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_1D, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    //glBindTexture(GL_TEXTURE_3D, 0);


    {
        OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->ScreenFillVertexBuffer);
        textured_vertex Vertices[] =
        {
            {{-1.0f,  1.0f, 0.0f, 1.0f}, {}, {0.0f, 1.0f}, 0xFFFFFFFF},
            {{-1.0f, -1.0f, 0.0f, 1.0f}, {}, {0.0f, 0.0f}, 0xFFFFFFFF},
            {{ 1.0f,  1.0f, 0.0f, 1.0f}, {}, {1.0f, 1.0f}, 0xFFFFFFFF},
            {{ 1.0f, -1.0f, 0.0f, 1.0f}, {}, {1.0f, 0.0f}, 0xFFFFFFFF},
        };
        OpenGL->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    }

    {
        f32 VSpan = 0.5f*0.125f;
        f32 USpan = VSpan / 32.0f;
        v2 UVSpan = {USpan, VSpan};
        v2 UVCenter = {0.615f, 0.55f};
        v2 UVMin = UVCenter - UVSpan;
        v2 UVMax = UVCenter + UVSpan;
        f32 TopY = 0.0f;
        f32 XDim = 9.0f/16.0f;

        OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->LightMapDebugBuffer);
        textured_vertex Vertices[] =
        {
            {{-1.0f,  TopY, 0.0f, 1.0f}, {}, {UVMin.x, UVMax.y}, 0xFFFFFFFF},
            {{-1.0f, -1.0f, 0.0f, 1.0f}, {}, {UVMin.x, UVMin.y}, 0xFFFFFFFF},
            {{-1.0f + XDim,  TopY, 0.0f, 1.0f}, {}, {UVMax.x, UVMax.y}, 0xFFFFFFFF},
            {{-1.0f + XDim, -1.0f, 0.0f, 1.0f}, {}, {UVMax.x, UVMin.y}, 0xFFFFFFFF},

            {{1.0f - XDim,  TopY, 0.0f, 1.0f}, {}, {UVMin.x, UVMax.y}, 0xFFFFFFFF},
            {{1.0f - XDim, -1.0f, 0.0f, 1.0f}, {}, {UVMin.x, UVMin.y}, 0xFFFFFFFF},
            {{1.0f,  TopY, 0.0f, 1.0f}, {}, {UVMax.x, UVMax.y}, 0xFFFFFFFF},
            {{1.0f, -1.0f, 0.0f, 1.0f}, {}, {UVMax.x, UVMin.y}, 0xFFFFFFFF},
        };
        OpenGL->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    }
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, 0);

    PlatformOpenGLSetVSync(OpenGL, OpenGL->CurrentSettings.
                           RequestVSync);
}

internal opengl_framebuffer *
GetDepthPeelReadBuffer(open_gl *OpenGL, u32 Index)
{
    opengl_framebuffer *PeelBuffer = OpenGL->DepthPeelResolveBuffer + Index;
    return(PeelBuffer);
}

internal void
OpenGLBeginScreenFill(open_gl *OpenGL, GLuint FramebufferHandle, u32 Width, u32 Height)
{
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, FramebufferHandle);
    glViewport(0, 0, Width, Height);
    glScissor(0, 0, Width, Height);
    glDepthFunc(GL_ALWAYS);

    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->ScreenFillVertexBuffer);
}

internal void
OpenGLEndScreenFill(open_gl *OpenGL)
{
    OpenGL->glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthFunc(GL_LEQUAL);
}

internal void
ResolveMultisample(open_gl *OpenGL, opengl_framebuffer *From, opengl_framebuffer *To,
                   u32 Width, u32 Height, GLuint MaskTexture)
{
    OpenGLBeginScreenFill(OpenGL, To->FramebufferHandle, Width, Height);

    UseProgramBegin(OpenGL, &OpenGL->ResolveMultisample);

    OpenGL->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, From->DepthHandle);
    for(u32 ColorIndex = 0;
        ColorIndex < OpenGLColor_Count;
        ++ColorIndex)
    {
        OpenGL->glActiveTexture(GL_TEXTURE1 + ColorIndex);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, From->ColorHandle[ColorIndex]);
    }
    OpenGL->glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, MaskTexture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGL->glActiveTexture(GL_TEXTURE0);

    UseProgramEnd(OpenGL, &OpenGL->ResolveMultisample.Common);

    OpenGLEndScreenFill(OpenGL);
}

internal void
OpenGLBindTex(open_gl *OpenGL, GLenum Slot, GLenum Type, GLuint Handle)
{
    OpenGL->glActiveTexture(Slot);
    glBindTexture(Type, Handle);
}

internal void
OpenGLManageTextures(open_gl *OpenGL, renderer_texture_queue *Queue)
{
    while(Queue->OpCount)
    {
        texture_op *Op = Queue->Ops + Queue->FirstOpIndex;
        if(Op->State == TextureOp_PendingLoad)
        {
            break;
        }
        else if(Op->State == TextureOp_ReadyToTransfer)
        {
            renderer_texture Texture = Op->Texture;
            void *Data = Op->Data;

            if(IsSpecialTexture(Texture))
            {
                GLuint Handle = GetSpecialTextureHandleFor(OpenGL, Texture);
                glBindTexture(GL_TEXTURE_2D_ARRAY, Handle);
#if 0
                glTexImage2D(GL_TEXTURE_2D, 0,
                             OpenGL->DefaultSpriteTextureFormat,
                             Texture.Width, Texture.Height, 0,
                             GL_BGRA_EXT, GL_UNSIGNED_BYTE, Data);
#endif
                OpenGL->glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, OpenGL->DefaultSpriteTextureFormat,
                                     Texture.Width, Texture.Height, 1,
                                     0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, Data);

            }
            else
            {
                u32 TextureIndex = TextureIndexFrom(Texture);
                Assert(TextureIndex < OpenGL->MaxTextureCount);
                glBindTexture(GL_TEXTURE_2D_ARRAY, OpenGL->TextureArray);
                for(mip_iterator MIP = IterateMIPs(Texture.Width,
                                                   Texture.Height,
                                                   Op->Data);
                    IsValid(&MIP);
                    Advance(&MIP))
                {
                    OpenGL->glTexSubImage3D(GL_TEXTURE_2D_ARRAY, MIP.Level, 0, 0,
                                            TextureIndex, MIP.Image.Width, MIP.Image.Height, 1,
                                            GL_BGRA_EXT, GL_UNSIGNED_BYTE, MIP.Image.Pixels);
                }
            }

            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            Assert(Op->State == TextureOp_Empty);
        }

        Queue->TransferMemoryFirstUsed = Op->TransferMemoryLastUsed;

        --Queue->OpCount;
        ++Queue->FirstOpIndex;
        if(Queue->FirstOpIndex >= ArrayCount(Queue->Ops))
        {
            Queue->FirstOpIndex = 0;
        }
    }
}

internal game_render_commands *
OpenGLBeginFrame(open_gl *OpenGL, v2u OSWindowDim, v2u RenderDim, rectangle2i DrawRegion)
{
    game_render_commands *Commands = &OpenGL->RenderCommands;

    Commands->Settings = OpenGL->CurrentSettings;
    Commands->Settings.RenderDim = RenderDim;

    Commands->OSWindowDim = OSWindowDim;
    Commands->OSDrawRegion = DrawRegion;

    Commands->MaxPushBufferSize = sizeof(OpenGL->PushBufferMemory);
    Commands->PushBufferBase = OpenGL->PushBufferMemory;
    Commands->PushBufferDataAt = OpenGL->PushBufferMemory;
    Commands->MaxVertexCount = OpenGL->MaxVertexCount;
    Commands->VertexCount = 0;
    Commands->MaxIndexCount = OpenGL->MaxIndexCount;
    Commands->IndexCount = 0;
    Commands->VertexArray = OpenGL->VertexArray;
    Commands->IndexArray = OpenGL->IndexArray;

    Commands->DiffuseLightAtlas = OpenGL->DiffuseLightAtlas;
    Commands->SpecularLightAtlas = OpenGL->SpecularLightAtlas;

    Commands->MaxLightOccluderCount = MAX_LIGHT_OCCLUDER_COUNT;
    Commands->LightOccluderCount = 0;
    Commands->LightOccluders = OpenGL->LightOccluders;

    Commands->MaxQuadTextureCount = OpenGL->MaxQuadTextureCount;
    Commands->QuadTextureCount = 0;
    Commands->QuadTextures = OpenGL->BitmapArray;

    return(Commands);
}

internal void
OpenGLEndFrame(open_gl *OpenGL, game_render_commands *Commands)
{
    rectangle2i DrawRegion = Commands->OSDrawRegion;
    u32 WindowWidth = Commands->OSWindowDim.Width;
    u32 WindowHeight = Commands->OSWindowDim.Height;

    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    //glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    //glEnable(GL_SAMPLE_ALPHA_TO_ONE);
    glEnable(GL_MULTISAMPLE);

    glEnable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->VertexBuffer);
    OpenGL->glBufferData(GL_ARRAY_BUFFER,
                         Commands->VertexCount*sizeof(textured_vertex),
                         Commands->VertexArray,
                         GL_STREAM_DRAW);
    OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBuffer);
    OpenGL->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         Commands->IndexCount*sizeof(u16),
                         Commands->IndexArray,
                         GL_STREAM_DRAW);

    OpenGL->glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, OpenGL->DiffuseLightAtlasHandle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetLightAtlasWidth(&OpenGL->DiffuseLightAtlas),GetLightAtlasHeight(&OpenGL->DiffuseLightAtlas),
                    GL_RGB, IsFloat(&OpenGL->DiffuseLightAtlas) ? GL_FLOAT : GL_UNSIGNED_BYTE, GetLightAtlasTexels(&OpenGL->DiffuseLightAtlas));
    glBindTexture(GL_TEXTURE_2D, OpenGL->SpecularLightAtlasHandle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GetLightAtlasWidth(&OpenGL->SpecularLightAtlas),GetLightAtlasHeight(&OpenGL->SpecularLightAtlas),
                    GL_RGB, IsFloat(&OpenGL->SpecularLightAtlas) ? GL_FLOAT : GL_UNSIGNED_BYTE, GetLightAtlasTexels(&OpenGL->SpecularLightAtlas));
    glBindTexture(GL_TEXTURE_2D, 0);

    if(!AreEqual(&Commands->Settings, &OpenGL->CurrentSettings))
    {
        OpenGLChangeToSettings(OpenGL, &Commands->Settings);
    }

    OpenGLManageTextures(OpenGL, &OpenGL->Header.TextureQueue);

    // TODO(casey): Error dialog in platform layer is UseRenderTargets can't happen
    b32 UseRenderTargets = (OpenGL->glBindFramebuffer != 0);
    Assert(UseRenderTargets);

    game_render_settings *Settings = &Commands->Settings;
    u32 RenderWidth = Settings->RenderDim.Width;
    u32 RenderHeight = Settings->RenderDim.Height;

    glClearDepth(1.0f);

    Assert(OpenGL->DepthPeelCount > 0);
    u32x MaxRenderTargetIndex = OpenGL->DepthPeelCount - 1;
    u32 OnPeelIndex = 0;
    u8 *PeelHeaderRestore = 0;
    u32 CurrentRenderTargetIndex = 0xFFFFFFFF;
    m4x4 Proj = Identity();
    for(u8 *HeaderAt = Commands->PushBufferBase;
        HeaderAt < Commands->PushBufferDataAt;
        )
    {
        render_group_entry_header *Header = (render_group_entry_header *)HeaderAt;
        HeaderAt += sizeof(render_group_entry_header);
        void *Data = (uint8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
            case RenderGroupEntryType_render_entry_full_clear:
            {
                HeaderAt += sizeof(render_entry_full_clear);
                render_entry_full_clear *Entry = (render_entry_full_clear *)Data;

                glClearColor(Entry->ClearColor.r, Entry->ClearColor.g, Entry->ClearColor.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_begin_peels:
            {
                HeaderAt += sizeof(render_entry_begin_peels);
                render_entry_begin_peels *Entry = (render_entry_begin_peels *)Data;

                PeelHeaderRestore = (u8 *)Header;
                OpenGLBindFramebuffer(OpenGL, &OpenGL->DepthPeelBuffer, RenderWidth, RenderHeight);

                glScissor(0, 0, RenderWidth, RenderHeight);
                if(OnPeelIndex == MaxRenderTargetIndex)
                {
                    glClearColor(Entry->ClearColor.r, Entry->ClearColor.g, Entry->ClearColor.b, 1.0f);
                }
                else
                {
                    glClearColor(0, 0, 0, 0);
                }
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_end_peels:
            {
                if(OpenGL->Multisampling)
                {
                    opengl_framebuffer *From = &OpenGL->DepthPeelBuffer;
                    opengl_framebuffer *To = OpenGL->DepthPeelResolveBuffer + OnPeelIndex;
                    GLuint Mask = 0;
                    if(OnPeelIndex == 0)
                    {
                        Mask = OpenGL->SinglePixelAllZeroesTexture;
                    }
                    else
                    {
                        Mask = OpenGL->DepthPeelResolveBuffer[OnPeelIndex - 1].ColorHandle[0];
                    }
#if 1
                    ResolveMultisample(OpenGL, From, To, RenderWidth, RenderHeight, Mask);
#else
                    glBindFramebuffer(GL_READ_FRAMEBUFFER, From->FramebufferHandle);
                    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, To->FramebufferHandle);
                    glViewport(0, 0, RenderWidth, RenderHeight);
                    glBlitFramebuffer(0, 0, RenderWidth, RenderHeight,
                                      0, 0, RenderWidth, RenderHeight,
                                      GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT,
                                      GL_NEAREST);
#endif
                }

                if(OnPeelIndex < MaxRenderTargetIndex)
                {
                    HeaderAt = PeelHeaderRestore;
                    OnPeelIndex++;
                }
                else
                {
                    Assert(OnPeelIndex == MaxRenderTargetIndex);

                    opengl_framebuffer *PeelBuffer = GetDepthPeelReadBuffer(OpenGL, 0);
                    OpenGLBindFramebuffer(OpenGL, PeelBuffer, RenderWidth, RenderHeight);
                    OnPeelIndex = 0;
                    glEnable(GL_BLEND);
                }
            } break;

            case RenderGroupEntryType_render_entry_depth_clear:
            {
                glClear(GL_DEPTH_BUFFER_BIT);
            } break;

            case RenderGroupEntryType_render_entry_textured_quads:
            {
                OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->VertexBuffer);
                OpenGL->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL->IndexBuffer);

                HeaderAt += sizeof(render_entry_textured_quads);
                render_entry_textured_quads *Entry = (render_entry_textured_quads *)Data;

                b32x Peeling = (OnPeelIndex > 0);

                render_setup *Setup = &Entry->Setup;

                rectangle2 ClipRect = Setup->ClipRect;
                s32 ClipMinX = S32BinormalLerp(0, ClipRect.Min.x, RenderWidth);
                s32 ClipMinY = S32BinormalLerp(0, ClipRect.Min.y, RenderHeight);
                s32 ClipMaxX = S32BinormalLerp(0, ClipRect.Max.x, RenderWidth);
                s32 ClipMaxY = S32BinormalLerp(0, ClipRect.Max.y, RenderHeight);
                glScissor(ClipMinX, ClipMinY, ClipMaxX - ClipMinX, ClipMaxY - ClipMinY);

                zbias_program *Prog = &OpenGL->ZBiasNoDepthPeel;
                f32 AlphaThreshold = 0.0f;
                if(Peeling)
                {
                    opengl_framebuffer *PeelBuffer = GetDepthPeelReadBuffer(OpenGL, OnPeelIndex - 1);

                    Prog = &OpenGL->ZBiasDepthPeel;
                    OpenGL->glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, PeelBuffer->DepthHandle);
                    OpenGL->glActiveTexture(GL_TEXTURE0);

                    if(OnPeelIndex == MaxRenderTargetIndex)
                    {
                        AlphaThreshold = 0.9f;
                    }
                }

                OpenGL->glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, OpenGL->DiffuseLightAtlasHandle);
                OpenGL->glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, OpenGL->SpecularLightAtlasHandle);

                OpenGL->glActiveTexture(GL_TEXTURE0);

                UseProgramBegin(OpenGL, Prog, Commands, Setup, AlphaThreshold);

                if(Entry->QuadTextures)
                {
                    // NOTE(casey): This is the multiple-dispatch slow path, for
                    // arbitrary sized textures
                    u32 IndexIndex = Entry->IndexArrayOffset;
                    for(u32 QuadIndex = 0;
                        QuadIndex < Entry->QuadCount;
                        ++QuadIndex)
                    {
                        renderer_texture Texture = Entry->QuadTextures[QuadIndex];
                        GLuint TextureHandle = GetSpecialTextureHandleFor(OpenGL, Texture);
                        glBindTexture(GL_TEXTURE_2D_ARRAY, TextureHandle);
                        OpenGL->glDrawElementsBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT,
                                                         (GLvoid *)(IndexIndex*sizeof(u16)),
                                                         Entry->VertexArrayOffset);
                        IndexIndex += (6*QuadIndex);
                    }
                }
                else
                {
                    // NOTE(casey): This is the single-dispatch fast path, for texture arrays
                    glBindTexture(GL_TEXTURE_2D_ARRAY, OpenGL->TextureArray);
                    OpenGL->glDrawElementsBaseVertex(GL_TRIANGLES, 6*Entry->QuadCount, GL_UNSIGNED_SHORT,
                                                     (GLvoid *)(Entry->IndexArrayOffset*sizeof(u16)),
                                                     Entry->VertexArrayOffset);
                }
                glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

                UseProgramEnd(OpenGL, &Prog->Common);
                if(Peeling)
                {
                    OpenGL->glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    OpenGL->glActiveTexture(GL_TEXTURE0);
                }
            } break;

            InvalidDefaultCase;
        }
    }

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    //ComputeLightTransport();

    OpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, OpenGL->ResolveFramebuffer.FramebufferHandle);
    glViewport(0, 0, RenderWidth, RenderHeight);
    glScissor(0, 0, RenderWidth, RenderHeight);
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->ScreenFillVertexBuffer);

    UseProgramBegin(OpenGL, &OpenGL->PeelComposite);
    u32 TextureBindIndex = GL_TEXTURE0;
    for(u32 PeelIndex = 0;
        PeelIndex <= MaxRenderTargetIndex;
        ++PeelIndex)
    {
        opengl_framebuffer *PeelBuffer = GetDepthPeelReadBuffer(OpenGL, PeelIndex);
        OpenGL->glActiveTexture(TextureBindIndex++);
        glBindTexture(GL_TEXTURE_2D, PeelBuffer->ColorHandle[OpenGLColor_SurfaceReflect]);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    OpenGL->glActiveTexture(GL_TEXTURE0);
    UseProgramEnd(OpenGL, &OpenGL->PeelComposite);

    OpenGL->glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glViewport(0, 0, WindowWidth, WindowHeight);
    glScissor(0, 0, WindowWidth, WindowHeight);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glViewport(DrawRegion.Min.x, DrawRegion.Min.y, GetWidth(DrawRegion), GetHeight(DrawRegion));
    glScissor(DrawRegion.Min.x, DrawRegion.Min.y, GetWidth(DrawRegion), GetHeight(DrawRegion));

    UseProgramBegin(OpenGL, &OpenGL->FinalStretch);

    glBindTexture(GL_TEXTURE_2D, OpenGL->ResolveFramebuffer.ColorHandle[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    UseProgramEnd(OpenGL, &OpenGL->FinalStretch);

#if 0
    OpenGL->glBindBuffer(GL_ARRAY_BUFFER, OpenGL->LightMapDebugBuffer);
    UseProgramBegin(OpenGL, &OpenGL->FinalStretch);
    glBindTexture(GL_TEXTURE_2D, OpenGL->DiffuseLightAtlasHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindTexture(GL_TEXTURE_2D, OpenGL->SpecularLightAtlasHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);

    UseProgramEnd(OpenGL, &OpenGL->FinalStretch);
#endif

    glBindTexture(GL_TEXTURE_2D, 0);
    OpenGL->glActiveTexture(GL_TEXTURE0);

}
