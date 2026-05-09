# Day 021 - Loading Game Code Dynamically

Re organised the Visual Studio project so there is now a single `Solution` that contains three `Projects`.

- The Win32 platform code
- The game code
- Utility code that is shared between the platform and game.

I've configured VS so that when you build the project, all three Projects are compiled and built and ready to run without any additional work needed. However, I also created the `Game\build.bat` to facilitate dynamic game code reloading outside of VS. At the moment, this `build.bat` file can only be run when the .exe is running outside of the VS Debugger.
