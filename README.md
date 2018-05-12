# Handmade Hero

## Editing Source Files

Open Visual Studio (tested version Visual Studio Community 2015). Click File -> New -> Project From Existing Code... Select `Visual C++`. Browse to the root of this directory. Enter the Project Name and click Next. 

Within the Specify Project Settings window select "Use external build system" and click Next.

Within the Debug Configuration Settings window use the following values:

**Build command line:** build.bat
**Output (for debugging):** "build\win32_handmade_debug.exe

Click Finish.

## Building

Once the project is open in  Visual Studio Community 2015, hit *F5* to build the program with the debugger attached. To build the program without the debugger attached hit *Ctrl* + *F5*.

### Old Notes

To build the source open the shell and run the build batch file.

```> build.bat```

Note, the first time you open the shell you'll need to run the shell batch file first.

```> shell.bat```

This sets the shell environment for building the source. This only needs to be done once per shell environment.
