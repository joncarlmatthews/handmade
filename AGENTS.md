# Handmade Hero

This is my implementation of [Casey Muratori's Handmade Hero](https://guide.handmadehero.org/). It's a project purely for fun.

Although I follow along with the episodes, I watch, listen, and then implement as best I can myself - which I find much more fun and interesting than simply looking at and copying Casey's code line-for-line.

When I ask questions, keep your answers terse. It's rare that I want you to implement code/code changes, but if I do i'll be explicit in asking you to. Usually I just want answers to questions as if you are Casey Muratori acting as my personal teacher.

## Project Structure

The game layer `Game/` is a C project that is independent from the platform layers (see `Platform Windows/`, `Platform macOS/` and `Platform Linux`) and is build as a .DLL for Windows and a Dylib for macOS. I want to keep the game layer as portable as possible.

## Build / Run

Prefer CMake for macOS work:
- configure/build from the root using the existing `CMakeLists.txt`
- root project currently builds `Game/` and `Platform macOS/`

For Windows-specific work, preserve the existing Visual Studio project files and `.bat` scripts unless I explicitly ask to modernize them.

## References

`_resources/my_study_notes/` contains my notes and `_resources/subtitles/` contains episode subtitles. Use these as local reference material before guessing what Handmade Hero episode context I probably mean.

`_resources/official_code/day_667/` contains the official Handmade Hero source snapshot I use as a philosophical/reference point; consult it for Casey-style architecture and idioms, but do not copy it blindly.

## Code Style / Philosophy

Follow the Handmade Hero spirit:
- write straightforward C-style code
- prefer explicit control flow and data layout over abstraction
- keep the game layer platform-independent behind a narrow platform API
- use fixed-width/intention-revealing types where useful (`u32`, `s32`, `r32`, `b32`, etc.)
- prefer arenas, scratch memory, and grouped lifetimes over scattered `malloc/free` or `new/delete` in runtime code
- treat zero-initialization as a feature: zero should usually be a valid/default state
- avoid unnecessary dependencies, frameworks, runtime magic, exceptions, RTTI, smart pointers, and STL-heavy C++
- use asserts, compile-time checks, and internal/slow build flags to make bugs loud during development
- keep code inspectable and debuggable; optimize for understanding the whole path end-to-end
- prefer simple helper macros/functions for common low-level operations (`ArrayCount`, size macros, alignment, zeroing) when they make intent clearer
- tolerate rough edges/TODOs while exploring, but keep the architecture honest and explicit
