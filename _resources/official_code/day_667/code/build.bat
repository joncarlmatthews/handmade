@echo off

ctime -begin handmade_hero.ctm

REM -d1reportTime
set CommonCompilerFlags=-diagnostics:column -WL -O2 -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7 -GS- -Gs9999999
set CommonCompilerFlags=-DHANDMADE_INTERNAL=1 -DHANDMADE_SLOW=1 -DHANDMADE_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags=-STACK:0x100000,0x100000 -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib bcrypt.lib kernel32.lib

IF NOT EXIST ..\data mkdir ..\data
IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

del *.pdb > NUL 2> NUL

REM Simple preprocessor
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\REM simple_preprocessor.cpp /link %CommonLinkerFlags%
REM pushd ..\handmade\code
REM ..\..\build\simple_preprocessor.exe > handmade_generated.h
REM popd

REM PNG parser
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\test_png.cpp /link %CommonLinkerFlags%

REM Sampling generator
cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\hhsphere.cpp /link %CommonLinkerFlags%

REM Lighting profiler
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\hhlightprof.cpp /link %CommonLinkerFlags%

REM Simple compressor
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\simple_compressor.cpp /link %CommonLinkerFlags%

REM Asset file builder build
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\test_asset_builder.cpp /link %CommonLinkerFlags%

REM Win32 font extractor
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\hhfont.cpp /link %CommonLinkerFlags%

REM Asset file editor
REM cl %CommonCompilerFlags% -D_CRT_SECURE_NO_WARNINGS ..\handmade\code\hhaedit.cpp /link %CommonLinkerFlags%


REM 32-bit build
REM cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp /link -subsystem:windows,5.1 %CommonLinkerFlags%

REM 64-bit build
REM Optimization switches /wO2
echo WAITING FOR PDB > lock.tmp

REM Renderers
cl %CommonCompilerFlags% -MTd ..\handmade\code\win32_handmade_opengl.cpp -LD /link -incremental:no -opt:ref -PDB:win32_handmade_opengl_%random%.pdb -EXPORT:Win32LoadRenderer -EXPORT:Win32BeginFrame -EXPORT:Win32EndFrame gdi32.lib opengl32.lib user32.lib

REM Game
cl %CommonCompilerFlags% -MTd -I..\iaca-win64\ ..\handmade\code\handmade.cpp ..\handmade\code\handmade_msvc.c -Fmhandmade.map -LD /link -incremental:no -opt:ref -PDB:handmade_%random%.pdb -EXPORT:GameGetSoundSamples -EXPORT:GameUpdateAndRender -EXPORT:DEBUGGameFrameEnd
set LastError=%ERRORLEVEL%
del lock.tmp
cl %CommonCompilerFlags% ..\handmade\code\win32_handmade.cpp ..\handmade\code\handmade_msvc.c -Fmwin32_handmade.map /link /NODEFAULTLIB /SUBSYSTEM:windows %CommonLinkerFlags%

REM Renderer testbed
cl %CommonCompilerFlags% -MTd ..\handmade\code\win32_renderer_test.cpp /link /SUBSYSTEM:windows %CommonLinkerFlags%

popd

ctime -end handmade_hero.ctm %LastError%
