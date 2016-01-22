@echo off
echo Build with MinGW and MSYS: mingw.sourceforge.net
rem set PATH=C:\MinGW\msys\1.0\bin;C:\MinGW\bin
rem assumes rm, cp, size are already in path
set CC=gcc
set AR=ar
set MAKE=make
rem make BACNET_PORT=win32 BUILD=release -f test.mak clean all
make -s -f test.mak clean all

