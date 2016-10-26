#!/bin/bash

set -e

# Change to extlibs directory
cd "$(dirname "$0")"

EXTDIR=$(pwd)

# Pick the preferred version of boost to use
BOOST_MAJOR=1
BOOST_MINOR=57
BOOST_REVISION=0

BOOST_VERSION="${BOOST_MAJOR}.${BOOST_MINOR}.${BOOST_REVISION}"

# Determine the architecture
HOST_ARCH=$(arch)

if [ "${HOST_ARCH}" != "x86_64" ];
then
    HOST_ARCH="x86"
fi

HOST_ARCH="linux-${HOST_ARCH}"

function cloneBoost {
    echo "Removing old boost repo..."
    rm -rf boost
    echo "Cloning boost from GIT HUB..."
    git clone --recursive https://github.com/boostorg/boost.git boost
}

function buildBoost {
    if [ ! -d "boost" ]; then
        cloneBoost
    fi

    # Determine the
    TOOLCHAIN=${ANDROID_NDK}/toolchains/${TOOLSET}-${VERSION}/prebuilt/${HOST_ARCH}/bin

    OLDPATH=$PATH
    PATH=$TOOLCHAIN:$PATH

    rm -f boost.log

    pushd boost
    echo "Checking out boost v${BOOST_VERSION}..."
    git checkout --force -B boost-${BOOST_VERSION} tags/boost-${BOOST_VERSION}                                   &>> ../boost.log
    git submodule foreach --recursive git checkout --force -B boost-${BOOST_VERSION} tags/boost-${BOOST_VERSION} &>> ../boost.log
    echo "Reset and clean all modular repositories..."
    git reset --hard HEAD                                     >> ../boost.log
    git clean -d --force                                      >> ../boost.log
    git clean -d --force -x                                   >> ../boost.log
    git submodule foreach --recursive git reset --hard HEAD   >> ../boost.log
    git submodule foreach --recursive git clean --force -d    >> ../boost.log
    git submodule foreach --recursive git clean --force -d -x >> ../boost.log
    echo "Copying user configs to boost..."
    cp ${EXTDIR}/../resource/patches/boost/${TOOLSET}/user-config.jam tools/build/src/user-config.jam
    echo "Boostrapping boost..."
    ./bootstrap.sh
    echo "Building..."
    ./b2 -q \
        target-os=linux \
        link=static \
        threading=multi \
        --layout=system \
        --build-type=minimal \
        -s PLATFORM=${PLATFORM} \
        -s VERSION=${VERSION} \
        --prefix="${EXTDIR}/../out/boost" \
        --includedir="${INCPATH}" \
        --libdir="${LIBPATH}" \
        --build-dir="$(pwd)/build" \
        --with-thread \
        --with-program_options \
        headers install
    popd

    if [ ! -d "${INCPATH}" ];
    then
        echo "Copying headers to android include directory..."
        mkdir -p ${INCPATH}
        cp --recursive --dereference boost/boost ${INCPATH}
    fi

    PATH=$OLDPATH
}

function checkBoost {
    PLATFORM=$1
    TOOLSET=$2
    VERSION=$3

    INCPATH="${EXTDIR}/../out/android/include"
    LIBPATH="${EXTDIR}/../out/android/lib/${TOOLSET}"

    if [ ! -d "${LIBPATH}" ];
    then
        buildBoost
    fi
}

checkBoost android-19 arm-linux-androideabi 4.9
checkBoost android-19 x86 4.9
