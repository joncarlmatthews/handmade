# Day 023 - Looped Live Code Editing

When we hit the L key, for each frame, we blast the memory held within the input struct out to a file on disk. When we hit the L key again we stop writing out to the recording file. We write the entire input struct each time. At the time of writing the input struct is 520 bytes in size. Meaning we write 520 bytes worth of data for each frame recorded.

After we've finished recording we immediately set the playback flag to `true` meaning we then (for each frame) read out 520 bytes *into* the input struct (therefore overwriting it entirely each time) This has the affect of "playing back" the input recorded.  When an application calls `CreateFile` to open a file for the first time, Windows places the file pointer at the beginning of the file. As bytes are read from or written to the file, Windows advances the file pointer the number of bytes read. Therefore, on each loop, the next set of 520 bytes are read and so on, until there are no more bytes to read from the input recording. When we hit the end of the file (aka we've read all the bytes) we kick off the read again from the start, meaning that the input recording is on an indefinite loop until the debugging session is stopped.

In addition to recording and looping the input, we save a copy of the game state at the point when we start recording the input. Then, when we begin the recording playback, we set the game state back to this recorded game state snapshot. This means that we can loop the game state back to a moment in time over and over again (which helps with debugging) E.g. if you killed a boss during the recorded input, the boss would reappear when the recording input loops.

In Windows, each process has up to a certain amount of addressable memory (no matter how much physical memory is actually available for the process). The OS does the hard work of moving stuff in an out of memory between different processes.

In terms of the addressable memory limits; on 32-bit versions of Windows, a single process can map and address no more than 3GB of virtual memory at time.

In 64-bit versions of Windows, a 32-bit process can map and address no more than 4GB of virtual memory at a time. For 64-bit processes, the amount is typically around 7TB. As yet, I don't understand how we're able to set the base address to 4TB deep on the 32-bit build.
