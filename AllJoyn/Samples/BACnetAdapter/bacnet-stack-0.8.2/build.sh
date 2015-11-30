#!/bin/sh
# Build script for MinGW
echo "Build with MinGW and MSYS: mingw.sourceforge.net"
# set PATH=C:\MinGW\msys\1.0\bin;C:\MinGW\bin
# assumes rm, cp, size are already in path
CC=gcc
AR=ar
MAKE=make
export CC AR MAKE
make BACNET_PORT=win32 BUILD=release clean all > /dev/null

# Build for MinGW debug
# make BACNET_PORT=win32 BUILD=debug clean all

# Build for MinGW MS/TP
# make BACNET_PORT=win32 BACDL_DEFINE=-DBACDL_MSTP=1 clean all

# On Linux, install mingw32 and use this:
# make BACNET_PORT=win32 CC=i586-mingw32msvc-gcc AR=i586-mingw32msvc-ar clean all

echo "Complete!"
