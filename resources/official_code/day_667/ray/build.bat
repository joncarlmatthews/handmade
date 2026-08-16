@echo off

ctime -begin ray.ctm

set CommonCompilerFlags=-arch:AVX2 -Od -MTd -nologo -fp:fast -fp:except- -Gm- -GR- -EHa- -Zo -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -FC -Z7
set CommonCompilerFlags=-DCOMPILER_MSVC -D_CRT_SECURE_NO_WARNINGS -DRAY_WIN32=1 %CommonCompilerFlags%
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib gdi32.lib winmm.lib opengl32.lib

IF NOT EXIST ..\..\build mkdir ..\..\build
pushd ..\..\build

cl %CommonCompilerFlags% ..\handmade\ray\ray.cpp /link %CommonLinkerFlags%
set LastError=%ERRORLEVEL%
popd

ctime -end ray.ctm %LastError%
IF NOT %LastError%==0 GOTO :end

pushd data
REM ..\..\..\build\ray.exe
REM start test.bmp
popd

:end
