@echo off
REM Turn off comments being outputted to the command line.^

REM cl -Zi .\win32_handmade.cpp /link User32.lib /out:"Debug\Handmade Hero.exe"^
mkdir build
cl -Zi .\win32_handmade.cpp /link User32.lib /out:"build\win32_handmade_debug.exe"^