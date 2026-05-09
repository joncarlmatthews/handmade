# C/C++ Specs, Vendors, Build, and Distribution

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

## Making sense of the moving pieces

### Specs And Implementations

**1. C/C++ language specification**

The language is the syntax and rules: variables, structs, pointers, functions,
`if`, `while`, templates, overloads, etc. It is not the compiler and it is not
the standard library.

The language specification describes what valid C/C++ code means. It does not
tell you which executable program compiles that code on your machine.

**2. C standard library specification**

The C standard library is primarily a specification: it says functions like
`malloc`, `free`, `printf`, `fopen`, `memcpy`, and `strlen` exist and describes
how they should behave.

The standard library specification is a contract. It is not, by itself, a folder
of source files you download from the C language committee.

**3. Vendors and implementations**

Vendors provide the concrete tools and libraries that make the specs usable on a
real machine. A vendor might provide a compiler, preprocessor, linker, headers,
standard library implementation, runtime pieces, platform SDKs, debugger
integration, and IDE/build-system support.

Examples:

- Windows/MSVC: Microsoft C runtime, nowadays commonly the Universal C Runtime.
- macOS/Clang: Clang compiles the code, while Apple's system libraries provide
  the C library implementation.
- Linux: often glibc or musl, regardless of whether GCC or Clang compiles the
  code.

| Vendor / ecosystem | What they provide | Notes |
| --- | --- | --- |
| Microsoft / MSVC | `cl.exe` compiler, preprocessor, `link.exe` linker, Windows SDK integration, MSVC headers, Universal C Runtime, MSVC runtime libraries, Visual Studio debugger/IDE/build tools | This is the normal Windows path for this project. The runtime gets installed with Visual Studio/Build Tools and may be installed for end users via the Visual C++ Redistributable. |
| Apple / Xcode | Apple Clang compiler, Apple linker tooling, macOS SDK, system headers, AppKit/CoreAudio/CoreGraphics frameworks, Apple system C library/runtime support, Xcode debugger/IDE/build tools | This is the normal macOS path for this project. Clang compiles the code, while Apple's SDK and system libraries provide the platform APIs and runtime/library pieces. |
| LLVM / Clang | Clang compiler frontend, preprocessor behavior, diagnostics, optimizer/backend via LLVM, compiler builtins and related toolchain pieces | Clang is mostly the compiler/toolchain technology, not automatically the whole C runtime. On macOS it is paired with Apple system libraries; on Linux it may be paired with glibc or musl; on Windows it can target MSVC-compatible runtimes. |
| GNU / Linux ecosystem | GCC compiler, GNU linker/binutils or LLVM linker alternatives, glibc on many distributions, POSIX/Linux headers and libraries, build tools like Make | Linux is less one-vendor-shaped. A distribution assembles compiler, libc, linker, kernel headers, debugger, and package manager pieces. |
| musl ecosystem | musl C library implementation, often paired with GCC or Clang | musl is an alternative libc commonly used for small/static Linux systems. It is a standard library/runtime implementation, not a compiler by itself. |

The key idea is:

```text
specification
  describes what should exist and how it should behave

implementation
  real vendor code/tools installed on your machine
```

**4. Getting C/C++ on your machine**

You do not install "C" itself. You install a toolchain and SDK that let you
write, compile, link, debug, and run C/C++ programs on a specific platform.

On Windows for this project, that usually means installing Visual Studio or the
Visual Studio Build Tools with the Desktop development with C++ workload. That
puts MSVC's compiler, preprocessor, linker, headers, libraries, Windows SDK
pieces, and runtime support on the machine.

End-user Windows machines may also need runtime DLLs. Those can arrive from the
Microsoft Visual C++ Redistributable, from Windows itself for some Universal C
Runtime pieces, or from an application installer that bundles the needed DLLs.

On macOS for this project, that means installing Xcode or the Xcode Command Line
Tools. That provides Apple Clang, Apple linker tooling, SDK headers, SDK stubs,
framework metadata, and the pieces needed to build against Apple's system
libraries. The actual system C library/runtime support is provided by macOS and
Apple's developer tools.

This is why the preprocessor can run: it is part of the installed compiler
toolchain. It is not provided by the C language spec and it is not provided by
your program's runtime.

**5. Platform API and SDK**

Platform APIs are operating-system-specific services:

- Win32, DirectSound, XInput on Windows.
- AppKit, CoreGraphics, CoreAudio, POSIX/mach APIs on macOS.

This project's platform layers should hide those APIs from the game layer.

The SDK is the bundle of headers, libraries, metadata, and tools that lets code
compile against those platform APIs.

**6. Build system / IDE**

Visual Studio projects, Xcode projects, `.bat` scripts, Make, and CMake do not
define the C/C++ language. They describe what files to compile, what compiler
flags to use, what libraries to link, where outputs go, and how debugging/running
is configured.

### Build And Runtime Pieces

**1. Preprocessor**

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

**2. Compiler**

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

**3. C runtime and standard library implementation**

The runtime is support code linked into or used by the program while it runs.
It helps with process startup, calling `main`, static initialization, heap
support, and standard library behavior. It is runtime support, not
preprocessor logic.

There are usually two sides:

- **Build-time pieces:** headers, import libraries, SDK stubs, startup object
  files, and linker metadata used while building.
- **Run-time pieces:** shared libraries or system libraries that the executable
  loads when it runs.

Some runtime code can be linked directly into the executable, and some can be
loaded from shared libraries. Which one happens depends on compiler/linker flags
and platform conventions.

**4. Linker**

The linker combines object files and resolves external symbols from libraries.
If one file calls `gameUpdate`, the linker is responsible for finding the object
file or library that actually provides `gameUpdate`.

The linker also decides which libraries become part of the final executable and
which dynamic libraries the executable expects to load at runtime.

### Running On An End User's Machine

There are usually two different machines to think about:

- **Build machine:** the developer machine that has the compiler, linker, SDKs,
  headers, debugger, and build system installed.
- **User machine:** the machine that only needs enough files and runtime support
  to run the finished program.

An end user normally does not need the compiler or SDK. They need the executable
and whatever runtime/data/library files the executable depends on.

#### Windows

For a simple Windows C/C++ program, distribution can be as simple as:

```text
Handmade Hero.exe
```

The user double-clicks the `.exe` and Windows loads it.

In practice, a game often needs more than the `.exe`:

```text
Handmade Hero.exe
Game.dll
data/
  test/
    test_hero_front_head.bmp
    ...
```

The `.exe` is the platform layer. The `.dll` may be the game layer. The `data/`
folder contains assets. Those files need to be placed where the program expects
to find them.

The executable may also depend on runtime DLLs, for example MSVC runtime DLLs.
Those can be handled in a few common ways:

- Install the Microsoft Visual C++ Redistributable on the user's machine.
- Bundle the required runtime DLLs next to the `.exe`, when the license and
  project settings allow it.
- Link some runtime pieces statically, so fewer external runtime DLLs are needed.

The target architecture matters. A 64-bit Windows build produces a 64-bit `.exe`
for 64-bit Windows. A 32-bit Windows build produces a 32-bit `.exe` that can run
on many 64-bit Windows machines through compatibility support, but not vice
versa. For this project, a release folder might look like:

```text
dist/
  Windows_64bit/
    Handmade Hero.exe
    Game.dll
    data/
```

#### macOS

On macOS, the usual user-facing artifact is an app bundle:

```text
Handmade Hero.app
```

It looks like a single app in Finder, but it is really a directory with a
specific structure:

```text
Handmade Hero.app/
  Contents/
    MacOS/
      Handmade Hero
    Resources/
      data/
```

The user double-clicks the `.app`, and macOS launches the executable inside
`Contents/MacOS/`.

Like Windows, the app still needs its assets and any dynamic libraries it depends
on. macOS apps may also need signing/notarization for smooth distribution outside
your own machine, especially if other people download the app from the internet.

#### Build Once, Run Elsewhere

In normal C/C++ distribution, users do not rebuild the program themselves. The
developer builds a binary for a target platform and architecture, then ships the
resulting executable/app plus its dependencies.

For this Handmade project, the mental model is:

```text
Windows build machine
  -> build Windows x64 exe/dll
  -> ship dist/Windows_64bit/
  -> user runs Handmade Hero.exe

macOS build machine
  -> build macOS app
  -> ship Handmade Hero.app plus assets
  -> user runs Handmade Hero.app
```

Cross-compiling is possible in C/C++, but it is an extra setup problem. The
simple path is to build Windows binaries on Windows and macOS binaries on macOS.
