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
) else if %PlatformArg% == Win32 (
    SET Platform=x86
    SET PlatformUpper=X86
) else (
    GOTO platform_usage
)

ECHO ============
ECHO Building %Platform%
ECHO ============

SET ProjectFolder=%~dp0"..\build\Platform Win32\"
SET ArchFolder=%ProjectFolder%%Platform%\
SET ConfigurationFolder=%ArchFolder%%Configuration%\

REM /nologo Suppresses display of sign-on banner.
REM /GS Buffers security check.
REM /sdl Enables additional security features and warnings.
REM /TP Specifies that all files are C++ source file (regardless of .c or .cpp extension)
REM /FC Display full path of source code files passed to cl.exe in diagnostic text.
REM /Oi Generates intrinsic functions.
REM /GR- disables run-time type information
REM /EHsc Catches only standard C++ exceptions and assumes that functions declared as extern "C" never throw a C++ exception.
REM /fp:precise Compiler preserves the source expression ordering and rounding properties of floating-point code when it generates and optimizes object code.
REM /permissive- Uses the conformance support in the current compiler version to determine which language constructs are non-conforming.
REM /diagnostics:column Controls the display of error and warning information, includes the column where the issue was found.
REM /D _UNICODE /D UNICODE Express support for Unicode strings
REM /WX Treats all warnings as errors.
REM /W4 Set compiler warning level to 4
REM /wd	Disables the specified warning.
REM /wd4201	Disables warning 4201 (permits specifying a structure without a declarator as members of another structure or union.)
REM /wd4201	Disables warning 4100 (permits unreferenced function parameters)
REM /wd4201	Disables warning 4505 (permits local and unreferenced functions aka dead code).
SET CommonCompilerFlags=/nologo /GS /sdl /TP /FC /Oi /GR- /EHsc /fp:precise /permissive- /diagnostics:column /D _UNICODE /D UNICODE /WX /W4 /wd4201 /wd4100 /wd4505

REM Build specific flags:
IF %Configuration% == Debug (

    REM /MTd Creates a debug multithreaded executable file using LIBCMTD.lib.
    REM /Zi Generates complete debugging information.
    REM /Od	Disables optimization.
    REM /RTC1 Enables run-time error checks
    REM /D Defines constants and macros.
    SET BuildSpecifcCompilerFlags=/MTd /Zi /Od /RTC1 /D _DEBUG /D HANDMADE_LOCAL_BUILD=1 /D HANDMADE_DEBUG_FPS=1 /D HANDMADE_DEBUG_AUDIO=1

) else (

    REM /MT Creates a multithreaded executable file using LIBCMT.lib.
    REM /O2 Maximize Speed of generated code.
    SET BuildSpecifcCompilerFlags=/MT /O2
)

REM /DLL Builds a DLL.
REM /INCREMENTAL Disable the Linker from running in incremental mode.
REM /OPT:REF eliminates functions and data that are never referenced
REM /NXCOMPAT Security feature that monitors and protects certain pages or regions of memory
REM /SUBSYSTEM:WINDOWS Application does not require a console, because it creates its own windows for interaction with the user.
REM /MACHINE Specifies the target platform.
REM /OUT Specifies the output file name.
SET CommonLinkerFlags=/DLL /INCREMENTAL:NO /OPT:REF /NXCOMPAT /SUBSYSTEM:WINDOWS /MACHINE:%Platform% /OUT:Game.dll

REM Build specific flags:
IF %Configuration% == Debug (

    REM /DEBUG Creates debugging information.
    SET BuildSpecificLinkerFlags=/DEBUG

) else (

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