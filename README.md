# "Handmade Hero"

Low level game programming in C flavoured C++.

## Current Progress

![Current progress](/current_state.png?raw=true "Current progress")

## To run the game

Within the `dist/` folder open the sub-folder relevant to your machine. E.g. `Win32_64bit/` for 64-bit Windows. Double click the executable within the folder.

Note: I have only written the platform layer for Windows thus far, meaning there are currently no playable versions for Mac or Linux (yet)

## To develop the game

### Development Environment

Using Windows as the development environment as it meets the following developer requirements:

✔ Debugger

✔ Fast compiler

✔ Allows setting of a base memory address

To develop or build the game's source code first install [Visual Studio Community 2022](https://visualstudio.microsoft.com/vs/community/). Once Visual Studio is installed navigate to **Tools** -> **Get Tools and Features** and make sure the **Desktop development with C++** workflow is installed. This will install the C++ compiler.


### Project Structure

There are two main parts to the project, the Win32 platform code (`Platform Win32\`) and the game code (`Game\`). Both are built into the same build directory. The platform code is built as an exe and the game code is built as a DLL.

### Coding Style

I recommend installing the [Editor Guidelines](https://marketplace.visualstudio.com/items?itemName=PaulHarrington.EditorGuidelinesPreview) Visual Studio plugin. I have yet to install and configure a Linter. It's on the list!

### Building the source code

To build and/or run the program whilst developing, use Visual Studio.

To just build the program, hit **Ctrl** + **Shift** + **B**

All build files (including the binary executables) are placed into the `build` directory under their target architecture and build configuration, defined in this format: `build\Win32\<arch>\<config>\` E.g. `build\Win32\x86\Debug\`, or `build\Win32\x64\Release\`

### Running the program

To run the executable, open the built .exe binary within the relevant `build\Win32\<arch>\<config>\` directory

To run the program from directly within Visual Studio, hit **Ctrl** + **F5**.

### Debugging the source code

To run the program in Visual Studio with the debugger attached, simply hit **F5**.

### Dynamically reloading the game code

The game code is built as a DLL to enable dynamic reloading of (just) the game code without having the rebuild the whole program. If you want to dynamically reload the game code, with the program running, execute the `built.bat` file using the Windows command prompt from with the `Game/` directory.

Click: **Tools** > **Command Line** > **Developer Command Prompt**

```
> cd Game
```

Configure the shell for building the source for the target environment (x86 or x64). Note: **this only needs to be run once per shell environment**, not each time you want to re-build the source:

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

Finally, run the build script with the relevant **Configuration** and **Platform Architecture** arguments:

```
> build.bat Debug x64
```

The platform layer will then dynamically reload the game code without the need to rebuild the platform code. You can safely run `build.bat` with (a) the Visual Studio debugger running the platform layer or (b) when simply running primary the .exe

This `build.bat` build script has been coded to match the Visual Studio build commands and so all files generated are placed in the usual directories.