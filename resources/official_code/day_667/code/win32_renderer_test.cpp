/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "win32_renderer_test.h"
#include "win32_handmade_renderer.h"
#include "handmade_renderer.cpp"
#include "handmade_camera.cpp"

volatile global b32x GlobalRunning;

internal b32x
IsEmpty(test_scene *Scene, u32 X, u32 Y)
{
    b32x Result = (Scene->Elements[Y][X].Type == Element_Grass);
    return(Result);
}

internal u32x
CountOccupantsIn3x3(test_scene *Scene, u32 CenterX, u32 CenterY)
{
    u32 OccupantCount = 0;
    
    for(u32 Y = (CenterY - 1);
        Y <= (CenterY + 1);
        ++Y)
    {
        for(u32 X = (CenterX - 1);
            X <= (CenterX + 1);
            ++X)
        {
            if(!IsEmpty(Scene, X, Y))
            {
                ++OccupantCount;
            }
        }
    }
    
    return(OccupantCount);
}

internal void
PlaceRandomInUnoccupied(test_scene *Scene, test_scene_element_type Element, u32 Count)
{
    u32 Placed = 0;
    while(Placed < Count)
    {
        u32 X = 1 + (rand() % (TEST_SCENE_DIM_X - 1));
        u32 Y = 1 + (rand() % (TEST_SCENE_DIM_Y - 1));
        
        if(CountOccupantsIn3x3(Scene, X, Y) == 0)
        {
            Scene->Elements[Y][X].Type = Element;
            ++Placed;
        }
    }
}

internal b32x
PlaceRectangularWall(test_scene *Scene, u32 MinX, u32 MinY, u32 MaxX, u32 MaxY)
{
    b32x Placed = true;
    
    cube_uv_layout WallUV = EncodeCubeUVLayout(0, 0,
                                               2, 2, 2, 2,
                                               0, 0);
    
    for(u32 Pass = 0;
        Placed && (Pass <= 1);
        ++Pass)
    {
        for(u32 X = MinX;
            X <= MaxX;
            ++X)
        {
            if(Pass == 0)
            {
                if(!(IsEmpty(Scene, X, MinY) && IsEmpty(Scene, X, MaxY)))
                {
                    Placed = false;
                    break;
                }
            }
            else
            {
                Scene->Elements[MinY][X].Type = Element_Wall;
                Scene->Elements[MaxY][X].Type = Element_Wall;
                
                if(X == MinX)
                {
                    Scene->Elements[MinY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               1, 1, 3, 1,
                                                                               0, 0);
                    Scene->Elements[MaxY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               1, 3, 1, 1,
                                                                               0, 0);
                }
                else if(X == MaxX)
                {
                    Scene->Elements[MinY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               1, 3, 3, 3,
                                                                               0, 1);
                    Scene->Elements[MaxY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               3, 1, 3, 3,
                                                                               0, 0);
                }
                else if(X == (MinX + 1))
                {
                    Scene->Elements[MinY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               2, 3, 2, 2,
                                                                               0, 0);
                    Scene->Elements[MaxY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               2, 2, 2, 1,
                                                                               0, 0);
                }
                else if(X == (MaxX - 1))
                {
                    Scene->Elements[MinY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               2, 1, 2, 2,
                                                                               0, 0);
                    Scene->Elements[MaxY][X].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               2, 2, 2, 3,
                                                                               0, 0);
                }
                else
                {
                    Scene->Elements[MinY][X].CubeUVLayout = WallUV;
                    Scene->Elements[MaxY][X].CubeUVLayout = WallUV;
                }
            }
        }
        
        for(u32 Y = (MinY + 1);
            Y < MaxY;
            ++Y)
        {
            if(Pass == 0)
            {
                if(!(IsEmpty(Scene, MinX, Y) && IsEmpty(Scene, MaxX, Y)))
                {
                    Placed = false;
                    break;
                }
            }
            else
            {
                Scene->Elements[Y][MinX].Type = Scene->Elements[Y][MaxX].Type = Element_Wall;
                
                if(Y == (MinY + 1))
                {
                    Scene->Elements[Y][MinX].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               1, 2, 2, 2,
                                                                               0, 0);
                    Scene->Elements[Y][MaxX].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               2, 2, 3, 2,
                                                                               0, 0);
                }
                else if(Y == (MaxY - 1))
                {
                    Scene->Elements[Y][MinX].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               3, 2, 2, 2,
                                                                               0, 0);
                    Scene->Elements[Y][MaxX].CubeUVLayout = EncodeCubeUVLayout(0, 0,
                                                                               2, 2, 1, 2,
                                                                               0, 0);
                }
                else
                {
                    Scene->Elements[Y][MinX].CubeUVLayout = 
                        Scene->Elements[Y][MaxX].CubeUVLayout =
                        WallUV;
                }
            }
        }
    }
    
    return(Placed);
}

internal void
InitTestScene(renderer_texture_queue *TextureOpQueue, test_scene *Scene)
{
    Scene->GrassTexture = LoadBMP(TextureOpQueue, "test_cube_grass.bmp", 1);
    Scene->WallTexture = LoadBMP(TextureOpQueue, "test_cube_wall.bmp", 2);
    Scene->TreeTexture = LoadBMP(TextureOpQueue, "test_sprite_tree.bmp", 3);
    Scene->HeadTexture = LoadBMP(TextureOpQueue, "test_sprite_head.bmp", 4);
    Scene->CoverTexture = LoadBMP(TextureOpQueue, "test_cover_grass.bmp", 5);
    
    Scene->MinP = V3(-0.5f*(f32)TEST_SCENE_DIM_X,
                     -0.5f*(f32)TEST_SCENE_DIM_Y,
                     0.0f);
    
    for(u32 WallIndex = 0;
        WallIndex < 8;
        ++WallIndex)
    {
        u32 X = 1 + (rand() % (TEST_SCENE_DIM_X - 10));
        u32 Y = 1 + (rand() % (TEST_SCENE_DIM_Y - 10));
        
        u32 DimX = 2 + (rand() % 6);
        u32 DimY = 2 + (rand() % 6);
        
        PlaceRectangularWall(Scene, X, Y, X + DimX, Y + DimY);
    }
    
    u32 TotalSquareCount = TEST_SCENE_DIM_X*TEST_SCENE_DIM_Y;
    PlaceRandomInUnoccupied(Scene, Element_Tree, TotalSquareCount/15);
}

internal void
PushSimpleScene(render_group *Group, test_scene *Scene)
{
    f32 WallRadius = 1.0f;
    
    srand(1234);
    for(s32 Y = 0;
        Y < TEST_SCENE_DIM_Y;
        ++Y)
    {
        for(s32 X = 0;
            X < TEST_SCENE_DIM_X;
            ++X)
        {
            test_scene_element Elem = Scene->Elements[Y][X];
            
            f32 Z = 0.4f*((f32)rand() / (f32)RAND_MAX);
            f32 R = 0.5f + 0.5f*((f32)rand() / (f32)RAND_MAX);
            if(Elem.Type == Element_Wall)
            {
                Z = 0.4f;
                R = 1.0f;
            }
            
            f32 ZRadius = 2.0f;
            v4 Color = V4(R, 1, 1, 1);
            v3 P = Scene->MinP + V3((f32)X, (f32)Y, Z);
            PushCube(Group, Scene->GrassTexture, P, V3(0.5f, 0.5f, ZRadius), Color);
            
            v3 GroundP = P + V3(0, 0, ZRadius);
            if(Elem.Type == Element_Tree)
            {
                PushUpright(Group, Scene->TreeTexture, GroundP, V2(2.0f, 2.5f));
            }
            else if(Elem.Type == Element_Wall)
            {
                PushCube(Group, Scene->WallTexture, GroundP + V3(0, 0, WallRadius), V3(0.5f, 0.5f, WallRadius), Color, Elem.CubeUVLayout);
            }
            else
            {
                for(u32 CoverIndex = 0;
                    CoverIndex < 60;
                    ++CoverIndex)
                {
                    v2 Disp = 0.8f*V2((f32)rand() / (f32)RAND_MAX,
                                      (f32)rand() / (f32)RAND_MAX) - V2(0.4f, 0.4f);
                    PushUpright(Group, Scene->CoverTexture, GroundP + V3(Disp, 0.0f), V2(0.4f, 0.4f));
                }
            }
        }
    }
    
    PushUpright(Group, Scene->HeadTexture, V3(0, 2.0f, 3.0f), V2(4.0f, 4.0f));
}

internal DWORD WINAPI
RenderLoop(LPVOID lpParameter)
{
    // NOTE(casey): When the render thread is created, the HWND render target
    // is passed as the lpParameter.
    HWND Window = (HWND)lpParameter;
    
    // NOTE(casey): We are going to want to time our rendering, so we use this
    // little utility structure that is defined later in this file.  It has
    // nothing to do with the render API, it just calls Win32 to get the timing.
    frame_stats Stats = InitFrameStats();
    
    platform_renderer_limits Limits = {};
    Limits.MaxQuadCountPerFrame = (1 << 18);
    Limits.MaxTextureCount = 256;
    Limits.TextureTransferBufferSize = (16*1024*1024);
    
    // NOTE(casey): Load the renderer DLL and get the address of the init function
    HMODULE RendererDLL = LoadLibrary(TEXT("win32_handmade_opengl.dll"));
    win32_load_renderer *Win32LoadOpenGLRenderer = (win32_load_renderer *)
        GetProcAddress(RendererDLL, Win32RendererFunctionTableNames[0]);
    renderer_begin_frame *Win32BeginFrame = (renderer_begin_frame *)
        GetProcAddress(RendererDLL, Win32RendererFunctionTableNames[1]);
    renderer_end_frame *Win32EndFrame = (renderer_end_frame *)
        GetProcAddress(RendererDLL, Win32RendererFunctionTableNames[2]);
    if(!Win32LoadOpenGLRenderer ||
       !Win32BeginFrame ||
       !Win32EndFrame)
    {
        MessageBox(Window,
                   TEXT("Please make sure win32_handmade_opengl.dll is present in the same directory as the win32_renderer_test.exe executable."),
                   TEXT("Unable to load win32_handmade_opengl.dll"),
                   MB_OK|MB_ICONERROR);
        ExitProcess(0);
    }
    
    // NOTE(casey): Initialize OpenGL so that it can render to our
    // window.  The Win32 OpenGL startup code is contained within the rendering DLL,
    // so we get the DC for our window and pass that to its loader call
    // so it can do all the startup for us.
    platform_renderer *Renderer = Win32LoadOpenGLRenderer(GetDC(Window), &Limits);
    
    // NOTE(casey): Now we initialize our test scene.  This has nothing to
    // do with the rendering API, it's just a way of making a data structure
    // we can use later to figure out what we want to render every frame.
    test_scene Scene = {};
    InitTestScene(&Renderer->TextureQueue, &Scene);
    
    // NOTE(casey): We get the default camera parameters for viewing a scene (at meter scale)
    // from a reasonable 3rd-person perspective
    camera Camera = GetStandardCamera();
    
    f32 tCameraShift = 0.0f; // Accumulator used in the rendering loop to animate the camera
    
    // NOTE(casey): The camera goes through two animation tests in the loop.
    // First it does a rotation around the scene (CameraIsPanning == false) with no panning,
    // then it does a pan around scene (CameraIsPanning == true) with no rotation,
    // just to test how both types of camera work look.
    b32 CameraIsPanning = true;
    
    // NOTE(Casey): You can set whatever render dimension you want, and
    // it will be aspect-aware stretched to fit the actual dimension of the window/screen
    v2u RenderDim = {1920, 1080};
    
    while(GlobalRunning)
    {
        // NOTE(casey): Get the size of our window from BlomboOS
        RECT ClientRect;
        GetClientRect(Window, &ClientRect);
        s32 WindowWidth = ClientRect.right - ClientRect.left;
        s32 WindowHeight = ClientRect.bottom - ClientRect.top;
        
        // NOTE(casey): Use our utility function to figure out how to fit a 16:9 drawing
        // region in there - this is not necessary if you don't want a fixed aspect
        // ratio, you can just use the whole rectangle if you want!  It would be:
        // rectangle2i DrawRegion = RectMinMax(0, 0, WindowWidth, WindowHeight);
        rectangle2i DrawRegion = AspectRatioFit(RenderDim.Width, RenderDim.Height, WindowWidth, WindowHeight);
        
        // NOTE(casey): To test both rotation and panning, we animate the camera location
        // in two sequential circles: one orbit, then one panned circle with the orientation
        // locked to forward.  This is all just math we do here in the test app, none
        // of it is related to the renderer.
        if(tCameraShift > Tau32)
        {
            tCameraShift -= Tau32;
            CameraIsPanning = !CameraIsPanning;
        }
        
        if(CameraIsPanning)
        {
            Camera.Offset = 10.0f*V3(cosf(tCameraShift), -0.2f + sinf(tCameraShift), 0.0f);
        }
        else
        {
            Camera.Orbit = tCameraShift;
        }
        
        // NOTE(casey): Tell the renderer to begin rendering a frame that occupies
        // DrawRegion inside a window that is WindowWidth and WindowHeight big.
        game_render_commands *Frame =
            Win32BeginFrame(Renderer, V2U(WindowWidth, WindowHeight), RenderDim, DrawRegion);
        
        // NOTE(casey): Although we may be forced through VSync anyway (either by the
        // Windows compositor or by the GPU settings), we ask for no vsync for this
        // test anyway, just in case we can get better timings.
        Frame->Settings.RequestVSync = false;
        Frame->Settings.LightingDisabled = false;
        
        // NOTE(casey): Draw a single render group, that starts with a clear screen
        v4 BackgroundColor = V4(0.15f, 0.15f, 0.15f, 0.0f);
        render_group Group = BeginRenderGroup(0, Frame, Render_Default, BackgroundColor);
        
        // NOTE(casey): Specify where to view the scene from
        ViewFromCamera(&Group, &Camera);
        
        // NOTE(casey): Give the renderer all the stuff that makes up our scene
        PushSimpleScene(&Group, &Scene);
        
        // NOTE(casey): Finish the render group
        EndRenderGroup(&Group);
        
        // NOTE(casey): Tell the renderer to actually render the frame (we have
        // no more render groups to send).
        Win32EndFrame(Renderer, Frame);
        
        f32 SecondsElapsed = UpdateFrameStats(&Stats);
        tCameraShift += 0.1f*SecondsElapsed;
    }
    
    return(0);
}

//
//
//
//
//
// NOTE(casey): Everything below here is just Win32 code to open a window and
// file code to load BMPs for textures.  None of it is related to the renderer API!
//
//
//
//
//

internal frame_stats
InitFrameStats(void)
{
    frame_stats Stats = {};
    
    QueryPerformanceFrequency(&Stats.PerfCountFrequencyResult);
    Stats.MinSPF = F32Max;
    
    return(Stats);
}

internal f32
UpdateFrameStats(frame_stats *Stats)
{
    f32 SecondsElapsed = 0.0f;
    
    LARGE_INTEGER EndCounter;
    QueryPerformanceCounter(&EndCounter);
    
    if(Stats->LastCounter.QuadPart != 0)
    {
        SecondsElapsed = ((f32)(EndCounter.QuadPart - Stats->LastCounter.QuadPart) /
                          (f32)Stats->PerfCountFrequencyResult.QuadPart);
        if(Stats->MinSPF > SecondsElapsed)
        {
            Stats->MinSPF = SecondsElapsed;
        }
        
        if(Stats->MaxSPF < SecondsElapsed)
        {
            Stats->MaxSPF = SecondsElapsed;
        }
        
        if(Stats->DisplayCounter++ == 120)
        {
            TCHAR MS[256];
            _stprintf_s(MS, ArrayCount(MS), TEXT("Min:%.02fms Max:%.02fms\n"),
                        1000.0f*Stats->MinSPF, 1000.0f*Stats->MaxSPF);
            OutputDebugString(MS);
            Stats->MinSPF = F32Max;
            Stats->MaxSPF = 0.0f;
            Stats->DisplayCounter = 0;
        }
    }
    
    Stats->LastCounter = EndCounter;
    
    return(SecondsElapsed);
}

internal void *
Win32AllocateMemory(umm Size)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return(Result);
}

internal entire_file
ReadEntireFile(char *FileName)
{
    entire_file Result = {};
    
    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);
        
        Result.Contents = Win32AllocateMemory(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else
    {
        printf("ERROR: Cannot open file %s.\n", FileName);
    }
    
    return(Result);
}

internal renderer_texture
LoadBMP(renderer_texture_queue *TextureOpQueue, char *FileName,
        u16 TextureIndex)
{
    loaded_bitmap Result = {};
    renderer_texture Texture = {};
    
    entire_file ReadResult = ReadEntireFile(FileName);
    if(ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        uint32 *Pixels = (uint32 *)((uint8 *)ReadResult.Contents + Header->BitmapOffset);
        Result.Memory = Pixels;
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        
        Assert(Result.Height >= 0);
        Assert(Header->Compression == 3);
        
        // NOTE(casey): If you are using this generically for some reason,
        // please remember that BMP files CAN GO IN EITHER DIRECTION and
        // the height will be negative for top-down.
        // (Also, there can be compression, etc., etc... DON'T think this
        // is complete BMP loading code because it isn't!!)
        
        // NOTE(casey): Byte order in memory is determined by the Header itself,
        // so we have to read out the masks and convert the pixels ourselves.
        uint32 RedMask = Header->RedMask;
        uint32 GreenMask = Header->GreenMask;
        uint32 BlueMask = Header->BlueMask;
        uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
        
        bit_scan_result RedScan = FindLeastSignificantSetBit(RedMask);
        bit_scan_result GreenScan = FindLeastSignificantSetBit(GreenMask);
        bit_scan_result BlueScan = FindLeastSignificantSetBit(BlueMask);
        bit_scan_result AlphaScan = FindLeastSignificantSetBit(AlphaMask);
        
        Assert(RedScan.Found);
        Assert(GreenScan.Found);
        Assert(BlueScan.Found);
        Assert(AlphaScan.Found);
        
        int32 RedShiftDown = (int32)RedScan.Index;
        int32 GreenShiftDown = (int32)GreenScan.Index;
        int32 BlueShiftDown = (int32)BlueScan.Index;
        int32 AlphaShiftDown = (int32)AlphaScan.Index;
        
        texture_op *Op = BeginTextureOp(TextureOpQueue, Result.Width*Result.Height*4);
        if(Op)
        {
            Texture = ReferToTexture(TextureIndex, Result.Width, Result.Height);
            Op->Texture = Texture;
            
            u32 *Source = Pixels;
            u32 *Dest = (u32 *)Op->Data;
            for(int32 Y = 0;
                Y < Header->Height;
                ++Y)
            {
                for(int32 X = 0;
                    X < Header->Width;
                    ++X)
                {
                    uint32 C = *Source++;
                    
                    v4 Texel = {(real32)((C & RedMask) >> RedShiftDown),
                        (real32)((C & GreenMask) >> GreenShiftDown),
                        (real32)((C & BlueMask) >> BlueShiftDown),
                        (real32)((C & AlphaMask) >> AlphaShiftDown)};
                    
                    Texel = SRGB255ToLinear1(Texel);
                    Texel.rgb *= Texel.a;
                    Texel = Linear1ToSRGB255(Texel);
                    
                    *Dest++ = (((uint32)(Texel.a + 0.5f) << 24) |
                               ((uint32)(Texel.r + 0.5f) << 16) |
                               ((uint32)(Texel.g + 0.5f) << 8) |
                               ((uint32)(Texel.b + 0.5f) << 0));
                }
            }
            
            CompleteTextureOp(TextureOpQueue, Op);
        }
        else
        {
            printf("ERROR: Out of texture transfer operations.\n");
        }
    }
    
    return(Texture);
}

internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR CommandLine,
        int ShowCommand)
{
    WNDCLASS WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.lpszClassName = TEXT("HandmadeHeroRendererTestWindowClass");
    
    if(RegisterClass(&WindowClass))
    {
        HWND Window =
            CreateWindowEx(
            0,
            WindowClass.lpszClassName,
            TEXT("Handmade Hero Renderer Test"),
            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);
        if(Window)
        {
            GlobalRunning = true;
            
            DWORD ThreadID;
            HANDLE ThreadHandle = CreateThread(0, Megabytes(16), RenderLoop, Window, 0, &ThreadID);
            CloseHandle(ThreadHandle);
            
            while(GlobalRunning)
            {
                MSG Message;
                DWORD LastMessage = 0;
                if(GetMessage(&Message, 0, 0, 0) > 0)
                {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                else
                {
                    GlobalRunning = false;
                }
            }
        }
    }
    
    ExitProcess(0);
}
