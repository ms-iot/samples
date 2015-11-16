@echo off
echo Build with MinGW and MSYS: mingw.sourceforge.net
rem set PATH=C:\MinGW\msys\1.0\bin;C:\MinGW\bin
rem assumes rm, cp, size are already in path
set CC=gcc
set AR=ar
set MAKE=make
make BACNET_PORT=win32 BUILD=release clean all

rem Build for MinGW debug
rem make BACNET_PORT=win32 BUILD=debug clean all

rem Build for MinGW MS/TP
rem make BACNET_PORT=win32 BACDL_DEFINE=-DBACDL_MSTP=1 clean all

rem On Linux, install mingw32 and use this:
rem make BACNET_PORT=win32 CC=i586-mingw32msvc-gcc AR=i586-mingw32msvc-ar clean all
