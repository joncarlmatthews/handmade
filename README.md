# Handmade Hero

## Editing Source Files

Open Visual Studio (tested version Visual Studio Community 2015). Click File -> New -> Project From Existing Code... Select `Visual C++`. Browse to the root of this directory. Enter the Project Name and click Next. 

Within the Specify Project Settings window select "Use external build system" and click Next.

Within the Debug Configuration Settings window use the following values:

**Build command line:** build.bat

**Output (for debugging):** "build\win32_handmade_debug.exe

Click Finish.

## Running

Once the project is open in Visual Studio Community 2015, hit **F5** to run the program with the debugger attached. To run the program without the debugger attached hit **Ctrl** + **F5**.

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