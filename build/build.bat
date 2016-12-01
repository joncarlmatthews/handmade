@echo off
REM Turn off comments being outputted to the command line.^

REM cl /Febuild/win32_handmade.exe /P /Fibuild/win32_handmade.obj win32_handmade.cpp^
cl -Zi ../win32_handmade.cpp User32.lib