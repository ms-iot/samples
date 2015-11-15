#!/bin/sh
# A script to download and build the libpifacedigital for PiFace
# The library is located at github.com/piface
# Since the PiFace library is GPLv3, we have to keep it separate.

git clone https://github.com/piface/libmcp23s17.git
git clone https://github.com/piface/libpifacedigital.git

# Build the library

make -C libmcp23s17
make -C libpifacedigital

