@echo off
REM MSVC compiler @see https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options?view=vs-2019

echo.
echo CUSTOM BUILD SCRIPT (64-bit)
echo.
echo *Notice: Check that vcvarsall.bat has been set to 64-bit mode (shell_x64.bat)*
echo.

pushd build
if not exist "x64" md "x64"
pushd "x64"
cl -FC -Zi ..\..\win32_handmade.cpp /link User32.lib Gdi32.lib Ole32.Lib
popd
popd