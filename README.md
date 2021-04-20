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

## Episode Notes

### Day 016 - VisualStudio Compiler Switches

I'm not using the bash files and instead am using Visual Studio 2019 to run the build.

To view the existing compiler flags, right click on the Project in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `C/C++` -> `Command Line`. In the top input box (`All Options`) you'll see the flags currently set. Note different flags will be set based on whether you're viewing the `Debug` or `Release` Configuration.

To add additional compiler flags, input them into the bottom input box (`Additional Options`). Note remember to have the correct Configuration selected (`Debug` or `Release`).

[View all Compiler flags](https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically)

 - -wd4201 (Don't warn on nameless struct/union)
 - -wd4100 (Ignore unreferenced formal parameter warnings so we can take our [approach of handling unresolved externals](https://github.com/joncarlmatthews/private/blob/master/technical/learning/c-cpp.md#how-to-handle-unresolved-externals))
 - /GR- (Disable run-time type checking as we wont use this)
 - -D HANDMADE_LOCAL_BUILD=1 (For debug builds only add a custom HANDMADE_LOCAL_BUILD flag)
