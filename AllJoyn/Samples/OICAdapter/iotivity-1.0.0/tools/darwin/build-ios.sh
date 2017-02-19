#!/bin/bash

# change this to what version of Xcode you have installed

scons TARGET_OS=ios TARGET_ARCH=armv7 RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=armv7s RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=arm64 RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=i386  RELEASE=false
scons TARGET_OS=ios TARGET_ARCH=x86_64 RELEASE=false
