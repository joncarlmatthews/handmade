@echo off

REM /nologo Suppresses display of sign-on banner.
REM /GS Buffers security check.
REM /sdl Enables additional security features and warnings.
REM /FC Display full path of source code files passed to cl.exe in diagnostic text.
REM /MT Creates a multithreaded executable file using LIBCMT.lib.
REM /Oi Generates intrinsic functions.
REM /GR- disables run-time type information
REM /WX Treats all warnings as errors.
REM /W4 Set compiler warning level to 4
REM /wd	Disables the specified warning.
REM /wd4201	Disables warning 4201 (permits specifying a structure without a declarator as members of another structure or union.)
REM /wd4201	Disables warning 4100 (permits unreferenced function parameters)
REM /ZI Includes debug information in a program database compatible with Edit and Continue.
REM /Od	Disables optimization.
REM /D Defines constants and macros.

set CompilerFlags=/nologo /sdl /GS /FC /MT /Oi /GR- /WX /W4 /wd4201 /wd4100 /Zi /Od /DHANDMADE_LOCAL_BUILD=1 /DHANDMADE_DEBUG_FPS=1 /DHANDMADE_DEBUG_AUDIO=1

SET folder=%~dp0..\build\Game\x64\Debug\
cd %folder%

cl %CompilerFlags% %~dp0\handmade.cpp /link /INCREMENTAL:NO /OPT:REF /DLL /OUT:Game.dll

cd %~dp0