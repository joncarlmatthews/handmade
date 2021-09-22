# Handmade

Low level game programming concepts in C/C++

## Pre-req

Install Visual Studio Community 2019. Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** options and check that the latest version of the Windows 10 SDK is installed. The Windows SDK gives you access to `windows.h`. If it's not installed you'll receive the `cannot open source file "windows.h"` error message when attempting the build the source code.

## Building

To just build the program, hit **Ctrl** + **Shift** + **B**

All build files (including the binary executables) are placed into the `build` directory under their target architecture and build configuration, defined in this format: `build\<arch>\<bldconfig>\` E.g. `build\Win32\Debug\`, `build\Win32\Release\` or `build\x64\Debug\`

## Running

To run the executable, open the .exe within the relevant `build\<arch>\<bldconfig>\` directory

To run the program from directly within Visual Studio, hit **Ctrl** + **F5**. To run the program in Visual Studio with the debugger attached, simply hit **F5**.

## Building outside of Visual Studio

Open Command Prompt. Navigate to the source directory:

```
> cd C:\work\dev\handmade
```

Configure Visual Studio's vcvarsall.bat to build in either x86 or x64 mode by running the relevant shell batch file:

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

The purpose of the shell scripts are to set the shell environment for building the source. This only needs to be run once per shell environment, not every time before you want to build the source.

Lastly, run the relevant build script.

```
> build_x86.bat
```

or

```
> build_x64.bat
```

All build files and executables are placed within the root of the relevant `build\<arch>\` folder.

## Command line flags

```
HANDMADE_LOCAL_BUILD
  - 0 for all non-dev builds
  - 1 to include code that should only be included within a local build of the game
```

```
HANDMADE_DEBUG
  - 0 no arbitrary debug
  - 1 arbitrary debug
```

```
HANDMADE_DEBUG_FPS
  - 0 no FPS console debug
  - 1 to debug FPS counts to the console.
```

```
HANDMADE_DEBUG_AUDIO
  - 0 no audio debug
  - 1 debug audio
```

## Episode Notes

### Day 021 - Loading Game Code Dynamically


### Day 020 - Debugging the Audio Sync

I think the solution was overly complex. I've chosen to write the frame's audio at the earliest possible place (which is at the write cursor) I then opt to write at least 4 frames worth of audio, to avoid gaps in the audio if the framerate were to unexpectedly drop. That's it. That's my solution.

### Day 019 - Improving Audio Synchronization

Good description of strategy here: [https://youtu.be/qFl62ka51Mc?t=5499](https://youtu.be/qFl62ka51Mc?t=5499)

I didn't do the audio to page flip sync, as I don't get how being 3 frames latent is better than the soundcard's 30ms (0.9 of a frame)

### Day 018 - Enforcing a Video Frame Rate

Hertz is another term for "cycles per second".

If our target is 60 frames per second, how many milliseconds do we have to compute a single frame?

(1000 milliseconds in 1 second)

60 frames per second  = (1000ms / 60fps) = 16.6ms per frame

30 frames per second  = (1000ms / 60fps) = 33.3ms per frame

`__rdtsc` is not used to determine time elapsed as it varies from machine to machine, rather it's used for profiling the performance of code.

`QueryPerformanceCounter` is used to determine how long it takes for a certain amount of code to execute.

To include an external .Lib (Library file) to your Project and include it within the linking phase of the compilation process, right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> Linker` -> `Input`. Assuming you want the .lib file to be included for all Configurations and all Plarforms, make sure you select `All Configurations` and `All Platforms` at the top of the dialog. Once done, in the top input box (`Additional Dependencies`) add the `.lib` file preceded by a semicolon to the end of the string e.g. `;Winmm.lib` and click `Apply`


### Day 016 - VisualStudio Compiler Switches

I'm not using the bash files and instead am using Visual Studio 2019 to run the build.

For each Project within the Solution:

To view the existing compiler flags, right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `C/C++` -> `Command Line`. In the top input box (`All Options`) you'll see the flags currently set. Note different flags will be set based on whether you're viewing the `Debug` or `Release` Configuration.

To add additional compiler flags, input them into the bottom input box (`Additional Options`). Note remember to have the correct Configuration selected (`Debug` or `Release`).

[View all Compiler flags](https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically)

 - -wd4201 (Don't warn on nameless struct/union)
 - -wd4100 (Ignore unreferenced formal parameter warnings so we can take our [approach of handling unresolved externals](https://github.com/joncarlmatthews/private/blob/master/technical/learning/c-cpp.md#how-to-handle-unresolved-externals))
 - /GR- (Disable run-time type checking as we wont use this)
 - -D HANDMADE_LOCAL_BUILD=1 (For debug builds only add a custom HANDMADE_LOCAL_BUILD flag)

### Day 000 - Visual Studio Set up - Build directories

I opted to tidy up and consolidate the location of the build files.

For each Project within the Solution:

Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `General` -> `General Properties`. Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. For the `Output Directory` value input `$(SolutionDir)build\$(ProjectName)\$(Platform)\$(Configuration)\`. For the `Intermediate Directory` value input `$(SolutionDir)build\$(ProjectName)\intermediates\$(Platform)\$(Configuration)\`. Click `Apply`

The Windows Platform project requires `winmm.lib` for the `timeBeginPeriod` and `timeEndPeriod` external symbols. Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `Linker` -> `Input`. Select `Additional Dependencies.  Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. Add `winmm.lib` to the list. Click `Apply`