## Episode Notes

### Day 035 - Virtualized Tile Maps

Now only reserving memory from our "tiles memory block" when we need to. As in, when we write a value to a tile. That said, the memory block allocated to hold all of the tile data is hardcoded to 1MB, it's just that we simply don't reserve a portion of it for a chunk's tiles if we don't need to. With this in mind, the sparse storage is somewhat basic at this point in the project.

### Day 033 - Virtualized Tile Maps

Ripped out the tile based scrolling. Implemented smooth scrolling. Made the world toroidal. This was the toughest day yet.


### Day 028 -  Drawing a Tile Map

Variables are written to starting from the right most bit, and move left one-bit at a time. Any bits not set, are left as 0. E.g.

```c
uint32 testVar;
testVar = 0xFFFF;      // 00000000 00000000 11111111 11111111
testVar = 0xFFFFFF;    // 00000000 11111111 11111111 11111111
testVar = 0xFFFFFFFF;  // 11111111 11111111 11111111 11111111
```


### Day 025 -  Finishing the Win32 Prototyping Layer

I refactored the game recording from using a file on disk to simply doing a memory swap. This means that when in a local build the platform layer allocates double the memory required to run the game. One block for the game itself, and one block for the recording. The reason for the refactoring is that copying memory is much quicker than writing the entire allocated game memory to a file on disk.

TCHAR:

A WCHAR if UNICODE is defined, a CHAR otherwise.

WCHAR:

A 16-bit Unicode character. For more information, see Character Sets Used By Fonts.

### Day 023 -  Looped Live Code Editing

When we hit the L key, for each frame, we blast the memory held within the input struct out to a file on disk. When we hit the L key again we stop writing out to the recording file. We write the entire input struct each time. At the time of writing the input struct is 520 bytes in size. Meaning we write 520 bytes worth of data for each frame recorded.

After we've finished recording we immediately set the playback flag to `true` meaning we then (for each frame) read out 520 bytes *into* the input struct (therefore overwriting it entirely each time) This has the affect of "playing back" the input recorded.  When an application calls `CreateFile` to open a file for the first time, Windows places the file pointer at the beginning of the file. As bytes are read from or written to the file, Windows advances the file pointer the number of bytes read. Therefore, on each loop, the next set of 520 bytes are read and so on, until there are no more bytes to read from the input recording. When we hit the end of the file (aka we've read all the bytes) we kick off the read again from the start, meaning that the input recording is on an indefinite loop until the debugging session is stopped.

In addition to recording and looping the input, we save a copy of the game state at the point when we start recording the input. Then, when we begin the recording playback, we set the game state back to this recorded game state snapshot. This means that we can loop the game state back to a moment in time over and over again (which helps with debugging) E.g. if you killed a boss during the recorded input, the boss would reappear when the recording input loops.

In Windows, each process has up to a certain amount of addressable memory (no matter how much physical memory is actually available for the process). The OS does the hard work of moving stuff in an out of memory between different processes. 

In terms of the addressable memory limits; on 32-bit versions of Windows, a single process can map and address no more than 3GB of virtual memory at time. 

In 64-bit versions of Windows, a 32-bit process can map and address no more than 4GB of virtual memory at a time. For 64-bit processes, the amount is typically around 7TB. As yet, I don't understand how we're able to set the base address to 4TB deep on the 32-bit build.

### Day 022 -  Instantaneous Live Code Editing

Can now live reload the game code whilst the platform layer is running either directly via the .exe or via the VS debugger!

### Day 021 - Loading Game Code Dynamically

Re organised the Visual Studio project so there is now a single `Solution` that contains three `Projects`.

- The Win32 platform code
- The game code
- Utility code that is shared between the platform and game.

I've configured VS so that when you build the project, all three Projects are compiled and built and ready to run without any additional work needed. However, I also created the `Game\build.bat` to facilitate dynamic game code reloading outside of VS. At the moment, this `build.bat` file can only be run when the .exe is running outside of the VS Debugger.

### Day 020 - Debugging the Audio Sync

I think the solution was overly complex. I've chosen to write the frame's audio at the earliest possible place (which is at the write cursor) I then opt to write at least 4 frames worth of audio, to avoid gaps in the audio if the framerate were to unexpectedly drop. That's it. That's my solution.

### Day 019 - Improving Audio Synchronization

Good description of strategy here: [https://youtu.be/qFl62ka51Mc?t=5499](https://youtu.be/qFl62ka51Mc?t=5499)

I didn't do the audio to page flip sync, as I don't get how being 3 frames latent is better than the soundcard's 30ms (0.9 of a frame)

### Day 018 - Enforcing a Video Frame Rate

Hertz is another term for "cycles per second".

If our target is 60 frames per second, how many milliseconds do we have to compute a single frame?

(1000 milliseconds in 1 second)

60 frames per second  = (1000ms / 60fps) = 16.6ms per frame

30 frames per second  = (1000ms / 60fps) = 33.3ms per frame

`__rdtsc` is not used to determine time elapsed as it varies from machine to machine, rather it's used for profiling the performance of code.

`QueryPerformanceCounter` is used to determine how long it takes for a certain amount of code to execute.

To include an external .Lib (Library file) to your Project and include it within the linking phase of the compilation process, right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> Linker` -> `Input`. Assuming you want the .lib file to be included for all Configurations and all Plarforms, make sure you select `All Configurations` and `All Platforms` at the top of the dialog. Once done, in the top input box (`Additional Dependencies`) add the `.lib` file preceded by a semicolon to the end of the string e.g. `;Winmm.lib` and click `Apply`


### Day 016 - VisualStudio Compiler Switches

I'm not using the bash files and instead am using Visual Studio 2019 to run the build.

For each Project within the Solution:

To view the existing compiler flags, right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `C/C++` -> `Command Line`. In the top input box (`All Options`) you'll see the flags currently set. Note different flags will be set based on whether you're viewing the `Debug` or `Release` Configuration.

To add additional compiler flags, input them into the bottom input box (`Additional Options`). Note remember to have the correct Configuration selected (`Debug` or `Release`).

[View all Compiler flags](https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options-listed-alphabetically)

 - -wd4201 (Don't warn on nameless struct/union)
 - -wd4100 (Ignore unreferenced formal parameter warnings so we can take our [approach of handling unresolved externals](https://github.com/joncarlmatthews/private/blob/master/technical/learning/c-cpp.md#how-to-handle-unresolved-externals))
 - /GR- (Disable run-time type checking as we wont use this)
 - -D HANDMADE_LOCAL_BUILD=1 (For debug builds only add a custom HANDMADE_LOCAL_BUILD flag)

### Day 000 - Visual Studio Set up - Build directories

*I opted to tidy up and consolidate the location of the build files.*

For each Project within the Solution:

Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `General` -> `General Properties`. Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. For the `Output Directory` value input `$(SolutionDir)build\$(ProjectName)\$(Platform)\$(Configuration)\`. For the `Intermediate Directory` value input `$(SolutionDir)build\$(ProjectName)\intermediates\$(Platform)\$(Configuration)\`. Click `Apply`

The Windows Platform project requires `winmm.lib` for the `timeBeginPeriod` and `timeEndPeriod` external symbols. Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `Linker` -> `Input`. Select `Additional Dependencies.  Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. Add `winmm.lib` to the list. Click `Apply`

*Set the warning to Level 4*

For each Project within the Solution:

Right click on the Project's name in the Solution Explorer and click on `Properties`. Then select `Configuration Properties` -> `C/C++`. Select `All Configurations` within the `Configuration` selector. Select `All Platforms` within the `Platform` selector. For the `Warning Level` value input `Level4 (/W4)`. Click `Apply`
