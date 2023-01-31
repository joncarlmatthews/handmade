# "Handmade Hero"

Low level game programming in C flavoured C++.

## Current Progress

![Current progress](/current_state.png?raw=true "Current progress")

## Development Environment

Using Windows as the development environment as it meets the following developer requirements:

✔ Debugger

✔ Fast compiler

✔ Allows setting of a base memory address

## Pre-req

Install Visual Studio Community 2019. Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** options and check that the latest version of the Windows 10 SDK is installed. The Windows SDK gives you access to `windows.h`. If it's not installed you'll receive the `cannot open source file "windows.h"` error message when attempting the build the source code.

## Project Structure

There are two main parts to the project, the Win32 platform code (`Platform Win32\`) and the game code (`Game\`). Both are built into the same build directory. The platform code is built as an exe and the game code is built as a DLL.

## Building

To just build the program, hit **Ctrl** + **Shift** + **B**

All build files (including the binary executables) are placed into the `build` directory under their target architecture and build configuration, defined in this format: `build\Win32\<arch>\<config>\` E.g. `build\Win32\x86\Debug\`, or `build\Win32\x64\Release\`

## Running

To run the executable, open the .exe within the relevant `build\Win32\<arch>\<config>\` directory

To run the program from directly within Visual Studio, hit **Ctrl** + **F5**.

To run the program in Visual Studio with the debugger attached, simply hit **F5**.

## Reccommended Visual Studio Plugins
[Editor Guidelines](https://marketplace.visualstudio.com/items?itemName=PaulHarrington.EditorGuidelinesPreview)

## Dynamically reloading the game code

The game code is built as a DLL to enable dynamic reloading of (just) the game code without having the rebuild the whole program. If you want to dynamically reload the game code, with the program running, execute the `built.bat` file using the Windows command prompt from with the `Game` directory.

Click: `Tools` -> `Command Line` -> `Developer Command Prompt`

```
> cd Game
```

Configure the shell for building the source to the desired environment (x86 or x64). This only needs to be run once per shell environment, not every time before you want to build the source:

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

Finally, run the build script with the relevant Configuration and Release arguments.

```
> build.bat Debug x86
```

The platform layer will then dynamically reload the game code without the need to rebuild the platform code. You can safely run `build.bat` with (a) the Visual Studio debugger running the platform layer or (b) when simply running primary the .exe

This `build.bat` build script has been coded to match the Visual Studio build commands and so all files generated are placed in the usual directories.