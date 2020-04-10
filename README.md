# Handmade Hero

## Pre-req

Install Visual Studio Community 2017. Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** options and check that the latest version of the Windows 10 SDK is installed. The Windows SDK gives you access to `windows.h`. If it's not installed you'll receive the `cannot open source file "windows.h"` error message when attempting the build the source code.

## Running

Once the project is open in Visual Studio, hit **F5** to run the program with the debugger attached. To run the program without the debugger attached hit **Ctrl** + **F5**.

To just build the program, hit **Ctrl** + **Shift** + **B**

## Building outside of Visual Studio

Open Command Prompt.

You'll need to run the shell batch file first.

```
> shell.bat
```

If you don't you'll receive the following error message:

```
> build.bat
'cl' is not recognized as an internal or external command,
operable program or batch file.
```

The `shell.bat` script sets the shell environment for building the source. This only needs to be run once per shell environment, not every time before you want to build the source.

Secondly, run the build script.

```
> build.bat
```
