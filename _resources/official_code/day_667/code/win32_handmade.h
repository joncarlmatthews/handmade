/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

struct win32_sound_output
{
    int SamplesPerSecond;
    uint32 RunningSampleIndex;
    int BytesPerSample;
    DWORD SecondaryBufferSize;
    DWORD SafetyBytes;
    
    // TODO(casey): Should running sample index be in bytes as well
    // TODO(casey): Math gets simpler if we add a "bytes per second" field?
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputByteCount;
    DWORD ExpectedFlipPlayCursor;
    
    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

#define WIN32_LOADED_CODE_ENTRY_POINT(name) b32x name(HMODULE Module, void *FunctionTable)
typedef WIN32_LOADED_CODE_ENTRY_POINT(win32_loaded_code_entry_point);

struct win32_loaded_code
{
    b32x IsValid;
    u32 TempDLLNumber;
    
    char *TransientDLLName;
    char *DLLFullPath;
    char *LockFullPath;
    
    HMODULE DLL;
    FILETIME DLLLastWriteTime;
    
    u32 FunctionCount;
    char **FunctionNames;
    void **Functions;
};

struct win32_game_function_table
{
    // IMPORTANT(casey): The  callbacks can be 0!  You must check before calling.
    // (or check the IsValid boolean in win32_loaded_code)
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;
    debug_game_frame_end *DEBUGFrameEnd;
};
global char *Win32GameFunctionTableNames[] =
{
    "GameUpdateAndRender",
    "GameGetSoundSamples",
    "DEBUGGameFrameEnd",
};

enum win32_fader_state
{
    Win32Fade_FadingIn,
    Win32Fade_WaitingForShow,
    Win32Fade_Inactive,
    Win32Fade_FadingGame,
    Win32Fade_FadingOut,
    Win32Fade_WaitingForClose,
};
struct win32_fader
{
    win32_fader_state State;
    HWND Window;    
    r32 Alpha;
};

enum win32_memory_block_flag
{
    Win32Mem_AllocatedDuringLooping = 0x1,
    Win32Mem_FreedDuringLooping = 0x2,
};
struct win32_memory_block
{
    platform_memory_block Block;
    win32_memory_block *Prev;
    win32_memory_block *Next;
    u64 LoopingFlags;
};

struct win32_saved_memory_block
{
    u64 BasePointer;
    u64 Size;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_state
{
    // NOTE(casey): To touch the memory ring, you must
    // take the memory mutex!
    ticket_mutex MemoryMutex;
    win32_memory_block MemorySentinel;
    
    HANDLE RecordingHandle;
    int InputRecordingIndex;
    
    HANDLE PlaybackHandle;
    int InputPlayingIndex;
    
    char EXEFileName[WIN32_STATE_FILE_NAME_COUNT];
    char *OnePastLastEXEFileNameSlash;
    
    HWND DefaultWindowHandle;
};

struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    uint32 volatile CompletionGoal;
    uint32 volatile CompletionCount;
    
    uint32 volatile NextEntryToWrite;
    uint32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;
    
    platform_work_queue_entry Entries[256];
};

struct win32_thread_startup
{
    platform_work_queue *Queue;
};
