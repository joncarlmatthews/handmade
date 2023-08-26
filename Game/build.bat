@echo off

REM =========================================================================================
REM
REM Builds the Game DLL
REM 
REM You need to set the shell environment by running shell_x64.bat or shell_x86.bat once per
REM shell before running this build script.
REM 
REM Usage: build.bat <Configuration> <Platform>
REM E.g. build.bat Release x86
REM
REM To view the build options Visual Studio is using:
REM 
REM Options > Projects and Solutions > Build and Run > MSBuild project build output verbosity.
REM 
REM Select "Detailed"
REM 
REM You'll then see the flsgs used within the Build dropdown of the Output Window
REM
REM List of all env variables: https://learn.microsoft.com/en-us/cpp/build/reference/common-macros-for-build-commands-and-properties?view=msvc-170
REM 
REM =========================================================================================

IF [%1]==[] GOTO usage
IF [%2]==[] GOTO usage

SET BuildToolsVersion=vc143

SET ConfigurationArg=%1
SET PlatformArg=%2

if %ConfigurationArg% == Release (
    SET Configuration=Release
) else if  %ConfigurationArg% == Debug (
    SET Configuration=Debug
) else (
    GOTO configuration_usage
)

SET Timestamp=%date:~6,4%-%date:~3,2%-%date:~0,2%-%time:~0,2%-%time:~3,2%-%time:~6,2%

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

if "%3"=="1" (
    ECHO  - running clang-tidy code analysis
    SET CodeAnalysis=false
)else if "%3"=="2" (
    ECHO  - running Windows code analysis
    SET CodeAnalysis=true
)else (
    ECHO  - skipping code analysis
    SET CodeAnalysis=false
)

if "%4"=="1" (
    ECHO  - copying assets
    SET CopyAssets=true
)else (
    ECHO  - skipping asset copy
    SET CopyAssets=false
)

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

REM folders for the additional includes
SET StartupProjectFolder=%~dp0..\Startup\

REM create the directories (if they dont already exist)
IF not exist %ProjectFolder% ( mkdir %ProjectFolder% )
IF not exist %BuildArchFolder% ( mkdir  %BuildArchFolder% )
IF not exist %BuildConfigurationFolder% ( mkdir %BuildConfigurationFolder% )
IF not exist %IntermediatesRootFolder% ( mkdir %IntermediatesRootFolder% )
IF not exist %IntermediatesProjectFolder% ( mkdir  %IntermediatesProjectFolder% )
IF not exist %IntermediatesArchFolder% ( mkdir %IntermediatesArchFolder% )
IF not exist %IntermediatesConfigurationFolder% ( mkdir %IntermediatesConfigurationFolder% )

REM delete any old PDB files from any previous build
del %BuildConfigurationFolder%Game_*.pdb

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
SET StartupProjectFolder=%StartupProjectFolder:\=\\%

REM Debug:
REM ECHO "%BuildConfigurationFolder%"
REM ECHO %IntermediatesConfigurationFolder%

REM shorthand copy of the IntermediatesConfigurationFolder variable name
REM for handy use in the link.exe call
SET icf=%IntermediatesConfigurationFolder%

REM Enabling Code Analysis. Command to match setting in Project > Properties > Code Analysis
SET codeAnalysisCompilerFlags=
if %CodeAnalysis% == true (
    if %Platform% == x86 (
        SET codeAnalysisCompilerFlags=/analyze /analyze:ruleset"%VCInstallDir%..\Team Tools\Static Analysis Tools\Rule Sets\NativeRecommendedRules.ruleset" /analyze:plugin"%VCToolsInstallDir%bin\HostX86\x86\EspXEngine.dll"
    )else if %Platform% == x64 (
        SET codeAnalysisCompilerFlags=/analyze /analyze:ruleset"%VCInstallDir%..\Team Tools\Static Analysis Tools\Rule Sets\NativeRecommendedRules.ruleset" /analyze:plugin"%VCToolsInstallDir%bin\HostX64\X64\EspXEngine.dll"
    )
)

SET CompilerFlags=""
SET LinkerFlags=""

REM 32-bit builds
IF %Platform% == x86 (

    IF %Configuration% == Debug (

        SET CompilerFlags=/c /Zi /JMC /nologo /W4 /WX /diagnostics:column /sdl /Od /Oy- /D WIN32 /D _DEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" %codeAnalysisCompilerFlags% /external:W4 /Gd /TP /FC /errorReport:prompt /wd4201 /wd4100 /wd4505 /D HANDMADE_LOCAL_BUILD=1

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL /ILK:"%IntermediatesConfigurationFolder%Game.ilk" /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X86 /DLL
    )

    IF %Configuration% == Release (

        SET CompilerFlags=/c /Zi /nologo /W4 /WX /diagnostics:column /sdl /O2 /Oi /Oy- /GL /D WIN32 /D NDEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" %codeAnalysisCompilerFlags% /external:W4 /Gd /TP /FC /errorReport:prompt /wd4201 /wd4100 /wd4505

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL:NO /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG:incremental /LTCGOUT:"%IntermediatesConfigurationFolder%Game.iobj" /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X86 /SAFESEH /DLL
    )
)

IF %Platform% == x64 (

    IF %Configuration% == Debug (

        REM updated.

        SET CompilerFlags=/c /Zi /JMC /nologo /W4 /WX /diagnostics:column /sdl /Od /D _DEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /D HANDMADE_LOCAL_BUILD /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /external:W4 /Gd /TP /FC /errorReport:prompt /wd4201 /wd4100 /wd4505 /I"%StartupProjectFolder%" /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%%BuildToolsVersion%_%Timestamp%.pdb" %codeAnalysisCompilerFlags%

        SET LinkerFlags=/OUT:"%BuildConfigurationFolder%Game.dll" /MANIFEST /NXCOMPAT /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /DYNAMICBASE "startup.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /IMPLIB:"%BuildConfigurationFolder%Game.lib" /DEBUG /DLL /MACHINE:X64 /INCREMENTAL /SUBSYSTEM:WINDOWS /MANIFESTUAC:NO /ManifestFile:"%IntermediatesConfigurationFolder%Game.dll.intermediate.manifest" /ERRORREPORT:PROMPT /ILK:"%IntermediatesConfigurationFolder%Game.ilk" /NOLOGO /LIBPATH:"%BuildConfigurationFolder%" /TLBID:1
    )

    IF %Configuration% == Release (

        SET CompilerFlags=/c /Zi /nologo /W4 /WX /diagnostics:column /sdl /O2 /Oi /GL /D NDEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" %codeAnalysisCompilerFlags% /external:W4 /Gd /TP /FC /errorReport:prompt /wd4201 /wd4100 /wd4505

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL:NO /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG:incremental /LTCGOUT:"%IntermediatesConfigurationFolder%Game.iobj" /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X64 /DLL
    )
)

REM Copy the data assets
if %CopyAssets% == true (
    robocopy "%DataFolder%" "%BuildConfigurationFolder%data" /E /PURGE /MIR
)

REM Compile the source code
cl %CompilerFlags% %~dp0game.cpp %~dp0intrinsics.cpp %~dp0utility.cpp %~dp0memory.cpp %~dp0player.cpp %~dp0world.cpp %~dp0tilemap.cpp %~dp0graphics.cpp %~dp0audio.cpp %~dp0filesystem.cpp %~dp0math.cpp

REM Run the linker
link %LinkerFlags% %icf%game.obj %icf%intrinsics.obj %icf%utility.obj %icf%memory.obj %icf%player.obj %icf%world.obj %icf%tilemap.obj %icf%graphics.obj %icf%audio.obj %icf%filesystem.obj %icf%math.obj

GOTO :eof

:usage
ECHO Usage: %0 ^<Configuration^> ^<Platform^> ^<CopyAssets^> 
exit /B 1

:configuration_usage
ECHO Invalid configuration. Supported configurations: Debug, Release
exit /B 1

:platform_usage
ECHO Invalid platform architecture. Supported platforms: x86, x64
exit /B 1