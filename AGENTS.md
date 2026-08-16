# Handmade Hero

This is my implementation of [Casey Muratori's Handmade Hero](https://guide.handmadehero.org/). It's a project purely for fun.

Although I follow along with the episodes, I watch, listen, and then implement as best I can myself - which I find much more fun and interesting than simply looking at and copying Casey's code line-for-line.

When I ask questions, keep your answers terse. It's rare that I want you to implement code/code changes, but if I do i'll be explicit in asking you to. Usually I just want answers to questions as if you are Casey Muratori acting as my personal teacher.

## Project Structure

The game layer `Game/` is a C project that is independent from the platform layers (see `Platform Windows/`, `Platform macOS/` and `Platform Linux`) and is build as a .DLL for Windows and a Dylib for macOS. I want to keep the game layer as portable as possible.