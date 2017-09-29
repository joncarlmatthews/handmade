@echo off
REM Turn off comments being outputted to the command line.^

REM Call vcvarsall.bat so we can run "cl" from the command line.^
REM cl is the VS build tool for compiling C/C++ files^
REM Pass the x64 flag so cl is set to compile in 64 bit mode^
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x64
