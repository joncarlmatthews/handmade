# Day 025 - Finishing the Win32 Prototyping Layer

I refactored the game recording from using a file on disk to simply doing a memory swap. This means that when in a local build the platform layer allocates double the memory required to run the game. One block for the game itself, and one block for the recording. The reason for the refactoring is that copying memory is much quicker than writing the entire allocated game memory to a file on disk.

TCHAR:

A WCHAR if UNICODE is defined, a CHAR otherwise.

WCHAR:

A 16-bit Unicode character. For more information, see Character Sets Used By Fonts.
