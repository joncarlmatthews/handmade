# C/C++ Toolchain Layers

This is the mental model I want to keep while moving this project between
Windows/MSVC and macOS/Clang.

```text
Source files
  Game/*.cpp, Platform Windows/*.cpp, Platform macOS/*.mm
        |
        v
Preprocessor
  Expands #include, #define, #if, #ifdef, defined(...)
  This is part of the compiler toolchain, not the C runtime.
        |
        v
Compiler
  Parses C/C++, type-checks, optimizes, emits object code.
  Examples: MSVC cl.exe, Clang/LLVM.
        |
        v
Object files
  .obj on Windows, .o on macOS/Linux.
        |
        v
Linker
  Combines object files and resolves external symbols from libraries.
  Examples: MSVC link.exe, Apple ld via Clang/Xcode.
        |
        v
Executable / dynamic library
  .exe/.dll on Windows, .app/.dylib on macOS.
```

## Roles And Responsibilities

**C/C++ language**

The language is the syntax and rules: variables, structs, pointers, functions,
`if`, `while`, templates, overloads, etc. It is not the compiler and it is not
the standard library.

**Preprocessor**

The preprocessor is an early text-processing stage in the compiler toolchain.
It handles `#include`, `#define`, `#if`, `#ifdef`, and `defined(...)`.

Example:

```c
#if defined(_DEBUG) || defined(DEBUG)
    #define HANDMADE_LOCAL_BUILD
#endif
```

`defined(...)` is built into the preprocessor. It is not a function, not a macro
I define myself, and not part of the C runtime.

**Compiler**

The compiler turns preprocessed C/C++ into object code. MSVC and Clang are
compilers/toolchains. They also provide compiler-specific predefined macros and
extensions, which is why shared headers need compiler branches.

Examples:

```c
#if COMPILER_MSVC
    // MSVC-specific code
#elif COMPILER_LLVM
    // Clang/LLVM-specific code
#endif
```

**C standard library**

The C standard library is primarily a specification: it says functions like
`malloc`, `free`, `printf`, `fopen`, `memcpy`, and `strlen` exist and describes
how they should behave.

The actual implementation comes from the platform/toolchain:

- Windows/MSVC: Microsoft C runtime, nowadays commonly the Universal C Runtime.
- macOS/Clang: Clang compiles the code, while Apple's system libraries provide
  the C library implementation.
- Linux: often glibc or musl, regardless of whether GCC or Clang compiles the
  code.

**C runtime**

The runtime is support code linked into or used by the program while it runs.
It helps with process startup, calling `main`, static initialization, heap
support, and standard library behavior. It is runtime support, not
preprocessor logic.

**Platform API**

Platform APIs are operating-system-specific services:

- Win32, DirectSound, XInput on Windows.
- AppKit, CoreGraphics, CoreAudio, POSIX/mach APIs on macOS.

This project's platform layers should hide those APIs from the game layer.

**Build system / IDE**

Visual Studio projects, Xcode projects, `.bat` scripts, Make, and CMake do not
define the C/C++ language. They describe what files to compile, what compiler
flags to use, what libraries to link, where outputs go, and how debugging/running
is configured.

## Handmade Project Split

```text
Game/
  Platform-independent game code.
  Should not include Win32, AppKit, DirectSound, CoreAudio, etc.

Platform Windows/
  Windows-specific platform layer.
  Owns Win32 windowing, input, timing, file I/O, DirectSound, XInput, and
  loading/reloading the game DLL.

Platform macOS/
  macOS-specific platform layer.
  Owns AppKit windowing, input, timing, file I/O, framebuffer display, and later
  CoreAudio/dylib loading.
```

The game layer defines what services it needs from a platform in `Game/game.h`.
Each platform layer implements those services using its own operating-system APIs.

## Key Distinction

```text
#if defined(DEBUG)
```

This is compile-time preprocessor logic.

```c
if (debugMode) {
}
```

This is runtime program logic.

That distinction matters because preprocessor branches can completely remove code
before the compiler proper sees it, while runtime `if` statements compile into
the program and execute later.

