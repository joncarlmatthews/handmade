# Handmade

Low level game programming concepts in C/C++


## Building outside of an IDE

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
