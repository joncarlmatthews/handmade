/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  TODO(casey):  THIS IS NOT A FINAL PLATFORM LAYER!!!
  
  - Make the right calls so Windows doesn't think we're "still loading" for a bit after we actually start
  - Saved game locations
  - Getting a handle to our own executable file
  - Raw Input (support for multiple keyboards)
  - ClipCursor() (for multimonitor support)
  - QueryCancelAutoplay
  - WM_ACTIVATEAPP (for when we are not the active application)
  - GetKeyboardLayout (for French keyboards, international WASD support)
  - ChangeDisplaySettings option if we detect slow fullscreen blit??
  
   Just a partial list of stuff!!
*/

#include "handmade_platform.h"
#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_shared.h"
#include "handmade_light_atlas.h"
#include "handmade_memory.h"

#undef function
#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <bcrypt.h>
#include <gl/gl.h>
#define function static

#include "win32_handmade.h"

//
// NOTE(casey): We prefer the discrete GPU in laptops where available
//
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x01;
    __declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x01;
}

platform_api Platform;

global win32_state GlobalWin32State;
global b32 GlobalSoftwareRendering;
global b32 GlobalRunning;
global b32 GlobalPause;
global v2u GlobalEnforcedAspectRatio = {16, 9};
global LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global s64 GlobalPerfCountFrequency;
global b32 DEBUGGlobalShowCursor;
global WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};
global GLuint GlobalBlitTextureHandle;

#include "handmade_renderer.h"
#include "handmade_renderer.cpp"
#include "win32_handmade_renderer.h"

// NOTE(casey): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(casey): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    return(ERROR_DEVICE_NOT_CONNECTED);
}
global x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(casey): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName, u32 Unique,
                          int DestCount, char *Dest)
{
    string A =
    {
        (umm)(State->OnePastLastEXEFileNameSlash - State->EXEFileName),
        (u8 *)State->EXEFileName,
    };
    string B = WrapZ(FileName);
    if(Unique == 0)
    {
        FormatString(DestCount, Dest, "%S%S", A, B);
    }
    else
    {
        FormatString(DestCount, Dest, "%S%d_%S", A, Unique, B);
    }
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    Win32BuildEXEPathFileName(State, FileName, 0, DestCount, Dest);
}

#if HANDMADE_INTERNAL
DEBUG_PLATFORM_EXECUTE_SYSTEM_COMMAND(DEBUGExecuteSystemCommand)
{
    debug_executing_process Result = {};
    
    STARTUPINFO StartupInfo = {};
    StartupInfo.cb = sizeof(StartupInfo);
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
    StartupInfo.wShowWindow = SW_HIDE;
    
    PROCESS_INFORMATION ProcessInfo = {};
    if(CreateProcess(Command,
                     CommandLine,
                     0,
                     0,
                     FALSE,
                     0,
                     0,
                     Path,
                     &StartupInfo,
                     &ProcessInfo))
    {
        Assert(sizeof(Result.OSHandle) >= sizeof(ProcessInfo.hProcess));
        *(HANDLE *)&Result.OSHandle = ProcessInfo.hProcess;
    }
    else
    {
        DWORD ErrorCode = GetLastError();
        *(HANDLE *)&Result.OSHandle = INVALID_HANDLE_VALUE;
    }
    
    return(Result);
}

DEBUG_PLATFORM_GET_PROCESS_STATE(DEBUGGetProcessState)
{
    debug_process_state Result = {};
    
    HANDLE hProcess = *(HANDLE *)&Process.OSHandle;
    if(hProcess != INVALID_HANDLE_VALUE)
    {
        Result.StartedSuccessfully = true;
        
        if(WaitForSingleObject(hProcess, 0) == WAIT_OBJECT_0)
        {
            DWORD ReturnCode = 0;
            GetExitCodeProcess(hProcess, &ReturnCode);
            Result.ReturnCode = ReturnCode;
            CloseHandle(hProcess);
        }
        else
        {
            Result.IsRunning = true;
        }
    }
    
    return(Result);
}
#endif

inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesExA(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}

inline b32
Win32TimeIsValid(FILETIME Time)
{
    b32 Result = (Time.dwLowDateTime != 0) || (Time.dwHighDateTime != 0);
    return(Result);
}

internal void
Win32UnloadCode(win32_loaded_code *Loaded)
{
    if(Loaded->DLL)
    {
        // TODO(casey): Currently, we never unload libraries, because
        // we may still be pointing to strings that are inside them
        // (despite our best efforts).  Should we just make "never unload"
        // be the policy?
        
        // FreeLibrary(GameCode->GameCodeDLL);
        Loaded->DLL = 0;
    }
    
    Loaded->IsValid = false;
    ZeroArray(Loaded->FunctionCount, Loaded->Functions);
}

internal void
Win32LoadCode(win32_state *State,
              win32_loaded_code *Loaded)
{
    char *SourceDLLName = Loaded->DLLFullPath;
    char *LockFileName = Loaded->LockFullPath;
    
    char TempDLLName[WIN32_STATE_FILE_NAME_COUNT];
    
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesExA(LockFileName, GetFileExInfoStandard, &Ignored))
    {
        Loaded->DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
        
        for(u32 AttemptIndex = 0;
            AttemptIndex < 128;
            ++AttemptIndex)
        {
            Win32BuildEXEPathFileName(State, Loaded->TransientDLLName, Loaded->TempDLLNumber,
                                      sizeof(TempDLLName), TempDLLName);
            if(++Loaded->TempDLLNumber >= 1024)
            {
                Loaded->TempDLLNumber = 0;
            }
            
            if(CopyFile(SourceDLLName, TempDLLName, FALSE))
            {
                break;
            }
        }
        
        Loaded->DLL = LoadLibraryA(TempDLLName);
        if(Loaded->DLL)
        {
            Loaded->IsValid = true;
            for(u32 FunctionIndex = 0;
                FunctionIndex < Loaded->FunctionCount;
                ++FunctionIndex)
            {
                void *Function = GetProcAddress(Loaded->DLL, Loaded->FunctionNames[FunctionIndex]);
                if(Function)
                {
                    Loaded->Functions[FunctionIndex] = Function;
                }
                else
                {
                    Loaded->IsValid = false;
                }
            }
        }
    }
    
    if(!Loaded->IsValid)
    {
        Win32UnloadCode(Loaded);
    }
}

internal b32x
Win32CheckForCodeChange(win32_loaded_code *Loaded)
{
    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(Loaded->DLLFullPath);
    b32x Result = (CompareFileTime(&NewDLLWriteTime, &Loaded->DLLLastWriteTime) != 0);
    return(Result);
}

internal void
Win32ReloadCode(win32_state *State, win32_loaded_code *Loaded)
{
    Win32UnloadCode(Loaded);
    for(u32 LoadTryIndex = 0;
        !Loaded->IsValid && (LoadTryIndex < 100);
        ++LoadTryIndex)
    {
        Win32LoadCode(State, Loaded);
        Sleep(100);
    }
}

internal void
Win32LoadXInput(void)
{
    // TODO(casey): Test this on Windows 8
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
    }
    
    if(!XInputLibrary)
    {
        // TODO(casey): Diagnostic
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}
        
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}
        
        // TODO(casey): Diagnostic
        
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(casey): Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        // NOTE(casey): Get a DirectSound object! - cooperative
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        
        // TODO(casey): Double-check that this works on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels*WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
            
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                // NOTE(casey): "Create" a primary buffer
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        // NOTE(casey): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(casey): Diagnostic
                    }
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
            else
            {
                // TODO(casey): Diagnostic
            }
            
            // TODO(casey): In release mode, should we not specify DSBCAPS_GLOBALFOCUS?
            
            // TODO(casey): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
#if HANDMADE_INTERNAL
            BufferDescription.dwFlags |= DSBCAPS_GLOBALFOCUS;
#endif
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO(casey): Diagnostic
        }
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal v2u
Win32GetWindowDimension(HWND Window)
{
    v2u Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = (u32)(ClientRect.right - ClientRect.left);
    Result.Height = (u32)(ClientRect.bottom - ClientRect.top);
    
    return(Result);
}

internal LRESULT CALLBACK
Win32FadeWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CLOSE:
        {
        } break;
        
        case WM_SETCURSOR:
        {
            SetCursor(0);
        } break;
        
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
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
        {
            // TODO(casey): Handle this with a message to the user?
            GlobalRunning = false;
        } break;
        
        case WM_WINDOWPOSCHANGING:
        {
            if(GetKeyState(VK_SHIFT) & 0x8000)
            {
                WINDOWPOS *NewPos = (WINDOWPOS *)LParam;
                
                RECT WindowRect;
                RECT ClientRect;
                GetWindowRect(Window, &WindowRect);
                GetClientRect(Window, &ClientRect);
                
                s32 ClientWidth = (ClientRect.right - ClientRect.left);
                s32 ClientHeight = (ClientRect.bottom - ClientRect.top);
                s32 WidthAdd = ((WindowRect.right - WindowRect.left) - ClientWidth);
                s32 HeightAdd = ((WindowRect.bottom - WindowRect.top) - ClientHeight);
                
                s32 RenderWidth = GlobalEnforcedAspectRatio.Width;
                s32 RenderHeight = GlobalEnforcedAspectRatio.Height;
                
                if((RenderWidth > 0) &&
                   (RenderHeight > 0))
                {
                    s32 SugX = NewPos->cx;
                    s32 SugY = NewPos->cy;
                    
                    s32 NewCx = (RenderWidth * (NewPos->cy - HeightAdd)) / RenderHeight;
                    s32 NewCy = (RenderHeight * (NewPos->cx - WidthAdd)) / RenderWidth;
                    
                    if(AbsoluteValue((r32)(NewPos->cx - NewCx)) <
                       AbsoluteValue((r32)(NewPos->cy - NewCy)))
                    {
                        NewPos->cx = NewCx + WidthAdd;
                    }
                    else
                    {
                        NewPos->cy = NewCy + HeightAdd;
                    }
                }
            }
            
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
        
        case WM_WINDOWPOSCHANGED:
        {
            // TODO(casey): For now, we are setting the window styles in here
            // because sometimes Windows can reposition our window out of fullscreen
            // without going through our ToggleFullscreen(), and we want to put our
            // title bar and border back when it does!
            
            WINDOWPOS *NewPos = (WINDOWPOS *)LParam;
            
            b32x BecomingFullscreen = false;
            MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
            if(GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY),
                              &MonitorInfo))
            {
                s32 MonWidth = (MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left);
                s32 MonHeight = (MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top);
                BecomingFullscreen = ((MonitorInfo.rcMonitor.left == NewPos->x) &&
                                      (MonitorInfo.rcMonitor.top == NewPos->y) &&
                                      (MonWidth == NewPos->cx) &&
                                      (MonHeight == NewPos->cy));
            }
            
            DWORD OldStyle = GetWindowLong(Window, GWL_STYLE);
            DWORD FullscreenStyle = OldStyle & ~WS_OVERLAPPEDWINDOW;
            DWORD WindowedStyle = OldStyle | WS_OVERLAPPEDWINDOW;
            DWORD NewStyle = (BecomingFullscreen) ? FullscreenStyle : WindowedStyle;
            
            if(NewStyle != OldStyle)
            {
                SetWindowLong(Window, GWL_STYLE, NewStyle);
            }
            
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
        
        case WM_SETCURSOR:
        {
            if(DEBUGGlobalShowCursor)
            {
                // SetCursor(LoadCursor(0, IDC_ARROW));
                Result = DefWindowProcA(Window, Message, WParam, LParam);
            }
            else
            {
                SetCursor(0);
            }
        } break;
        
        case WM_ACTIVATEAPP:
        {
#if 0
            if(WParam == TRUE)
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;
        
        case WM_DESTROY:
        {
            // TODO(casey): Handle this as an error - recreate window?
            GlobalRunning = false;
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            Assert(!"Keyboard input came in through a non-dispatch message!");
        } break;
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            //            OutputDebugStringA("default\n");
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

internal void
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region1Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;
            ByteIndex < Region2Size;
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
                     game_sound_output_buffer *SourceBuffer)
{
    // TODO(casey): More strenuous test!
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite,
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size,
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        
        // TODO(casey): Collapse these two loops
        DWORD Region1SampleCount = Region1Size/SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region1SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD SampleIndex = 0;
            SampleIndex < Region2SampleCount;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState,
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;
    
    if(Value < -DeadZoneThreshold)
    {
        Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
    }
    else if(Value > DeadZoneThreshold)
    {
        Result = (real32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));
    }
    
    return(Result);
}

internal void
Win32GetInputFileLocation(win32_state *State, bool32 InputStream,
                          int SlotIndex, int DestCount, char *Dest)
{
    char Temp[64];
    wsprintf(Temp, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    Win32BuildEXEPathFileName(State, Temp, DestCount, Dest);
}

#if HANDMADE_INTERNAL
internal
DEBUG_PLATFORM_GET_MEMORY_STATS(Win32GetMemoryStats)
{
    debug_platform_memory_stats Stats = {};
    
    BeginTicketMutex(&GlobalWin32State.MemoryMutex);
    win32_memory_block *Sentinel = &GlobalWin32State.MemorySentinel;
    for(win32_memory_block *SourceBlock = Sentinel->Next;
        SourceBlock != Sentinel;
        SourceBlock = SourceBlock->Next)
    {
        Assert(SourceBlock->Block.Size <= U32Max);
        
        ++Stats.BlockCount;
        Stats.TotalSize += SourceBlock->Block.Size;
        Stats.TotalUsed += SourceBlock->Block.Used;
    }
    EndTicketMutex(&GlobalWin32State.MemoryMutex);
    
    return(Stats);
}
#endif

internal void
Win32VerifyMemoryListIntegrity(void)
{
    BeginTicketMutex(&GlobalWin32State.MemoryMutex);
    local_persist u32 FailCounter;
    win32_memory_block *Sentinel = &GlobalWin32State.MemorySentinel;
    for(win32_memory_block *SourceBlock = Sentinel->Next;
        SourceBlock != Sentinel;
        SourceBlock = SourceBlock->Next)
    {
        Assert(SourceBlock->Block.Size <= U32Max);
    }
    ++FailCounter;
    if(FailCounter == 35)
    {
        int BreakHere = 3;
    }
    EndTicketMutex(&GlobalWin32State.MemoryMutex);
}

internal void
Win32BeginRecordingInput(win32_state *State, int InputRecordingIndex)
{
    char FileName[WIN32_STATE_FILE_NAME_COUNT];
    Win32GetInputFileLocation(State, true, InputRecordingIndex, sizeof(FileName), FileName);
    State->RecordingHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(State->RecordingHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        
        State->InputRecordingIndex = InputRecordingIndex;
        win32_memory_block *Sentinel = &GlobalWin32State.MemorySentinel;
        
        BeginTicketMutex(&GlobalWin32State.MemoryMutex);
        for(win32_memory_block *SourceBlock = Sentinel->Next;
            SourceBlock != Sentinel;
            SourceBlock = SourceBlock->Next)
        {
            if(!(SourceBlock->Block.Flags & PlatformMemory_NotRestored))
            {
                win32_saved_memory_block DestBlock;
                void *BasePointer = SourceBlock->Block.Base;
                DestBlock.BasePointer = (u64)BasePointer;
                DestBlock.Size = SourceBlock->Block.Size;
                WriteFile(State->RecordingHandle, &DestBlock, sizeof(DestBlock), &BytesWritten, 0);
                Assert(DestBlock.Size <= U32Max);
                WriteFile(State->RecordingHandle, BasePointer, (u32)DestBlock.Size, &BytesWritten, 0);
            }
        }
        EndTicketMutex(&GlobalWin32State.MemoryMutex);
        
        win32_saved_memory_block DestBlock = {};
        WriteFile(State->RecordingHandle, &DestBlock, sizeof(DestBlock), &BytesWritten, 0);
    }
}

internal void
Win32EndRecordingInput(win32_state *State)
{
    CloseHandle(State->RecordingHandle);
    State->InputRecordingIndex = 0;
}

internal void
Win32FreeMemoryBlock(win32_memory_block *Block)
{
    BeginTicketMutex(&GlobalWin32State.MemoryMutex);
    Block->Prev->Next = Block->Next;
    Block->Next->Prev = Block->Prev;
    EndTicketMutex(&GlobalWin32State.MemoryMutex);
    
    // NOTE(casey): For porting to other platforms that need the size to unmap
    // pages, you can get it from Block->Block.Size!
    
    BOOL Result = VirtualFree(Block, 0, MEM_RELEASE);
    Assert(Result);
}

internal void
Win32ClearBlocksByMask(win32_state *State, u64 Mask)
{
    for(win32_memory_block *BlockIter = State->MemorySentinel.Next;
        BlockIter != &State->MemorySentinel;
        )
    {
        win32_memory_block *Block = BlockIter;
        BlockIter = BlockIter->Next;
        
        if((Block->LoopingFlags & Mask) == Mask)
        {
            Win32FreeMemoryBlock(Block);
        }
        else
        {
            Block->LoopingFlags = 0;
        }
    }
}

internal void
Win32BeginInputPlayBack(win32_state *State, int InputPlayingIndex)
{
    Win32ClearBlocksByMask(State, Win32Mem_AllocatedDuringLooping);
    
    char FileName[WIN32_STATE_FILE_NAME_COUNT];
    Win32GetInputFileLocation(State, true, InputPlayingIndex, sizeof(FileName), FileName);
    State->PlaybackHandle = CreateFileA(FileName, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if(State->PlaybackHandle != INVALID_HANDLE_VALUE)
    {
        State->InputPlayingIndex = InputPlayingIndex;
        
        for(;;)
        {
            win32_saved_memory_block Block = {};
            DWORD BytesRead;
            ReadFile(State->PlaybackHandle, &Block, sizeof(Block), &BytesRead, 0);
            if(Block.BasePointer != 0)
            {
                void *BasePointer = (void *)Block.BasePointer;
                Assert(Block.Size <= U32Max);
                ReadFile(State->PlaybackHandle, BasePointer, (u32)Block.Size, &BytesRead, 0);
            }
            else
            {
                break;
            }
        }
        // TODO(casey): Stream memory in from the file!
    }
}

internal void
Win32EndInputPlayBack(win32_state *State)
{
    Win32ClearBlocksByMask(State, Win32Mem_FreedDuringLooping);
    CloseHandle(State->PlaybackHandle);
    State->InputPlayingIndex = 0;
}

internal void
Win32RecordInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesWritten;
    WriteFile(State->RecordingHandle, NewInput, sizeof(*NewInput), &BytesWritten, 0);
}

internal void
Win32PlayBackInput(win32_state *State, game_input *NewInput)
{
    DWORD BytesRead = 0;
    if(ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0))
    {
        if(BytesRead == 0)
        {
            // NOTE(casey): We've hit the end of the stream, go back to the beginning
            int PlayingIndex = State->InputPlayingIndex;
            Win32EndInputPlayBack(State);
            Win32BeginInputPlayBack(State, PlayingIndex);
            ReadFile(State->PlaybackHandle, NewInput, sizeof(*NewInput), &BytesRead, 0);
        }
    }
}

internal void
ToggleFullscreen(HWND Window)
{
    // NOTE(casey): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void
Win32ProcessPendingMessages(win32_state *State, game_controller_input *KeyboardController,
                            game_input *Input)
{
    MSG Message = {};
    for(;;)
    {
        BOOL GotMessage = FALSE;
        
        {
            TIMED_BLOCK("PeekMessage");
            
            // NOTE(casey): These must be in sorted order!
            DWORD SkipMessages[] =
            {
                // NOTE(casey): We were previously skipping WM_PAINT, but it appears
                // that it is generally dispatched directly to the window in almost
                // all cases, so there is no sense generating an additional spurious
                // call to PeekMessage to skip a message that (almost?) never goes
                // through the main thread message loop.
                //                WM_PAINT,
                
                // NOTE(casey): We observed bad behavior when skipping WM_MOUSEMOVE (at
                // least on Windows 10), so it _cannot_ be skipped or queued mouse movements
                // at rapid speed seem to create a scenario where PeekMessage will take in
                // the tens of millions of cycles just to return that it _doesn't_ have
                // a message!  (Or more specifically, 4 calls to PeekMessage were taking
                // tens of millions of cycles, on the order of 5 million cycles per call).
                //                WM_MOUSEMOVE,
                
                // TODO(casey): Why are we getting 0x738 (1848) sent to us?  This is a WM_USER
                // message!
                0x738,
                0xFFFFFFFF,
            };
            DWORD LastMessage = 0;
            for(u32 SkipIndex = 0;
                SkipIndex < ArrayCount(SkipMessages);
                ++SkipIndex)
            {
                
                DWORD Skip = SkipMessages[SkipIndex];
                GotMessage = PeekMessage(&Message, 0, LastMessage, Skip - 1, PM_REMOVE);
                if(GotMessage)
                {
                    break;
                }
                
                LastMessage = Skip + 1;
            }
        }
        
        if(!GotMessage)
        {
            break;
        }
        
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                uint32 VKCode = (uint32)Message.wParam;
                
                bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
                bool32 ShiftKeyWasDown = (GetKeyState(VK_SHIFT) & (1 << 15));
                
                // NOTE(casey): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool32 IsDown = ((Message.lParam & (1UL << 31)) == 0);
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }
                    else if(VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    else if(VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    else if(VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    else if(VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    }
                    else if(VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    }
                    else if(VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                    }
                    else if(VKCode == VK_RETURN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }
                    else if((VKCode == VK_SPACE) ||
                            (VKCode == VK_SHIFT))
                    {
                        b32 EitherDown = ((GetKeyState(VK_SPACE) & (1 << 15)) ||
                                          (GetKeyState(VK_SHIFT) & (1 << 15)));
                        KeyboardController->ClutchMax = (EitherDown ? 1.0f : 0.0f);
                    }
#if HANDMADE_INTERNAL
                    else if(VKCode == 'P')
                    {
                        if(IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
                    else if(VKCode == 'L')
                    {
                        if(IsDown)
                        {
                            if(AltKeyWasDown)
                            {
                                Win32BeginInputPlayBack(State, 1);
                            }
                            else
                            {
                                if(State->InputPlayingIndex == 0)
                                {
                                    if(State->InputRecordingIndex == 0)
                                    {
                                        Win32BeginRecordingInput(State, 1);
                                    }
                                    else
                                    {
                                        Win32EndRecordingInput(State);
                                        Win32BeginInputPlayBack(State, 1);
                                    }
                                }
                                else
                                {
                                    Win32EndInputPlayBack(State);
                                }
                            }
                        }
                    }
#endif
                    if(IsDown)
                    {
                        if((VKCode == VK_F4) && AltKeyWasDown)
                        {
                            GlobalRunning = false;
                        }
                        else if((VKCode == VK_RETURN) && AltKeyWasDown)
                        {
                            if(Message.hwnd)
                            {
                                ToggleFullscreen(Message.hwnd);
                            }
                        }
                        else if((VKCode >= VK_F1) && (VKCode <= VK_F12))
                        {
                            Input->FKeyPressed[VKCode - VK_F1 + 1] = true;
                        }
                    }
                }
                
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) /
                     (real32)GlobalPerfCountFrequency);
    return(Result);
}

internal void
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal bool32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;
    
    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead,
                                                  NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {
            platform_work_queue_entry Entry = Queue->Entries[Index];
            // TODO(casey): If we were serious about threading, there would be a thread-specific
            // struct that contained the scratch arena for the thread that it can use
            // for all its temporary work.
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }
    
    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }
    
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

DWORD WINAPI
ThreadProc(LPVOID lpParameter)
{
    win32_thread_startup *Thread = (win32_thread_startup *)lpParameter;
    platform_work_queue *Queue = Thread->Queue;
    
    u32 TestThreadID = GetThreadID();
    Assert(TestThreadID == GetCurrentThreadId());
    
    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(Queue))
        {
            WaitForSingleObjectEx(Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }
    
    //    return(0);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buffer);
}

internal void
Win32MakeQueue(platform_work_queue *Queue, uint32 ThreadCount, win32_thread_startup *Startups)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;
    
    uint32 InitialCount = 0;
    Queue->SemaphoreHandle = CreateSemaphoreEx(0,
                                               InitialCount,
                                               ThreadCount,
                                               0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        win32_thread_startup *Startup = Startups + ThreadIndex;
        Startup->Queue = Queue;
        
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, Megabytes(1), ThreadProc, Startup, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

struct win32_platform_file_group
{
    memory_arena Memory;
};

internal char *
UTF8FromUTF16(memory_arena *Arena, u32 NameSize, wchar_t *Name)
{
    u32 ResultStorage = (u32)(4*NameSize);
    char *Result = (char *)PushSize(Arena, ResultStorage + 1);
    u32 ResultSize = WideCharToMultiByte(CP_UTF8, 0, Name, NameSize,
                                         Result, ResultStorage, 0, 0);
    Result[ResultSize] = 0;
    return(Result);
}

internal wchar_t *
UTF16FromUTF8(memory_arena *Arena, u32 NameSize, char *Name)
{
    u32 ResultStorage = (u32)(2*NameSize);
    wchar_t *Result = (wchar_t *)PushSize(Arena, ResultStorage + 2);
    u32 ResultSize = MultiByteToWideChar(CP_UTF8, 0, Name, NameSize,
                                         Result, ResultStorage);
    Result[ResultSize] = 0;
    return(Result);
}

internal platform_file_info *
Win32AllocateFileInfo(platform_file_group *FileGroup, WIN32_FILE_ATTRIBUTE_DATA *Data)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    
    platform_file_info *Info = PushStruct(&Win32FileGroup->Memory, platform_file_info);
    Info->Next = FileGroup->FirstFileInfo;
    Info->FileDate = (((u64)Data->ftLastWriteTime.dwHighDateTime << (u64)32) | (u64)Data->ftLastWriteTime.dwLowDateTime);
    Info->FileSize = (((u64)Data->nFileSizeHigh << (u64)32) | (u64)Data->nFileSizeLow);
    FileGroup->FirstFileInfo = Info;
    ++FileGroup->FileCount;
    
    return(Info);
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(Win32GetAllFilesOfTypeBegin)
{
    platform_file_group Result = {};
    
    win32_platform_file_group *Win32FileGroup = BootstrapPushStruct(win32_platform_file_group, Memory);
    Result.Platform = Win32FileGroup;
    
    wchar_t *Stem = L"";
    wchar_t *WildCard = L"*.*";
    switch(Type)
    {
        case PlatformFileType_AssetFile:
        {
            Stem = L"data\\";
            WildCard = L"data\\*.hha";
        } break;
        
        case PlatformFileType_SavedGameFile:
        {
            Stem = L"data\\";
            WildCard = L"data\\*.hhs";
        } break;
        
        case PlatformFileType_HHT:
        {
            Stem = L"tags\\";
            WildCard = L"tags\\*.hht";
        } break;
        
        case PlatformFileType_Dump:
        {
            Stem = L"debug\\";
            WildCard = L"debug\\*.dump";
        } break;
        
        InvalidDefaultCase;
    }
    u32 StemSize = 0;
    for(wchar_t *Scan = Stem;
        *Scan;
        ++Scan)
    {
        ++StemSize;
    }
    
    WIN32_FIND_DATAW FindData;
    HANDLE FindHandle = FindFirstFileW(WildCard, &FindData);
    while(FindHandle != INVALID_HANDLE_VALUE)
    {
        platform_file_info *Info = Win32AllocateFileInfo(&Result, (WIN32_FILE_ATTRIBUTE_DATA *)&FindData);
        wchar_t *BaseNameBegin = FindData.cFileName;
        wchar_t *BaseNameEnd = 0;
        wchar_t *Scan = BaseNameBegin;
        while(*Scan)
        {
            if(Scan[0] == L'.')
            {
                BaseNameEnd = Scan;
            }
            
            ++Scan;
        }
        if(!BaseNameEnd)
        {
            BaseNameEnd = Scan;
        }
        
        u32 BaseNameSize = (u32)(BaseNameEnd - BaseNameBegin);
        Info->BaseName = UTF8FromUTF16(&Win32FileGroup->Memory, BaseNameSize, BaseNameBegin);
        
        // NOTE(casey): This will not be technically correct if you use Unicode filenames.
        for(char *Lower = Info->BaseName;
            *Lower;
            ++Lower)
        {
            *Lower = ToLowercase(*Lower);
        }
        
        u32 cFileNameSize = (u32)(((Scan - FindData.cFileName) + 1));
        Info->Platform = PushArray(&Win32FileGroup->Memory, StemSize + cFileNameSize, wchar_t);
        CopyArray(StemSize, Stem, Info->Platform);
        CopyArray(cFileNameSize, FindData.cFileName, (wchar_t *)Info->Platform + StemSize);
        
        if(!FindNextFileW(FindHandle, &FindData))
        {
            break;
        }
    }
    FindClose(FindHandle);
    
    return(Result);
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_END(Win32GetAllFilesOfTypeEnd)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    if(Win32FileGroup)
    {
        Clear(&Win32FileGroup->Memory);
    }
}

internal PLATFORM_GET_FILE_BY_PATH(Win32GetFileByPath)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    platform_file_info *Result = 0;
    
    wchar_t *Path16 = UTF16FromUTF8(&Win32FileGroup->Memory, StringLength(Path), Path);
    
    WIN32_FILE_ATTRIBUTE_DATA Data = {};
    if(GetFileAttributesExW(Path16, GetFileExInfoStandard, &Data) ||
       (ModeFlags & OpenFile_Write))
    {
        Result = Win32AllocateFileInfo(FileGroup, &Data);
        // TODO(casey): Should the base name be duplicated??
        Result->BaseName = Path;
        Result->Platform = Path16;
    }
    
    return(Result);
}

internal PLATFORM_FILE_ERROR(Win32FileError)
{
#if HANDMADE_INTERNAL
    OutputDebugString("WIN32 FILE ERROR: ");
    OutputDebugString(Message);
    OutputDebugString("\n");
#endif
    
    Handle->NoErrors = false;
}

internal PLATFORM_OPEN_FILE(Win32OpenFile)
{
    platform_file_handle Result = {};
    Assert(sizeof(HANDLE) <= sizeof(Result.Platform));
    
    DWORD HandlePermissions = 0;
    DWORD HandleCreation = 0;
    if(ModeFlags & OpenFile_Read)
    {
        HandlePermissions |= GENERIC_READ;
        HandleCreation = OPEN_EXISTING;
    }
    
    if(ModeFlags & OpenFile_Write)
    {
        HandlePermissions |= GENERIC_WRITE;
        HandleCreation = OPEN_ALWAYS;
    }
    
    wchar_t *FileName = (wchar_t *)Info->Platform;
    HANDLE Win32Handle = CreateFileW(FileName, HandlePermissions,
                                     FILE_SHARE_READ, 0, HandleCreation, 0, 0);
    Result.NoErrors = (Win32Handle != INVALID_HANDLE_VALUE);
    *(HANDLE *)&Result.Platform = Win32Handle;
    
    return(Result);
}

internal PLATFORM_SET_FILE_SIZE(Win32SetFileSize)
{
    if(PlatformNoFileErrors(Handle))
    {
        HANDLE Win32Handle = *(HANDLE *)&Handle->Platform;
        
        LARGE_INTEGER FilePointer = {};
        FilePointer.QuadPart = Size;
        
        // NOTE(casey): You can only do it this way in a multithreaded scenario
        // if you _don't_ use SetFilePointerEx anywhere else (eg., not in reads
        // and writes), which we don't.
        if(SetFilePointerEx(Win32Handle, FilePointer, 0, FILE_BEGIN) &&
           SetEndOfFile(Win32Handle))
        {
            Win32FileError(Handle, "Unable to set the end of the file.");
        }
    }
}

internal PLATFORM_READ_DATA_FROM_FILE(Win32ReadDataFromFile)
{
    if(PlatformNoFileErrors(Handle))
    {
        HANDLE Win32Handle = *(HANDLE *)&Handle->Platform;
        
        OVERLAPPED Overlapped = {};
        Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
        Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
        
        uint32 FileSize32 = SafeTruncateToU32(Size);
        
        DWORD BytesRead;
        if(ReadFile(Win32Handle, Dest, FileSize32, &BytesRead, &Overlapped) &&
           (FileSize32 == BytesRead))
        {
            // NOTE(casey): File read succeeded!
        }
        else
        {
            Win32FileError(Handle, "Read file failed.");
        }
    }
}

internal PLATFORM_WRITE_DATA_TO_FILE(Win32WriteDataToFile)
{
    if(PlatformNoFileErrors(Handle))
    {
        HANDLE Win32Handle = *(HANDLE *)&Handle->Platform;
        
        OVERLAPPED Overlapped = {};
        Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
        Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
        
        uint32 FileSize32 = SafeTruncateToU32(Size);
        
        DWORD BytesWritten;
        if(WriteFile(Win32Handle, Source, FileSize32, &BytesWritten, &Overlapped) &&
           (FileSize32 == BytesWritten))
        {
            // NOTE(casey): File read succeeded!
        }
        else
        {
            Win32FileError(Handle, "Write file failed.");
        }
    }
}

internal PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(Win32AtomicReplaceFileContents)
{
    b32 Result = false;
    
    wchar_t *ExistingFileName = (wchar_t *)Info->Platform;
    wchar_t *LastSlash = ExistingFileName;
    for(wchar_t *Scan = ExistingFileName;
        *Scan;
        ++Scan)
    {
        if((*Scan == L'/') ||
           (*Scan == L'\\'))
        {
            LastSlash = Scan;
        }
    }
    
    wchar_t TempFileName[MAX_PATH + 1];
    wchar_t RestoreSlash = *LastSlash;
    *LastSlash = 0;
    UINT TempOK = GetTempFileNameW(ExistingFileName, L"hh_", 0, TempFileName);
    *LastSlash = RestoreSlash;
    
    if(TempOK)
    {
        HANDLE File = CreateFileW(TempFileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if(File != INVALID_HANDLE_VALUE)
        {
            b32x WriteOK = false;
            DWORD BytesWritten = 0;
            if(WriteFile(File, Source, SafeTruncateToU32(Size), &BytesWritten, 0))
            {
                WriteOK = (BytesWritten == Size);
            }
            CloseHandle(File);
            
            if(WriteOK)
            {
                Result = MoveFileExW(TempFileName, ExistingFileName, MOVEFILE_REPLACE_EXISTING);
            }
        }
        
        if(!Result)
        {
            DeleteFileW(TempFileName);
        }
    }
    
    return(Result);
}

internal PLATFORM_CLOSE_FILE(Win32CloseFile)
{
    HANDLE Win32Handle = *(HANDLE *)&Handle->Platform;
    if(Win32Handle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(Win32Handle);
    }
}

inline b32x
Win32IsInLoop(win32_state *State)
{
    b32x Result = ((State->InputRecordingIndex != 0) ||
                   (State->InputPlayingIndex));
    return(Result);
}

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory)
{
    // NOTE(casey): We require memory block headers not to change the cache
    // line alignment of an allocation
    Assert(sizeof(win32_memory_block) == 64);
    
    umm PageSize = 4096; // TODO(casey): Query from system?
    umm TotalSize = Size + sizeof(win32_memory_block);
    umm BaseOffset = sizeof(win32_memory_block);
    umm ProtectOffset = 0;
    if(Flags & PlatformMemory_UnderflowCheck)
    {
        TotalSize = Size + 2*PageSize;
        BaseOffset = 2*PageSize;
        ProtectOffset = PageSize;
    }
    else if(Flags & PlatformMemory_OverflowCheck)
    {
        umm SizeRoundedUp = AlignPow2(Size, PageSize);
        TotalSize = SizeRoundedUp + 2*PageSize;
        BaseOffset = PageSize + SizeRoundedUp - Size;
        ProtectOffset = PageSize + SizeRoundedUp;
    }
    
    win32_memory_block *Block = (win32_memory_block *)
        VirtualAlloc(0, TotalSize,
                     MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Assert(Block);
    Block->Block.Base = (u8 *)Block + BaseOffset;
    Assert(Block->Block.Used == 0);
    Assert(Block->Block.ArenaPrev == 0);
    
    if(Flags & (PlatformMemory_UnderflowCheck|PlatformMemory_OverflowCheck))
    {
        DWORD OldProtect = 0;
        BOOL Protected = VirtualProtect((u8 *)Block + ProtectOffset, PageSize, PAGE_NOACCESS, &OldProtect);
        Assert(Protected);
    }
    
    win32_memory_block *Sentinel = &GlobalWin32State.MemorySentinel;
    Block->Next = Sentinel;
    Block->Block.Size = Size;
    Block->Block.Flags = Flags;
    Block->LoopingFlags = 0;
    if(Win32IsInLoop(&GlobalWin32State) && !(Flags & PlatformMemory_NotRestored))
    {
        Block->LoopingFlags = Win32Mem_AllocatedDuringLooping;
    }
    
    BeginTicketMutex(&GlobalWin32State.MemoryMutex);
    Block->Prev = Sentinel->Prev;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    EndTicketMutex(&GlobalWin32State.MemoryMutex);
    
    platform_memory_block *PlatBlock = &Block->Block;
    return(PlatBlock);
}

PLATFORM_DEALLOCATE_MEMORY(Win32DeallocateMemory)
{
    if(Block)
    {
        win32_memory_block *Win32Block = ((win32_memory_block *)Block);
        if(Win32IsInLoop(&GlobalWin32State) && !(Win32Block->Block.Flags & PlatformMemory_NotRestored))
        {
            Win32Block->LoopingFlags = Win32Mem_FreedDuringLooping;
        }
        else
        {
            Win32FreeMemoryBlock(Win32Block);
        }
    }
}

PLATFORM_ERROR_MESSAGE(Win32ErrorMessage)
{
    char *Caption = "Handmade Hero Warning";
    
    UINT MBoxType = MB_OK;
    if(Type == PlatformError_Fatal)
    {
        Caption = "Handmade Hero Fatal Error";
        MBoxType |= MB_ICONSTOP;
    }
    else
    {
        MBoxType |= MB_ICONWARNING;
    }
    
    MessageBoxExA(GlobalWin32State.DefaultWindowHandle, Message, Caption, MBoxType, 0);
    
    if(Type == PlatformError_Fatal)
    {
        ExitProcess(1);
    }
}

#if HANDMADE_INTERNAL
global debug_table GlobalDebugTable_;
debug_table *GlobalDebugTable = &GlobalDebugTable_;
#endif

internal void
Win32FullRestart(char *SourceEXE, char *DestEXE, char *DeleteEXE)
{
    DeleteFile(DeleteEXE);
    if(MoveFile(DestEXE, DeleteEXE))
    {
        if(MoveFile(SourceEXE, DestEXE))
        {
            STARTUPINFO StartupInfo = {};
            StartupInfo.cb = sizeof(StartupInfo);
            PROCESS_INFORMATION ProcessInfo = {};
            if(CreateProcess(DestEXE,
                             GetCommandLine(),
                             0,
                             0,
                             FALSE,
                             0,
                             0,
                             "w:\\handmade\\data\\",
                             &StartupInfo,
                             &ProcessInfo))
            {
                CloseHandle(ProcessInfo.hProcess);
            }
            else
            {
                // TODO(casey): Error!
            }
            
            ExitProcess(0);
        }
    }
}

internal u32 Win32GetEntropy(void)
{
    u32 Result = 1234;
    BCryptGenRandom(0, (u8 *)&Result, sizeof(Result), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    return Result;
}

extern "C" int __stdcall WinMainCRTStartup()
{
    SetDefaultFPBehavior();
    
    HINSTANCE Instance = GetModuleHandle(0);
    
    DEBUGSetEventRecording(true);
    
    win32_state *State = &GlobalWin32State;
    State->MemorySentinel.Prev = &State->MemorySentinel;
    State->MemorySentinel.Next = &State->MemorySentinel;
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    Win32GetEXEFileName(State);
    
    char Win32EXEFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(State, "win32_handmade.exe",
                              sizeof(Win32EXEFullPath), Win32EXEFullPath);
    
    char TempWin32EXEFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(State, "win32_handmade_temp.exe",
                              sizeof(TempWin32EXEFullPath), TempWin32EXEFullPath);
    
    char DeleteWin32EXEFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(State, "win32_handmade_old.exe",
                              sizeof(DeleteWin32EXEFullPath), DeleteWin32EXEFullPath);
    
    char SourceGameCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(State, "handmade.dll",
                              sizeof(SourceGameCodeDLLFullPath), SourceGameCodeDLLFullPath);
    
    char RendererCodeDLLFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(State, "win32_handmade_opengl.dll",
                              sizeof(RendererCodeDLLFullPath), RendererCodeDLLFullPath);
    
    
    char CodeLockFullPath[WIN32_STATE_FILE_NAME_COUNT];
    Win32BuildEXEPathFileName(State, "lock.tmp",
                              sizeof(CodeLockFullPath), CodeLockFullPath);
    
    u32 Entropy = Win32GetEntropy();
    
    // NOTE(casey): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    Win32LoadXInput();
    
#if HANDMADE_INTERNAL
    DEBUGGlobalShowCursor = true;
#endif
    WNDCLASSA WindowClass = {};
    
    WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    //    WindowClass.hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
            0, // WS_EX_TOPMOST|WS_EX_LAYERED,
            WindowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW,
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
            State->DefaultWindowHandle = Window;
            
#if !HANDMADE_INTERNAL
            ToggleFullscreen(Window);
#endif
            HDC RendererDC = GetDC(Window);
            
            // NOTE(casey): We need to initialize OpenGL so that it can render to our
            // window.  The Win32 startup code is contained within win32_handmade_opengl.cpp,
            // so we get the DC for our window and pass that to its Win32InitOpenGL call
            // so it can do all the startup for us.
            u32 MaxQuadCountPerFrame = (1 << 20);
            // forward to the asset system
            platform_renderer_limits Limits = {};
            Limits.MaxQuadCountPerFrame = MaxQuadCountPerFrame;
            Limits.MaxTextureCount = HANDMADE_NORMAL_TEXTURE_COUNT;
            Limits.MaxSpecialTextureCount = HANDMADE_SPECIAL_TEXTURE_COUNT;
            Limits.TextureTransferBufferSize = HANDMADE_TEXTURE_TRANSFER_BUFFER_SIZE;
            
            win32_renderer_function_table RendererFunctions = {};
            win32_loaded_code RendererCode = {};
            RendererCode.TransientDLLName = "handmade_renderer_temp.dll";
            RendererCode.DLLFullPath = RendererCodeDLLFullPath;
            RendererCode.LockFullPath = CodeLockFullPath;
            RendererCode.FunctionCount = ArrayCount(Win32RendererFunctionTableNames);
            RendererCode.FunctionNames = Win32RendererFunctionTableNames;
            RendererCode.Functions = (void **)&RendererFunctions;
            
            Win32LoadCode(State, &RendererCode);
            if(!RendererCode.IsValid)
            {
                Win32ErrorMessage(PlatformError_Fatal, "Unable to load renderer");
            }
            
            platform_renderer *Renderer = RendererFunctions.LoadRenderer(GetDC(Window), &Limits);
            
            win32_thread_startup HighPriStartups[6] = {};
            platform_work_queue HighPriorityQueue = {};
            Win32MakeQueue(&HighPriorityQueue, ArrayCount(HighPriStartups), HighPriStartups);
            
            win32_thread_startup LowPriStartups[2] = {};
            platform_work_queue LowPriorityQueue = {};
            Win32MakeQueue(&LowPriorityQueue, ArrayCount(LowPriStartups), LowPriStartups);
            
            win32_sound_output SoundOutput = {};
            
            // TODO(casey): How do we reliably query on this on Windows?
            int MonitorRefreshHz = 60;
            int Win32RefreshRate = GetDeviceCaps(RendererDC, VREFRESH);
            if(Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            f32 GameUpdateHz = (f32)(MonitorRefreshHz);
            
            // TODO(casey): Make this like sixty seconds?
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(int16)*2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample;
            // TODO(casey): Actually compute this variance and see
            // what the lowest reasonable value is.
            SoundOutput.SafetyBytes = (int)(((real32)SoundOutput.SamplesPerSecond*(real32)SoundOutput.BytesPerSample / GameUpdateHz)/3.0f);
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            
            GlobalRunning = true;
            
            // TODO(casey): Let's make this our first growable arena!
            memory_arena FrameTempArena = {};
            
            // TODO(casey): Pool with bitmap VirtualAlloc
            // TODO(casey): Remove MaxPossibleOverrun?
            u32 MaxPossibleOverrun = 2*8*sizeof(u16);
            int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize + MaxPossibleOverrun,
                                                   MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            
#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif
            
            game_memory GameMemory = {};
#if HANDMADE_INTERNAL
            GameMemory.DebugTable = GlobalDebugTable;
#endif
            GameMemory.HighPriorityQueue = &HighPriorityQueue;
            GameMemory.LowPriorityQueue = &LowPriorityQueue;
            GameMemory.PlatformAPI.AddEntry = Win32AddEntry;
            GameMemory.PlatformAPI.CompleteAllWork = Win32CompleteAllWork;
            
            GameMemory.PlatformAPI.GetAllFilesOfTypeBegin = Win32GetAllFilesOfTypeBegin;
            GameMemory.PlatformAPI.GetAllFilesOfTypeEnd = Win32GetAllFilesOfTypeEnd;
            GameMemory.PlatformAPI.OpenFile = Win32OpenFile;
            GameMemory.PlatformAPI.SetFileSize = Win32SetFileSize;
            GameMemory.PlatformAPI.GetFileByPath = Win32GetFileByPath;
            GameMemory.PlatformAPI.ReadDataFromFile = Win32ReadDataFromFile;
            GameMemory.PlatformAPI.WriteDataToFile = Win32WriteDataToFile;
            GameMemory.PlatformAPI.AtomicReplaceFileContents = Win32AtomicReplaceFileContents;
            GameMemory.PlatformAPI.FileError = Win32FileError;
            GameMemory.PlatformAPI.CloseFile = Win32CloseFile;
            
            GameMemory.PlatformAPI.AllocateMemory = Win32AllocateMemory;
            GameMemory.PlatformAPI.DeallocateMemory = Win32DeallocateMemory;
            GameMemory.PlatformAPI.ErrorMessage = Win32ErrorMessage;
            
#if HANDMADE_INTERNAL
            GameMemory.PlatformAPI.DEBUGExecuteSystemCommand = DEBUGExecuteSystemCommand;
            GameMemory.PlatformAPI.DEBUGGetProcessState = DEBUGGetProcessState;
            GameMemory.PlatformAPI.DEBUGGetMemoryStats = Win32GetMemoryStats;
#endif
            
            GameMemory.TextureQueue = &Renderer->TextureQueue;
            Platform = GameMemory.PlatformAPI;
            
            if(Samples)
            {
                // TODO(casey): Monitor XBox controllers for being plugged in after
                // the fact!
                b32 XBoxControllerPresent[XUSER_MAX_COUNT] = {};
                for(u32 ControllerIndex = 0;
                    ControllerIndex < XUSER_MAX_COUNT;
                    ++ControllerIndex)
                {
                    XBoxControllerPresent[ControllerIndex] = true;
                }
                
                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];
                
                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();
                
                int DebugTimeMarkerIndex = 0;
                win32_debug_time_marker DebugTimeMarkers[30] = {0};
                
                DWORD AudioLatencyBytes = 0;
                real32 AudioLatencySeconds = 0;
                bool32 SoundIsValid = false;
                
                win32_game_function_table Game = {};
                win32_loaded_code GameCode = {};
                GameCode.TransientDLLName = "handmade_game_temp.dll";
                GameCode.DLLFullPath = SourceGameCodeDLLFullPath;
                GameCode.LockFullPath = CodeLockFullPath;
                GameCode.FunctionCount = ArrayCount(Win32GameFunctionTableNames);
                GameCode.FunctionNames = Win32GameFunctionTableNames;
                GameCode.Functions = (void **)&Game;
                
                Win32LoadCode(State, &GameCode);
                
                DEBUGSetEventRecording(GameCode.IsValid);
                
                ShowWindow(Window, SW_SHOW);
                b32x RendererWasReloaded = false;
                u32 ExpectedFramesPerUpdate = 1;
                f32 TargetSecondsPerFrame = (f32)ExpectedFramesPerUpdate / (f32)GameUpdateHz;
                while(GlobalRunning)
                {
                    {DEBUG_DATA_BLOCK("Platform");
                        DEBUG_VALUE(ExpectedFramesPerUpdate);
                    }
                    {DEBUG_DATA_BLOCK("Platform/Controls");
                        DEBUG_B32(GlobalPause);
                        DEBUG_B32(GlobalSoftwareRendering);
                    }
                    
                    //
                    //
                    //
                    
                    NewInput->dtForFrame = TargetSecondsPerFrame;
                    
                    //
                    //
                    //
                    
                    BEGIN_BLOCK("Input Processing");
                    
                    /* NOTE(casey): 1080p display mode is 1920x1080 -> Half of that is 960x540
                       1920 -> 2048 = 2048-1920 -> 128 pixels
                       1080 -> 2048 = 2048-1080 -> pixels 968
                       1024 + 128 = 1152
                    */
                    v2u RenderDim =
                    {
                        // 192, 108,
                        // 480, 270,
                        // 960, 540,
                        // 1280, 720,
                        // 1279, 719,
                        
                        1920, 1080
                    };
                    
                    v2u Dimension = Win32GetWindowDimension(Window);
                    rectangle2i DrawRegion = AspectRatioFit(RenderDim.Width,
                                                            RenderDim.Height,
                                                            Dimension.Width, Dimension.Height);
                    
                    game_render_commands *Frame = 0;
                    if(RendererCode.IsValid)
                    {
                        Frame = RendererFunctions.BeginFrame(
                            Renderer, Dimension, RenderDim, DrawRegion);
                    }
                    
                    // TODO(casey): Zeroing macro
                    // TODO(casey): We can't zero everything because the up/down state will
                    // be wrong!!!
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    *NewKeyboardController = {};
                    NewKeyboardController->IsConnected = true;
                    for(int ButtonIndex = 0;
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                        ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }
                    NewKeyboardController->ClutchMax = OldKeyboardController->ClutchMax;
                    
                    {
                        TIMED_BLOCK("Win32 Message Processing");
                        ZeroStruct(NewInput->FKeyPressed);
                        Win32ProcessPendingMessages(State, NewKeyboardController,
                                                    NewInput);
                    }
                    
                    if(!GlobalPause)
                    {
                        {
                            TIMED_BLOCK("Mouse Position");
                            POINT MouseP;
                            GetCursorPos(&MouseP);
                            ScreenToClient(Window, &MouseP);
                            f32 MouseX = (r32)MouseP.x;
                            f32 MouseY = (r32)((Dimension.Height - 1) - MouseP.y);
                            
                            NewInput->ClipSpaceMouseP.x =
                                ClampBinormalMapToRange((f32)DrawRegion.Min.x, MouseX, (f32)DrawRegion.Max.x);
                            NewInput->ClipSpaceMouseP.y =
                                ClampBinormalMapToRange((f32)DrawRegion.Min.y, MouseY, (f32)DrawRegion.Max.y);
                            NewInput->ClipSpaceMouseP.z = 0; // TODO(casey): Support mousewheel?
                            
                            NewInput->ShiftDown = (GetKeyState(VK_SHIFT) & (1 << 15));
                            NewInput->AltDown = (GetKeyState(VK_MENU) & (1 << 15));
                            NewInput->ControlDown = (GetKeyState(VK_CONTROL) & (1 << 15));
                        }
                        
                        {
                            TIMED_BLOCK("Mousebutton Processing");
                            DWORD WinButtonID[PlatformMouseButton_Count] =
                            {
                                VK_LBUTTON,
                                VK_MBUTTON,
                                VK_RBUTTON,
                                VK_XBUTTON1,
                                VK_XBUTTON2,
                            };
                            for(u32 ButtonIndex = 0;
                                ButtonIndex < PlatformMouseButton_Count;
                                ++ButtonIndex)
                            {
                                NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
                                NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
                                Win32ProcessKeyboardMessage(&NewInput->MouseButtons[ButtonIndex],
                                                            GetKeyState(WinButtonID[ButtonIndex]) & (1 << 15));
                            }
                        }
                        
                        {
                            TIMED_BLOCK("XBox Controllers");
                            
                            // TODO(casey): Need to not poll disconnected controllers to avoid
                            // xinput frame rate hit on older libraries...
                            // TODO(casey): Should we poll this more frequently
                            DWORD MaxControllerCount = XUSER_MAX_COUNT;
                            if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
                            {
                                MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
                            }
                            
                            for (DWORD ControllerIndex = 0;
                                 ControllerIndex < MaxControllerCount;
                                 ++ControllerIndex)
                            {
                                DWORD OurControllerIndex = ControllerIndex + 1;
                                game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
                                game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
                                
                                XINPUT_STATE ControllerState;
                                if(XBoxControllerPresent[ControllerIndex] &&
                                   XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                                {
                                    NewController->IsConnected = true;
                                    NewController->IsAnalog = OldController->IsAnalog;
                                    
                                    // NOTE(casey): This controller is plugged in
                                    // TODO(casey): See if ControllerState.dwPacketNumber increments too rapidly
                                    XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
                                    
                                    BYTE TriggerMax = Pad->bLeftTrigger;
                                    if(TriggerMax < Pad->bRightTrigger)
                                    {
                                        TriggerMax = Pad->bRightTrigger;
                                    }
                                    
                                    NewController->ClutchMax = ((f32)TriggerMax / 255.0f);
                                    
                                    // TODO(casey): This is a square deadzone, check XInput to
                                    // verify that the deadzone is "round" and show how to do
                                    // round deadzone processing.
                                    NewController->StickAverageX = Win32ProcessXInputStickValue(
                                        Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                    NewController->StickAverageY = Win32ProcessXInputStickValue(
                                        Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                    if((NewController->StickAverageX != 0.0f) ||
                                       (NewController->StickAverageY != 0.0f))
                                    {
                                        NewController->IsAnalog = true;
                                    }
                                    
                                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                                    {
                                        NewController->StickAverageY = 1.0f;
                                        NewController->IsAnalog = false;
                                    }
                                    
                                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                                    {
                                        NewController->StickAverageY = -1.0f;
                                        NewController->IsAnalog = false;
                                    }
                                    
                                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                                    {
                                        NewController->StickAverageX = -1.0f;
                                        NewController->IsAnalog = false;
                                    }
                                    
                                    if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                                    {
                                        NewController->StickAverageX = 1.0f;
                                        NewController->IsAnalog = false;
                                    }
                                    
                                    real32 Threshold = 0.5f;
                                    Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageX < -Threshold) ? 1 : 0,
                                        &OldController->MoveLeft, 1,
                                        &NewController->MoveLeft);
                                    Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageX > Threshold) ? 1 : 0,
                                        &OldController->MoveRight, 1,
                                        &NewController->MoveRight);
                                    Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageY < -Threshold) ? 1 : 0,
                                        &OldController->MoveDown, 1,
                                        &NewController->MoveDown);
                                    Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageY > Threshold) ? 1 : 0,
                                        &OldController->MoveUp, 1,
                                        &NewController->MoveUp);
                                    
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->ActionDown, XINPUT_GAMEPAD_A,
                                                                    &NewController->ActionDown);
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->ActionRight, XINPUT_GAMEPAD_B,
                                                                    &NewController->ActionRight);
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->ActionLeft, XINPUT_GAMEPAD_X,
                                                                    &NewController->ActionLeft);
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->ActionUp, XINPUT_GAMEPAD_Y,
                                                                    &NewController->ActionUp);
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->LeftShoulder, XINPUT_GAMEPAD_LEFT_SHOULDER,
                                                                    &NewController->LeftShoulder);
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                                                    &NewController->RightShoulder);
                                    
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->Start, XINPUT_GAMEPAD_START,
                                                                    &NewController->Start);
                                    Win32ProcessXInputDigitalButton(Pad->wButtons,
                                                                    &OldController->Back, XINPUT_GAMEPAD_BACK,
                                                                    &NewController->Back);
                                }
                                else
                                {
                                    // NOTE(casey): The controller is not available
                                    NewController->IsConnected = false;
                                    XBoxControllerPresent[ControllerIndex] = false;
                                }
                            }
                        }
                    }
                    END_BLOCK();
                    
                    //
                    //
                    //
                    
                    BEGIN_BLOCK("Game Update");
                    
                    if(Frame && !GlobalPause)
                    {
                        if(State->InputRecordingIndex)
                        {
                            Win32RecordInput(State, NewInput);
                        }
                        
                        if(State->InputPlayingIndex)
                        {
                            game_input Temp = *NewInput;
                            Win32PlayBackInput(State, NewInput);
                            for(u32 MouseButtonIndex = 0;
                                MouseButtonIndex < PlatformMouseButton_Count;
                                ++MouseButtonIndex)
                            {
                                NewInput->MouseButtons[MouseButtonIndex] = Temp.MouseButtons[MouseButtonIndex];
                            }
                            NewInput->ClipSpaceMouseP = Temp.ClipSpaceMouseP;
                        }
                        if(Game.UpdateAndRender)
                        {
                            NewInput->Entropy = Entropy;
                            
                            Game.UpdateAndRender(&GameMemory, NewInput, Frame);
                            if(NewInput->QuitRequested)
                            {
                                GlobalRunning = false;
                            }
                            if(NewInput->EntropyRequested)
                            {
                                Entropy = Win32GetEntropy();;
                            }
                        }
                    }
                    
                    END_BLOCK();
                    
                    //
                    //
                    //
                    
                    BEGIN_BLOCK("Audio Update");
                    
                    if(!GlobalPause)
                    {
                        LARGE_INTEGER AudioWallClock = Win32GetWallClock();
                        real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);
                        
                        DWORD PlayCursor;
                        DWORD WriteCursor;
                        if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
                        {
                            /* NOTE(casey):
                            
                               Here is how sound output computation works.
                               
                               We define a safety value that is the number
                               of samples we think our game update loop
                               may vary by (let's say up to 2ms)
                               
                               When we wake up to write audio, we will look
                               and see what the play cursor position is and we
                               will forecast ahead where we think the play
                               cursor will be on the next frame boundary.
                               
                               We will then look to see if the write cursor is
                               before that by at least our safety value.  If
                               it is, the target fill position is that frame
                               boundary plus one frame.  This gives us perfect
                               audio sync in the case of a card that has low
                               enough latency.
                               
                               If the write cursor is _after_ that safety
                               margin, then we assume we can never sync the
                               audio perfectly, so we will write one frame's
                               worth of audio plus the safety margin's worth
                               of guard samples.
                            */
                            if(!SoundIsValid)
                            {
                                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                SoundIsValid = true;
                            }
                            
                            DWORD ByteToLock = ((SoundOutput.RunningSampleIndex*SoundOutput.BytesPerSample) %
                                                SoundOutput.SecondaryBufferSize);
                            
                            DWORD ExpectedSoundBytesPerFrame =
                                (int)((real32)(SoundOutput.SamplesPerSecond*SoundOutput.BytesPerSample) /
                                      GameUpdateHz);
                            real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
                            DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame)*(real32)ExpectedSoundBytesPerFrame);
                            
                            DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
                            
                            DWORD SafeWriteCursor = WriteCursor;
                            if(SafeWriteCursor < PlayCursor)
                            {
                                SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            Assert(SafeWriteCursor >= PlayCursor);
                            SafeWriteCursor += SoundOutput.SafetyBytes;
                            
                            bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);
                            
                            DWORD TargetCursor = 0;
                            if(AudioCardIsLowLatency)
                            {
                                TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
                            }
                            else
                            {
                                TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame +
                                                SoundOutput.SafetyBytes);
                            }
                            TargetCursor = (TargetCursor % SoundOutput.SecondaryBufferSize);
                            
                            DWORD BytesToWrite = 0;
                            if(ByteToLock > TargetCursor)
                            {
                                BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                                BytesToWrite += TargetCursor;
                            }
                            else
                            {
                                BytesToWrite = TargetCursor - ByteToLock;
                            }
                            
                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = Align8(BytesToWrite / SoundOutput.BytesPerSample);
                            BytesToWrite = SoundBuffer.SampleCount*SoundOutput.BytesPerSample;
                            SoundBuffer.Samples = Samples;
                            if(Game.GetSoundSamples)
                            {
                                Game.GetSoundSamples(&GameMemory, &SoundBuffer);
                            }
                            
#if HANDMADE_INTERNAL
                            win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                            Marker->OutputPlayCursor = PlayCursor;
                            Marker->OutputWriteCursor = WriteCursor;
                            Marker->OutputLocation = ByteToLock;
                            Marker->OutputByteCount = BytesToWrite;
                            Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;
                            
                            DWORD UnwrappedWriteCursor = WriteCursor;
                            if(UnwrappedWriteCursor < PlayCursor)
                            {
                                UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                            AudioLatencySeconds =
                                (((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) /
                                 (real32)SoundOutput.SamplesPerSecond);
                            
#endif
                            Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                        }
                        else
                        {
                            SoundIsValid = false;
                        }
                    }
                    
                    END_BLOCK();
                    
                    //
                    // NOTE(casey): Output statistics to the debug view
                    //
                    
                    {DEBUG_DATA_BLOCK("Renderer");
                        DEBUG_VALUE(Renderer->TotalFramebufferMemory);
                        DEBUG_VALUE(Renderer->TotalTextureMemory);
                        DEBUG_VALUE(Renderer->UsedMultisampleCount);
                    }
                    
                    //
                    //
                    //
                    
#if HANDMADE_INTERNAL
                    BEGIN_BLOCK("Debug Collation");
                    
                    b32 ExecutableNeedsToBeReloaded = Win32CheckForCodeChange(&GameCode);
                    
#if 0
                    FILETIME NewEXETime = Win32GetLastWriteTime(TempWin32EXEFullPath);
                    FILETIME OldEXETime = Win32GetLastWriteTime(Win32EXEFullPath);
                    if(Win32TimeIsValid(NewEXETime))
                    {
                        b32 Win32NeedsToBeReloaded =
                            (CompareFileTime(&NewEXETime, &OldEXETime) != 0);
                        // TODO(casey): Compare file contents here
                        if(Win32NeedsToBeReloaded)
                        {
                            Win32FullRestart(TempWin32EXEFullPath, Win32EXEFullPath, DeleteWin32EXEFullPath);
                        }
                    }
#endif
                    
                    GameMemory.ExecutableReloaded = false;
                    if(ExecutableNeedsToBeReloaded)
                    {
                        Win32CompleteAllWork(&HighPriorityQueue);
                        Win32CompleteAllWork(&LowPriorityQueue);
                        DEBUGSetEventRecording(false);
                    }
                    
                    if(Game.DEBUGFrameEnd)
                    {
                        Game.DEBUGFrameEnd(&GameMemory, NewInput, Frame);
                    }
                    
                    if(ExecutableNeedsToBeReloaded)
                    {
                        Win32ReloadCode(State, &GameCode);
                        GameMemory.ExecutableReloaded = true;
                        DEBUGSetEventRecording(GameCode.IsValid);
                    }
                    
                    
                    END_BLOCK();
#endif
                    
                    //
                    //
                    //
                    
                    // TODO(casey): Leave this off until we have actual vblank support?
#if 0
                    BEGIN_BLOCK(FramerateWait);
                    
                    if(!GlobalPause)
                    {
                        LARGE_INTEGER WorkCounter = Win32GetWallClock();
                        real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
                        
                        // TODO(casey): NOT TESTED YET!  PROBABLY BUGGY!!!!!
                        real32 SecondsElapsedForFrame = WorkSecondsElapsed;
                        if(SecondsElapsedForFrame < TargetSecondsPerFrame)
                        {
                            if(SleepIsGranular)
                            {
                                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
                                                                   SecondsElapsedForFrame));
                                if(SleepMS > 0)
                                {
                                    Sleep(SleepMS);
                                }
                            }
                            
                            real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                                       Win32GetWallClock());
                            if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                // TODO(casey): LOG MISSED SLEEP HERE
                            }
                            
                            while(SecondsElapsedForFrame < TargetSecondsPerFrame)
                            {
                                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                                Win32GetWallClock());
                            }
                        }
                        else
                        {
                            // TODO(casey): MISSED FRAME RATE!
                            // TODO(casey): Logging
                        }
                    }
                    
                    END_BLOCK(FramerateWait);
#endif
                    
                    //
                    //
                    //
                    
                    BEGIN_BLOCK("Frame Display");
                    
                    if(RendererCode.IsValid)
                    {
                        if(RendererWasReloaded)
                        {
                            ++Frame->Settings.Version;
                            RendererWasReloaded = false;
                        }
                        RendererFunctions.EndFrame(Renderer, Frame);
                    }
                    
                    if(Win32CheckForCodeChange(&RendererCode))
                    {
                        Win32ReloadCode(State, &RendererCode);
                        RendererWasReloaded = true;
                        
                    }
                    
                    FlipWallClock = Win32GetWallClock();
                    
                    game_input *Temp = NewInput;
                    NewInput = OldInput;
                    OldInput = Temp;
                    // TODO(casey): Should I clear these here?
                    
                    END_BLOCK();
                    
                    LARGE_INTEGER EndCounter = Win32GetWallClock();
                    f32 MeasuredSecondsPerFrame = Win32GetSecondsElapsed(LastCounter, EndCounter);
                    f32 ExactTargetFramesPerUpdate = MeasuredSecondsPerFrame*(f32)MonitorRefreshHz;
                    u32 NewExpectedFramesPerUpdate = RoundReal32ToInt32(ExactTargetFramesPerUpdate);
                    ExpectedFramesPerUpdate = NewExpectedFramesPerUpdate;
                    
                    TargetSecondsPerFrame = MeasuredSecondsPerFrame;
                    
                    FRAME_MARKER(MeasuredSecondsPerFrame);
                    LastCounter = EndCounter;
                }
            }
            else
            {
                Win32ErrorMessage(PlatformError_Fatal, "Unable to allocate memory for sound output.");
            }
        }
        else
        {
            Win32ErrorMessage(PlatformError_Fatal, "Unable to open game window.");
        }
    }
    else
    {
        Win32ErrorMessage(PlatformError_Fatal, "Unable to register game window handle.");
    }
    
    ExitProcess(0);
    
    //return(0);
}
