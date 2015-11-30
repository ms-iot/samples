@echo off
echo Build with MinGW and MSYS: mingw.sourceforge.net
rem set PATH=C:\MinGW\msys\1.0\bin;C:\MinGW\bin
rem assumes rm, cp, size are already in path
set CC=gcc
set AR=ar
set MAKE=make
set TARGET_EXT=.exe
make clean all
