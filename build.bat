@echo off
REM Turn off comments being outputted to the command line.^

echo *********************************
echo CUSTOM HANDMADE HERO BUILD SCRIPT!
echo *********************************

mkdir build
pushd build
cl -FC -Zi ..\win32_handmade.cpp /link User32.lib Gdi32.lib
popd