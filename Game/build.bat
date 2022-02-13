@echo off

REM =========================================================================================
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

SET Timestamp=%date:~6,4%-%date:~3,2%-%date:~0,2%-%time:~0,2%-%time:~3,2%-%time:~6,2%

REM PlatformFolder to match Visual Studio's build directory structures
if %PlatformArg% == x64 (
    SET Platform=x64
    SET PlatformFolder=x64
) else if %PlatformArg% == x86 (
    SET Platform=x86
    SET PlatformFolder=Win32
) else if %PlatformArg% == Win32 (
    SET Platform=x86
    SET PlatformFolder=Win3
) else (
    GOTO platform_usage
)

ECHO ============
ECHO Building %Platform%
ECHO ============

REM Root build folder for project
SET ProjectFolder=%~dp0..\build\Win32\

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

REM delete any old PDB files from any previous build
del %BuildConfigurationFolder%Game_*.pdb

REM double backslash directories (cl.exe and link.exe need the directories to be double backslashed)
SET ProjectFolder=%ProjectFolder:\=\\%
SET BuildArchFolder=%BuildArchFolder:\=\\%
SET BuildConfigurationFolder=%BuildConfigurationFolder:\=\\%
SET IntermediatesRootFolder=%IntermediatesRootFolder:\=\\%
SET IntermediatesProjectFolder=%IntermediatesProjectFolder:\=\\%
SET IntermediatesArchFolder=%IntermediatesArchFolder:\=\\%
SET IntermediatesConfigurationFolder=%IntermediatesConfigurationFolder:\=\\%

REM Debug:
REM ECHO "%BuildConfigurationFolder%"
REM ECHO %IntermediatesConfigurationFolder%

SET CompilerFlags=""
SET LinkerFlags=""

REM 32-bit builds
IF %Platform% == x86 (

    IF %Configuration% == Debug (

        SET CompilerFlags=/c /ZI /JMC /nologo /W4 /WX /diagnostics:column /sdl /Od /Oy- /D WIN32 /D _DEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" /external:W4 /Gd /TP /analyze- /FC /errorReport:prompt /wd4201 /wd4100 /wd4505 /D HANDMADE_LOCAL_BUILD=1

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL /ILK:"%IntermediatesConfigurationFolder%Game.ilk" /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X86 /DLL %IntermediatesConfigurationFolder%game.obj
    )

    IF %Configuration% == Release (

        SET CompilerFlags=/c /Zi /nologo /W4 /WX /diagnostics:column /sdl /O2 /Oi /Oy- /GL /D WIN32 /D NDEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" /external:W4 /Gd /TP /analyze- /FC /errorReport:prompt /wd4201 /wd4100 /wd4505

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL:NO /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG:incremental /LTCGOUT:"%IntermediatesConfigurationFolder%Game.iobj" /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X86 /SAFESEH /DLL %IntermediatesConfigurationFolder%game.obj
    )
)

IF %Platform% == x64 (

    IF %Configuration% == Debug (

        SET CompilerFlags=/c /ZI /JMC /nologo /W4 /WX /diagnostics:column /sdl /Od /D _DEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /RTC1 /MDd /GS /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" /external:W4 /Gd /TP /FC /errorReport:prompt /wd4201 /wd4100 /wd4505 /D HANDMADE_LOCAL_BUILD=1

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL /ILK:"%IntermediatesConfigurationFolder%Game.ilk" /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X64 /DLL %IntermediatesConfigurationFolder%game.obj
    )

    IF %Configuration% == Release (

        SET CompilerFlags=/c /Zi /nologo /W4 /WX /diagnostics:column /sdl /O2 /Oi /GL /D NDEBUG /D GAME_EXPORTS /D _WINDOWS /D _USRDLL /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /permissive- /Zc:wchar_t /Zc:forScope /Zc:inline /Fo"%IntermediatesConfigurationFolder%" /Fd"%IntermediatesConfigurationFolder%vc142_%Timestamp%.pdb" /external:W4 /Gd /TP /FC /errorReport:prompt /wd4201 /wd4100 /wd4505

        SET LinkerFlags=/ERRORREPORT:PROMPT /OUT:"%BuildConfigurationFolder%Game.dll" /INCREMENTAL:NO /NOLOGO kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /MANIFESTUAC:NO /manifest:embed /DEBUG /PDB:"%BuildConfigurationFolder%Game_%Timestamp%.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG:incremental /LTCGOUT:"%IntermediatesConfigurationFolder%Game.iobj" /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"%BuildConfigurationFolder%Game.lib" /MACHINE:X64 /DLL %IntermediatesConfigurationFolder%game.obj
    )
)

REM Compile the source code
cl %CompilerFlags% %~dp0game.cpp

REM Run the linker
link %LinkerFlags%

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