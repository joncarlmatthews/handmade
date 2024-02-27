#ifndef HEADER_HH_MACROS
#define HEADER_HH_MACROS

//
// MACROS.H
//==============================================================================
// Macros that are shared across both platform and game layers


// Compiler stuff.
// -----------------------------------------------------------------------------

// Which compiler is being used?
// 
// MSVC
#if (!defined(COMPILER_MSVC))
#define COMPILER_MSVC 0
#endif

// LLVM/Clang
#if (!defined(COMPILER_LLVM))
#define COMPILER_LLVM 0
#endif

 // If neither are explicitly specified, try and determine which is being used
#if (!COMPILER_MSVC) && (!COMPILER_LLVM)
#ifdef _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#ifdef __llvm__
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif
#endif

#if (COMPILER_MSVC) && (COMPILER_LLVM)
assert(!"only 1 target compiler can be specified")
#endif


// Runtime debug settings.
// -----------------------------------------------------------------------------

#if HANDMADE_LOCAL_BUILD
    //#define HANDMADE_DEBUG_FPS
    //#define HANDMADE_DEBUG_CLOCKCYCLES
    //#define HANDMADE_DEBUG_AUDIO
    #define HANDMADE_LIVE_LOOP_EDITING
    //#define HANDMADE_DEBUG_LIVE_LOOP_EDITING
    //#define HANDMADE_DEBUG_TILE_POS
    //#define HANDMADE_WALK_THROUGH_WALLS
    #endif


    // Function-like Macros
    // -----------------------------------------------------------------------------

    // Return the number of elements in a static array
    #define countArray(arr) (sizeof(arr) / sizeof((arr)[0]))

    // Assertion
    #if HANDMADE_LOCAL_BUILD
        // NOLINTBEGIN
    #define assert(expression) \
        if (!(expression)) { \
            __pragma(warning(push)) \
            __pragma(warning(disable: 6011)) \
            int *address = 0x0; \
            *address = 0; \
            __pragma(warning(pop)) \
        }
    // NOLINTEND
    #else
    #define assert(expression)
    #endif

    // DLL function export syntax
    #if COMPILER_MSVC
    #define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)
    #else
    assert(!"TODO: Support for DLL function exports for non-Win32 platforms");
#endif


// Object-like Macros
// -----------------------------------------------------------------------------

// variables considered global
#define global_var static

// Static variables within a local scope (e.g. case statement, function)
#define local_persist_var static

// Functions that are only available within the translation unit they're declared in.
// This helps the compiler out by knowing that there is no external linking to be done.
#define internal_func static


// Game settings
// -----------------------------------------------------------------------------


// Game resolution. @see https://en.wikipedia.org/wiki/Display_resolution
// WXGA (16:9)
#define FRAME_BUFFER_PIXEL_WIDTH  1280
#define FRAME_BUFFER_PIXEL_HEIGHT 720

// FHD (16:9)
//#define FRAME_BUFFER_PIXEL_WIDTH  1920
//#define FRAME_BUFFER_PIXEL_HEIGHT 1080

// Maximum number of supported controllers. 1 keyboard, 4 gamepad controllers.
#define MAX_CONTROLLERS 5
#define GAME_PI 3.14159265358979323846   // pi
#define GAME_MAX_PATH 260

#endif