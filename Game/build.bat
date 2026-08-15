@echo off

REM =========================================================================================
REM
REM Builds the Game DLL. Debug build only, as this is used as a tool to hot reload the game code
REM 
REM You need to set the shell environment by running shell_x64.bat or shell_x86.bat once per
REM shell before running this build script. 
REM 
REM Usage: build.bat <PlatformArch>
REM 
REM E.g. build.bat x86
REM
REM To view the build options Visual Studio is using:
REM 
REM Options > Projects and Solutions > Build and Run > MSBuild project build output verbosity.
REM 
REM Select "Detailed"
REM 
REM You'll then see the flags used within the Build dropdown of the Output Window
REM
REM List of all env variables: https://learn.microsoft.com/en-us/cpp/build/reference/common-macros-for-build-commands-and-properties?view=msvc-170
REM 
REM =========================================================================================

IF [%1]==[] GOTO usage

SET BuildToolsVersion=vc143
SET Configuration=Debug
SET Timestamp=%date:~6,4%-%date:~3,2%-%date:~0,2%-%time:~0,2%-%time:~3,2%-%time:~6,2%

SET PlatformArg=%1

REM PlatformFolder to match Visual Studio's build directory structures
if %PlatformArg% == x64 (
    SET Platform=x64
    SET PlatformFolder=x64
) else if %PlatformArg% == x86 (
    SET Platform=x86
    SET PlatformFolder=x86
) else if %PlatformArg% == Windows (
    SET Platform=x86
    SET PlatformFolder=x86
) else (
    GOTO platform_usage
)

ECHO =============
ECHO Building %Platform%
ECHO =============

REM Root build folder for Solution and Project
SET ProjectFolder=%~dp0..\build\Windows\
SET SolutionFolder=%~dp0..\

REM Solution level folders
SET DataFolder=%SolutionFolder%data

REM folders for the build location
SET BuildArchFolder=%ProjectFolder%%PlatformFolder%\
SET BuildConfigurationFolder=%BuildArchFolder%%Configuration%\

REM folders for the intermediates location
SET IntermediatesRootFolder=%ProjectFolder%intermediates\
SET IntermediatesProjectFolder=%IntermediatesRootFolder%Game\
SET IntermediatesArchFolder=%IntermediatesProjectFolder%%PlatformFolder%\
SET IntermediatesConfigurationFolder=%IntermediatesArchFolder%%Configuration%\

REM create the directories (if they dont already exist)
IF not exist %ProjectFolder% ( mkdir %ProjectFolder% )
IF not exist %BuildArchFolder% ( mkdir  %BuildArchFolder% )
IF not exist %BuildConfigurationFolder% ( mkdir %BuildConfigurationFolder% )
IF not exist %IntermediatesRootFolder% ( mkdir %IntermediatesRootFolder% )
IF not exist %IntermediatesProjectFolder% ( mkdir  %IntermediatesProjectFolder% )
IF not exist %IntermediatesArchFolder% ( mkdir %IntermediatesArchFolder% )
IF not exist %IntermediatesConfigurationFolder% ( mkdir %IntermediatesConfigurationFolder% )

REM double backslash directories. (cl.exe and link.exe need the directories to be double backslashed)
SET ProjectFolder=%ProjectFolder:\=\\%
SET SolutionFolder=%SolutionFolder:\=\\%
SET DataFolder=%DataFolder:\=\\%
SET BuildArchFolder=%BuildArchFolder:\=\\%
SET BuildConfigurationFolder=%BuildConfigurationFolder:\=\\%
SET IntermediatesRootFolder=%IntermediatesRootFolder:\=\\%
SET IntermediatesProjectFolder=%IntermediatesProjectFolder:\=\\%
SET IntermediatesArchFolder=%IntermediatesArchFolder:\=\\%
SET IntermediatesConfigurationFolder=%IntermediatesConfigurationFolder:\=\\%

REM Debug:
REM ECHO "%BuildConfigurationFolder%"
REM ECHO %IntermediatesConfigurationFolder%

REM shorthand copy of the IntermediatesConfigurationFolder variable name
REM for handy use in the link.exe call
SET icf=%IntermediatesConfigurationFolder%

SET CompilerFlags=""
SET LinkerFlags=""

REM /c Just compile, don't also link
REM /nologo Suppresses the display of the copyright banner when the compiler starts up
REM /diagnostics:column Compile error format. Include the column where the issue was found.
REM /sdl Enables recommended Security Development Lifecycle (SDL) checks.
REM /Od Turns off all optimizations in the program and speeds compilation.
REM /D Preprocessor definitions
REM /RTC1 enable run-time error checks
REM /MDd Include the DLL-specific version of the run-time library in the .obj file
REM /GS Buffer security checks and stack buffer overrun protection
REM /fp:precise Precise floating point spec implementation
REM /Gd cdecl calling convention for functions
REM /TC Treat all source files as C, not C++
REM /std:clatest Use MSVC's latest available C standard mode. At time of writing this is the C23-era mode.

REM 32-bit builds
IF %Platform% == x86 (

    SET CompilerFlags=/c /TC /std:clatest /sdl /Od /RTC1 /MDd /GS /fp:precise /Gd ^
        /diagnostics:column /nologo ^
        /W4 /WX /wd4201 /wd4100 /wd4505 ^
        /D WIN32 /D _DEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE ^
        /Fo"%IntermediatesConfigurationFolder%" ^
        /Fd"%IntermediatesConfigurationFolder%%BuildToolsVersion%_%Timestamp%.pdb"

    SET LinkerFlags=/OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL /ILK:"%IntermediatesConfigurationFolder%Game.ilk" /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X86 /DLL
)

REM 64-bit builds
IF %Platform% == x64 (

    SET CompilerFlags=/c /TC /std:clatest /nologo /W4 /WX /diagnostics:column /sdl /Od /D _DEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /RTC1 /MDd /GS /fp:precise /Gd /FC /wd4201 /wd4100 /wd4505 /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%%BuildToolsVersion%_%Timestamp%.pdb"

    SET LinkerFlags=/OUT:"%BuildConfigurationFolder%Game.dll" /MANIFEST /NXCOMPAT /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /DYNAMICBASE "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /IMPLIB:"%BuildConfigurationFolder%Game.lib" /DEBUG /DLL /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:WINDOWS /MANIFESTUAC:NO /ManifestFile:"%IntermediatesConfigurationFolder%Game.dll.intermediate.manifest" /ILK:"%IntermediatesConfigurationFolder%Game.ilk" /NOLOGO /LIBPATH:"%BuildConfigurationFolder%" /TLBID:1
)

REM Compile the source code
cl %CompilerFlags% %~dp0game.c %~dp0intrinsics.c %~dp0utility.c %~dp0memory.c %~dp0player.c %~dp0world.c %~dp0tilemap.c %~dp0graphics.c %~dp0audio.c %~dp0filesystem.c %~dp0math.c

REM Link all of the compiled code together into a DLL
link %LinkerFlags% %icf%game.obj %icf%intrinsics.obj %icf%utility.obj %icf%memory.obj %icf%player.obj %icf%world.obj %icf%tilemap.obj %icf%graphics.obj %icf%audio.obj %icf%filesystem.obj %icf%math.obj

GOTO :eof

:usage
ECHO Usage: %0 ^<PlatformArch^>
exit /B 1

:platform_usage
ECHO Invalid platform architecture. Supported platforms: x86, x64
exit /B 1
