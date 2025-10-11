@echo off

SET options= -Zi -EHa -DDEBUG_BUILD=1
if "%~1"=="release" (SET options=-O2 -EHsc -DRELEASE_BUILD=1)

if exist build\ (
    echo exist
) else (
    mkdir build
)

cl %options% main.c /link /LIBPATH:..\lib msvcrt.lib Gdi32.lib WinMM.lib kernel32.lib shell32.lib User32.lib /OUT:build\main.exe 

