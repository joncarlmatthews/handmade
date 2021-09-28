@echo off

REM /nologo Suppresses display of sign-on banner.
REM /GS Buffers security check.
REM /sdl Enables additional security features and warnings.
REM /FC Display full path of source code files passed to cl.exe in diagnostic text.
REM /Oi Generates intrinsic functions.
REM /GR- disables run-time type information
REM /WX Treats all warnings as errors.
REM /W4 Set compiler warning level to 4
REM /wd	Disables the specified warning.
REM /wd4201	Disables warning 4201 (permits specifying a structure without a declarator as members of another structure or union.)
REM /wd4201	Disables warning 4100 (permits unreferenced function parameters)
set CommonCompilerFlags=/nologo /sdl /GS /FC /Oi /GR- /WX /W4 /wd4201 /wd4100 /MTd /Zi /Od

REM Build specific flags:
REM /MTd Creates a debug multithreaded executable file using LIBCMTD.lib.
REM /Zi Generates complete debugging information.
REM /Od	Disables optimization.
REM /D Defines constants and macros.
set BuildSpecifcCompilerFlags=/MTd /Zi /Od /D HANDMADE_LOCAL_BUILD=1 /D HANDMADE_DEBUG_FPS=1 /D HANDMADE_DEBUG_AUDIO=1

REM /DLL Builds a DLL.
REM /INCREMENTAL Disable the Linker from running in incremental mode.
REM /OPT:REF eliminates functions and data that are never referenced
REM /OUT Specifies the output file name.
set CommonLinkerFlags=/DLL /INCREMENTAL:NO /OPT:REF /OUT:Game.dll

REM /MACHINE Specifies the target platform.
REM /DEBUG Creates debugging information.
set BuildSpecificLinkerFlags=/MACHINE:X64 /DEBUG

SET folder=%~dp0..\build\Game\x64\Debug\
cd %folder%

cl %CommonCompilerFlags% %BuildSpecifcCompilerFlags% %~dp0\handmade.cpp /link %CommonLinkerFlags% %BuildSpecificLinkerFlags%

cd %~dp0