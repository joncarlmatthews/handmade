# Handmade Hero

## Editing Source Files

Open Visual Studio (tested version Visual Studio Community 2015). Click File -> New -> Project From Existing Code... Select `Visual C++`. Browse to the root of this directory. Enter the Project Name and click Next. 

Within the Specify Project Settings window select "Use Visual Studio" with a Project Type of `Windows application project`. Click Next. Click Finish.

## Building

Once the project is open in  Visual Studio Community 2015, hit *F5* to build the program with the debugger attached. To build the program without the debugger attached hit *Ctrl* + *F5*.

### Old Notes

To build the source open the shell and run the build batch file.

```> build.bat```

Note, the first time you open the shell you'll need to run the shell batch file first.

```> shell.bat```

This sets the shell environment for building the source. This only needs to be done once per shell environment.