# "Handmade Hero"

Low level game engine programming in C flavoured C++. No frameworks or libraries are used for the game layer (except `math.h`). Everything written from scratch. Masochist.

## Current Progress

![Current progress](/current_state.png?raw=true "Current progress")

## To run the game

Within the `dist/` folder open the sub-folder relevant to your machine. E.g. `Win32_64bit/` for 64-bit Windows. Double click the executable within the folder.

Note: I have only written the platform layer for Windows thus far, meaning there are currently no playable versions for Mac or Linux (yet)

## Development

### Development Environment

Using Windows as the development environment as it meets the following developer requirements:

✔ Debugger

✔ Fast compiler

✔ Allows setting of a base memory address

To develop or build the game's source code first install [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community/). Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** and make sure the **Desktop development with C++** workflow is installed. This will install the C++ compiler.


### Code Structure

There are three distinct parts to the code.

1. The Platform Layer (handles the creation of the window, loading assets etc)
2. The Game Layer (handles the platform agnositc game engine and gameplay code)
3. Startup Library (Provided shared utilities and definitions)

Each distinct part is a separate Visual Studio [project](https://learn.microsoft.com/en-us/visualstudio/ide/solutions-and-projects-in-visual-studio?view=vs-2022#solutions). All three projects are held within a single [solution](https://learn.microsoft.com/en-us/visualstudio/ide/solutions-and-projects-in-visual-studio?view=vs-2022#projects).

The solution's build order is: `Startup` > `Game` > `Platform`, as `Platform` depends on both `Game` and `Startup` and `Game` depends on `Startup`.

Building the solution compiles all three projects automatically, in the order defined above. The platform layer, game layer and startup library are all built into the same build directory (`build/<platform>/<arch>/<buildmode>/`). The platform code is built as an executable, the game layer is built as a [dynamic link library](https://learn.microsoft.com/en-us/troubleshoot/windows-client/deployment/dynamic-link-library) (.dll) and the startup library is built as a [static library](https://en.wikipedia.org/wiki/Static_library) (.lib)

#### Platform Layer

The platform layer code resides within the `Platform Windows/` directory and associated VS project. The platform code is built as an executable (.exe) and is the main entry point for the game.

> At the time of writing, only the Windows platform layer has been prototyped. Future platform layers will reside in the `Platform Linux/` and `Platform macOS/` directories respectively.

#### Game Layer

The game layer (which includes the game engine and gameplay code) resides within the `Game/` directory and associated VS project. 

The game layer is built as a dynamic link library (.dll). The resulting DLL code is loaded into the platform executable at run time.

#### Startup Library

The startup library code resides within the `Startup/` directory and associated VS project. 

The starup code is built as a static library (.lib). The resulting .lib file is automatically copied into the code during the build process by the linker. `Startup.lib` is explicitly defined as an additional dependancy of both `Game` and `Platform`. 

### Coding Style

I recommend installing the [Editor Guidelines](https://marketplace.visualstudio.com/items?itemName=PaulHarrington.EditorGuidelinesPreview) Visual Studio plugin.

I have yet to install and configure a Linter. It's on the list!

### Building the source code

To build and/or run the program whilst developing, use Visual Studio.

To just build the program, hit **Ctrl** + **Shift** + **B**

All build files (including the binary executables) are placed into the `build` directory under their target architecture and build configuration, defined in this format: `build/<platform>/<arch>/<buildmode>/` E.g. `build/Windows/x86/Debug/`, or `build/Windows/x64/Release/`

### Running the program

To run the executable, open the built .exe binary within the relevant `build/<platform>/<arch>/<buildmode>/` directory

To run the program from directly within Visual Studio, hit **Ctrl** + **F5**.

### Debugging the source code

To run the program in Visual Studio with the debugger attached, simply hit **F5**.

### Dynamically reloading the game code

The game code is built as a DLL to enable dynamic reloading of (just) the game code without having the rebuild the whole program. This enables fast debug cycles.

Click: **Tools** > **Command Line** > **Developer Command Prompt**

Move into the `Game/` directory

```
> cd Game
```

Configure the shell for building the source for the target environment (x86 or x64). Note: **this only needs to be run once per shell environment**, not each time you want to re-build the source:

```
> shell_x86.bat
```

or 

```
> shell_x64.bat
```

If you don't, you'll receive the following error message when trying to build:

```
'cl' is not recognized as an internal or external command,
operable program or batch file.
```

Make sure you have `HANDMADE_LIVE_LOOP_EDITING` defined within `game.h`

```c
#define HANDMADE_LIVE_LOOP_EDITING
```

Launch the program from within Visual Studio with the debugger attached. **F5**.

With the program running, execute the build script with the relevant **Configuration** and **Platform Architecture** arguments:

```
> build.bat Debug x64
```

The platform layer will then dynamically reload the game code without the need to rebuild the platform code. You can safely run `build.bat` with (a) the Visual Studio debugger running the platform layer or (b) when simply running primary the .exe

This `build.bat` build script has been coded to match the Visual Studio build commands and so all files generated are placed in the usual directories.

### Useful links

 - [C-runtime library reference (MSVC)](https://learn.microsoft.com/en-us/cpp/c-runtime-library/c-run-time-library-reference)
 - C types: en.cppreference.com/w/c/types/**<type>** E.g. [https://en.cppreference.com/w/c/types/size_t](https://en.cppreference.com/w/c/types/size_t)
 - [C standard library header files](https://en.cppreference.com/w/c/header)
 - [GCC](https://gcc.gnu.org/onlinedocs/gcc-13.2.0/gcc/#SEC_Contents) and their [C preprocessor documentation](https://gcc.gnu.org/onlinedocs/cpp/index.html#SEC_Contents)
