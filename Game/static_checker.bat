@echo off

set Wildcard=*.h *.cpp *.c *.ini

echo =============
echo STATICS FOUND
echo =============
findstr -s -n -l "static" %Wildcard%

echo =============
echo GLOBALS FOUND
echo =============
findstr -s -n -l "global_var" %Wildcard%
findstr -s -n -l "local_persist_var" %Wildcard%