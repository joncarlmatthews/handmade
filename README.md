# Handmade Hero

## Editing Source Files

Open Visual Studio (tested version Visual Studio Community 2015). Click File -> New -> Project From Existing Code... Browse to the root of this directory. Enter the Project Name so that it matches that of the build file's exe (win32_handmade) and click Next. 

Within the Specify Project Settings window select "Use external build system" Click Next.

...

## Building

To build the source open the shell and run the build batch file.

```> build.bat```

Note, the first time you open the shell you'll need to run the shell batch file first.

```> shell.bat```

This sets the shell environment for building the source. This only needs to be done once per shell environment.

To debug the executable in Visual Studio once the source is built, run

```> devenv win32_handmade.exe```

When Visual Studio has loaded, hit F11 to start the debugging process.