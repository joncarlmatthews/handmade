@echo off
REM Turn off comments being outputted to the command line.^

echo.
echo CUSTOM BUILD SCRIPT (32-bit)
echo.
echo *Notice: Check that vcvarsall.bat has been set to 32-bit mode (shell_x86.bat)*
echo.

pushd build
if not exist "Win32" md "Win32"
pushd "Win32"
cl -FC -Zi ..\..\win32_handmade.cpp /link User32.lib Gdi32.lib
popd
popd