# Handmade Hero

## Pre-req

Install Visual Studio Community 2017. Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** options and check that the latest version of the Windows 10 SDK is installed. The Windows SDK gives you access to `windows.h`. If it's not installed you'll receive the `cannot open source file "windows.h"` error message when attempting the build the source code.

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

If you don't you'll receive the following error message:

```
'cl' is not recognized as an internal or external command,
operable program or batch file.
```

The shell scripts sets the shell environment for building the source. This only needs to be run once per shell environment, not every time before you want to build the source.

Lastly, run the relevant build script.

```
> build_x86.bat
```

or

```
> build_x64.bat
```

All build scripts are placed within the root of the relevant `build\<arch>\` folder.
