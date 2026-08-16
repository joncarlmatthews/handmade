/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#define counted_pointer(IGNORED)

/*
  NOTE(casey):
  
  HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only
    
  HANDMADE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#ifdef __cplusplus
extern "C" {
#endif
    
#include "handmade_types.h"
    
#pragma pack(push, 1)
    struct bitmap_id
    {
        u32 Value;
    };
    
    struct sound_id
    {
        u32 Value;
    };
    
    struct font_id
    {
        u32 Value;
    };
#pragma pack(pop)
    
    
    
    /*
      NOTE(casey): Services that the platform layer provides to the game
    */
#if HANDMADE_INTERNAL
    /* IMPORTANT(casey):
    
       These are NOT for doing anything in the shipping game - they are
       blocking and the write doesn't protect against lost data!
    */
    typedef struct debug_read_file_result
    {
        uint32 ContentsSize;
        void *Contents;
    } debug_read_file_result;
    
    typedef struct debug_executing_process
    {
        u64 OSHandle;
    } debug_executing_process;
    
    typedef struct debug_process_state
    {
        b32 StartedSuccessfully;
        b32 IsRunning;
        s32 ReturnCode;
    } debug_process_state;
    
    typedef struct debug_platform_memory_stats
    {
        umm BlockCount;
        umm TotalSize; // NOTE(casey): Does not technically include the header
        umm TotalUsed;
    } debug_platform_memory_stats;
    
#define DEBUG_PLATFORM_EXECUTE_SYSTEM_COMMAND(name) debug_executing_process name(char *Path, char *Command, char *CommandLine)
    typedef DEBUG_PLATFORM_EXECUTE_SYSTEM_COMMAND(debug_platform_execute_system_command);
    
#define DEBUG_PLATFORM_GET_MEMORY_STATS(name) debug_platform_memory_stats name(void)
    typedef DEBUG_PLATFORM_GET_MEMORY_STATS(debug_platform_get_memory_stats);
    
    // TODO(casey): Do we want a formal release mechanism here?
#define DEBUG_PLATFORM_GET_PROCESS_STATE(name) debug_process_state name(debug_executing_process Process)
    typedef DEBUG_PLATFORM_GET_PROCESS_STATE(debug_platform_get_process_state);
    
    // TODO(casey): Actually start using this???
    extern struct game_memory *DebugGlobalMemory;
    
#endif
    
    // TODO(casey): Not really sure exactly where these will eventually live.
    // They might want to be dynamic, depending on the graphics memory?
#define HANDMADE_NORMAL_TEXTURE_COUNT 256
#define HANDMADE_SPECIAL_TEXTURE_COUNT 16
#define HANDMADE_TEXTURE_TRANSFER_BUFFER_SIZE (128*1024*1024)
    
    /*
      NOTE(casey): Services that the game provides to the platform layer.
      (this may expand in the future - sound on separate thread, etc.)
    */
    
    // FOUR THINGS - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use
    
    struct manual_sort_key
    {
        u16 AlwaysInFrontOf;
        u16 AlwaysBehind;
    };
    
#define BITMAP_BYTES_PER_PIXEL 4
    typedef struct game_offscreen_buffer
    {
        // NOTE(casey): Pixels are always 32-bits wide, Memory Order BB GG RR XX
        void *Memory;
        int Width;
        int Height;
        int Pitch;
    } game_offscreen_buffer;
    
    typedef struct game_sound_output_buffer
    {
        int SamplesPerSecond;
        int SampleCount;
        
        // IMPORTANT(casey): Samples must be padded to a multiple of 4 samples!
        int16 *Samples;
    } game_sound_output_buffer;
    
    typedef struct game_button_state
    {
        int HalfTransitionCount;
        bool32 EndedDown;
    } game_button_state;
    
    typedef struct game_controller_input
    {
        b32 IsConnected;
        b32 IsAnalog;
        f32 StickAverageX;
        f32 StickAverageY;
        f32 ClutchMax; // NOTE(casey): This is the "dodge" clutch, eg. triggers or space bar?
        
        union
        {
            game_button_state Buttons[12];
            struct
            {
                game_button_state MoveUp;
                game_button_state MoveDown;
                game_button_state MoveLeft;
                game_button_state MoveRight;
                
                game_button_state ActionUp;
                game_button_state ActionDown;
                game_button_state ActionLeft;
                game_button_state ActionRight;
                
                game_button_state LeftShoulder;
                game_button_state RightShoulder;
                
                game_button_state Back;
                game_button_state Start;
                
                // NOTE(casey): All buttons must be added above this line
                
                game_button_state Terminator;
            };
        };
    } game_controller_input;
    
#define MAX_CONTROLLER_COUNT 5
    enum game_input_mouse_button
    {
        PlatformMouseButton_Left,
        PlatformMouseButton_Middle,
        PlatformMouseButton_Right,
        PlatformMouseButton_Extended0,
        PlatformMouseButton_Extended1,
        
        PlatformMouseButton_Count,
    };
    typedef struct game_input
    {
        r32 dtForFrame;
        u32 Entropy;
        
        game_controller_input Controllers[MAX_CONTROLLER_COUNT];
        
        // NOTE(casey): Signals back to the platform layer
        b32 QuitRequested;
        b32 EntropyRequested;
        
        // NOTE(casey): For debugging only
        game_button_state MouseButtons[PlatformMouseButton_Count];
        v3 ClipSpaceMouseP;
        b32 ShiftDown, AltDown, ControlDown;
        b32 FKeyPressed[13]; // NOTE(casey): 1 is F1, etc., for clarity - 0 is not used!
    } game_input;
    
    inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
    {
        Assert(ControllerIndex < ArrayCount(Input->Controllers));
        
        game_controller_input *Result = &Input->Controllers[ControllerIndex];
        return(Result);
    }
    
    inline b32 WasPressed(game_button_state State)
    {
        b32 Result = ((State.HalfTransitionCount > 1) ||
                      ((State.HalfTransitionCount == 1) && (State.EndedDown)));
        
        return(Result);
    }
    
    inline b32 IsDown(game_button_state State)
    {
        b32 Result = (State.EndedDown);
        
        return(Result);
    }
    
    typedef struct platform_file_handle
    {
        b32 NoErrors;
        void *Platform;
    } platform_file_handle;
    
    typedef struct platform_file_info
    {
        platform_file_info *Next;
        u64 FileDate; // NOTE(casey): This is a 64-bit number that _means_ the date to the platform, but doesn't have to be understood by the app as any particular date.
        u64 FileSize;
        char *BaseName; // NOTE(casey): Doesn't include a path or an extension
        void *Platform;
    } platform_file_info;
    typedef struct platform_file_group
    {
        u32 FileCount;
        platform_file_info *FirstFileInfo;
        void *Platform;
    } platform_file_group;
    
    typedef enum platform_file_type
    {
        PlatformFileType_AssetFile,
        PlatformFileType_SavedGameFile,
        PlatformFileType_HHT,
        PlatformFileType_Dump,
        
        PlatformFileType_Count,
    } platform_file_type;
    
    enum platform_memory_block_flags
    {
        PlatformMemory_NotRestored = 0x1,
        PlatformMemory_OverflowCheck = 0x2,
        PlatformMemory_UnderflowCheck = 0x4,
    };
    struct platform_memory_block
    {
        u64 Flags;
        u64 Size;
        u8 *Base;
        umm Used;
        platform_memory_block *ArenaPrev;
    };
    
#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group name(platform_file_type Type)
    typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);
    
#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
    typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);
    
    // TODO(casey): Add a way to replace a file, or set the size of a file?
    enum platform_open_file_mode_flags
    {
        OpenFile_Read = 0x1,
        OpenFile_Write = 0x2,
    };
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup, platform_file_info *Info, u32 ModeFlags)
    typedef PLATFORM_OPEN_FILE(platform_open_file);
    
#define PLATFORM_SET_FILE_SIZE(name) void name(platform_file_handle *Handle, u64 Size)
    typedef PLATFORM_SET_FILE_SIZE(platform_set_file_size);
    
#define PLATFORM_GET_FILE_BY_PATH(name) platform_file_info *name(platform_file_group *FileGroup, char *Path, u32 ModeFlags)
    typedef PLATFORM_GET_FILE_BY_PATH(platform_get_file_by_path);
    
#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Dest)
    typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);
    
#define PLATFORM_WRITE_DATA_TO_FILE(name) void name(platform_file_handle *Handle, u64 Offset, u64 Size, void *Source)
    typedef PLATFORM_WRITE_DATA_TO_FILE(platform_write_data_to_file);
    
#define PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(name) b32 name(platform_file_info *Info, u64 Size, void *Source)
    typedef PLATFORM_ATOMIC_REPLACE_FILE_CONTENTS(platform_atomic_replace_file_contents);
    
#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
    typedef PLATFORM_FILE_ERROR(platform_file_error);
    
#define PLATFORM_CLOSE_FILE(name) void name(platform_file_handle *Handle)
    typedef PLATFORM_CLOSE_FILE(platform_close_file);
    
#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)
    
    struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
    typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);
    
#define PLATFORM_ALLOCATE_MEMORY(name) platform_memory_block *name(memory_index Size, u64 Flags)
    typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);
    
#define PLATFORM_DEALLOCATE_MEMORY(name) void name(platform_memory_block *Block)
    typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);
    
    typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
    typedef void platform_complete_all_work(platform_work_queue *Queue);
    
    
    enum platform_error_type
    {
        PlatformError_Fatal,
        PlatformError_Nonfatal,
    };
#define PLATFORM_ERROR_MESSAGE(name) void name(platform_error_type Type, char *Message)
    typedef PLATFORM_ERROR_MESSAGE(platform_error_message);
    
    typedef struct platform_api
    {
        platform_add_entry *AddEntry;
        platform_complete_all_work *CompleteAllWork;
        
        platform_get_all_files_of_type_begin *GetAllFilesOfTypeBegin;
        platform_get_all_files_of_type_end *GetAllFilesOfTypeEnd;
        platform_get_file_by_path *GetFileByPath;
        platform_open_file *OpenFile;
        platform_set_file_size *SetFileSize;
        platform_read_data_from_file *ReadDataFromFile;
        platform_write_data_to_file *WriteDataToFile;
        platform_atomic_replace_file_contents *AtomicReplaceFileContents;
        platform_file_error *FileError;
        platform_close_file *CloseFile;
        
        platform_allocate_memory *AllocateMemory;
        platform_deallocate_memory *DeallocateMemory;
        
        platform_error_message *ErrorMessage;
        
#if HANDMADE_INTERNAL
        debug_platform_execute_system_command *DEBUGExecuteSystemCommand;
        debug_platform_get_process_state *DEBUGGetProcessState;
        debug_platform_get_memory_stats *DEBUGGetMemoryStats;
#endif
        
    } platform_api;
    extern platform_api Platform;
    
    typedef struct game_memory
    {
        struct game_state *GameState;
        
#if HANDMADE_INTERNAL
        struct debug_table *DebugTable;
        struct debug_state *DebugState;
#endif
        
        platform_work_queue *HighPriorityQueue;
        platform_work_queue *LowPriorityQueue;
        struct renderer_texture_queue *TextureQueue;
        
        b32 ExecutableReloaded;
        platform_api PlatformAPI;
    } game_memory;
    
    struct game_render_commands;
    struct game_render_settings;
#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_render_commands *RenderCommands)
    typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
    
    // NOTE(casey): At the moment, this has to be a very fast function, it cannot be
    // more than a millisecond or so.
    // TODO(casey): Reduce the pressure on this function's performance by measuring it
    // or asking about it, etc.
#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
    typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
    
#include "handmade_debug_interface.h"
    