# Handmade Hero

## Pre-req

Install Visual Studio Community 2017. Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** options and check that the latest version of the Windows 10 SDK is installed. The Windows SDK gives you access to `windows.h`. If it's not installed you'll receive the `cannot open source file "windows.h"` error message when attempting the build the source code. The other setting I had to change to resolve this issue was to change the **Platform** from **Win32** to **x64** under **Project -> Properties**. This will then show the **Windows SDK Version** option where you can select the latest version of the Windows SDK which was installed in the first step.

## Editing Source Files

Open Visual Studio (tested version Visual Studio Community 2017). Click File -> New -> Project From Existing Code... Select `Visual C++`. Browse to the root of this directory. Enter the Project Name and click Next. 

Within the Specify Project Settings window select "Use external build system" and click Next.

Within the Debug Configuration Settings window use the following values:

**Build command line:** build.bat

**Output (for debugging):** build\win32_handmade_debug.exe

Click Finish.

## Running

Once the project is open in Visual Studio, hit **F5** to run the program with the debugger attached. To run the program without the debugger attached hit **Ctrl** + **F5**.

To just build the program, hit **Ctrl** + **Shift** + **B**

All three of the above will invoke the custom build script (`build.bat`). You can verify this by viewing the build script debug. You will see the batch file's signature.

Note, if you're running the `build.bat` file outside the context of Visual Studio, you'll need to run the shell batch file first.

```
> shell.bat
> build.bat
```

If you don't you'll receive the following error message:

```
> build.bat
'cl' is not recognized as an internal or external command,
operable program or batch file.
```

The `shell.bat` script sets the shell environment for building the source. This only needs to be run once per shell environment, not every time before you want to build the source.
