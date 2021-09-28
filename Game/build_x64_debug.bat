@echo off

REM =========================================================================================
REM You need to set the shell environment by running shell_x64.bat or shell_x86.bat once per
REM shell before running this build script.
REM Usage: build.bat <Configuration> <Platform>
REM E.g. build.bat Release x86
REM =========================================================================================

IF [%1]==[] GOTO usage
IF [%2]==[] GOTO usage

SET ConfigurationArg=%1
SET PlatformArg=%2

if %ConfigurationArg% == Release (
    SET Configuration=Release
) else if  %ConfigurationArg% == Debug (
    SET Configuration=Debug
) else (
    GOTO configuration_usage
)

if %PlatformArg% == x64 (
    SET Platform=x64
    SET PlatformUpper=X64
) else if %PlatformArg% == x86 (
    SET Platform=x86
    SET PlatformUpper=X86
) else (
    GOTO platform_usage
)

ECHO ============
ECHO Building %Platform%
ECHO ============

SET ProjectFolder=%~dp0..\build\Game\
SET ArchFolder=%ProjectFolder%%Platform%\
SET ConfigurationFolder=%ArchFolder%%Configuration%\

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
SET CommonCompilerFlags=/nologo /sdl /GS /FC /Oi /GR- /WX /W4 /wd4201 /wd4100

REM Build specific flags:
IF %Configuration% == Debug (

    REM /MTd Creates a debug multithreaded executable file using LIBCMTD.lib.
    REM /Zi Generates complete debugging information.
    REM /Od	Disables optimization.
    REM /D Defines constants and macros.
    SET BuildSpecifcCompilerFlags=/MTd /Zi /Od /D HANDMADE_LOCAL_BUILD=1 /D HANDMADE_DEBUG_FPS=1 /D HANDMADE_DEBUG_AUDIO=1

) else (

    REM /MT Creates a multithreaded executable file using LIBCMT.lib.
    SET BuildSpecifcCompilerFlags=/MT
)

REM /DLL Builds a DLL.
REM /INCREMENTAL Disable the Linker from running in incremental mode.
REM /OPT:REF eliminates functions and data that are never referenced
REM /OUT Specifies the output file name.
SET CommonLinkerFlags=/DLL /INCREMENTAL:NO /OPT:REF /MACHINE:%Platform% /OUT:Game.dll

REM Build specific flags:
IF %Configuration% == Debug (

    REM /MACHINE Specifies the target platform.
    REM /DEBUG Creates debugging information.
    SET BuildSpecificLinkerFlags=/DEBUG

) else (

    REM /MACHINE Specifies the target platform.
    REM /DEBUG Creates debugging information.
    SET BuildSpecificLinkerFlags=
)


IF not exist %ProjectFolder% ( mkdir %ProjectFolder% )
IF not exist %ArchFolder% ( mkdir  %ArchFolder% )
IF not exist %ConfigurationFolder% ( mkdir %ConfigurationFolder% )

cd %ConfigurationFolder%

cl %CommonCompilerFlags% %BuildSpecifcCompilerFlags% %~dp0\handmade.cpp /link %CommonLinkerFlags% %BuildSpecificLinkerFlags%

cd %~dp0

GOTO :eof

:usage
ECHO Usage: %0 ^<Configuration^> ^<Platform^>
exit /B 1

:configuration_usage
ECHO Invalid configuration. Supported configurations: Debug, Release
exit /B 1

:platform_usage
ECHO Invalid platform. Supported platforms: x86, x64
exit /B 1