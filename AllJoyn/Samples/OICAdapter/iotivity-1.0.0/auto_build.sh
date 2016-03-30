#! /bin/bash

# Ideally we will capture the exit code of each step and try them all before failing
# the build script.  For now, use set -e and fail the build at first failure.
set -e

function build_all()
{
	if [ $(uname -s) = "Linux" ]
        then
		build_linux_unsecured $1 $2
		build_linux_secured $1 $2
		build_linux_unsecured_with_ra $1 $2
		build_linux_secured_with_ra $1 $2
		build_linux_unsecured_with_rd $1 $2
		build_linux_secured_with_rd $1 $2
	fi

	build_android $1 $2

	build_arduino $1 $2

	build_tizen $1 $2

	if [ $(uname -s) = "Darwin" ]
	then
		build_darwin $1 $2
	fi
}

function build_linux()
{
	build_linux_unsecured $1 $2

	build_linux_secured $1 $2
}

function build_linux_unsecured()
{
	echo "*********** Build for linux ************"
	scons RELEASE=$1 $2
}

function build_linux_secured()
{
	echo "*********** Build for linux with Security *************"
	scons RELEASE=$1 SECURED=1 $2
}

function build_linux_unsecured_with_ra()
{

	echo "*********** Build for linux With Remote Access *************"
	scons RELEASE=$1 WITH_RA=1 $2
}

function build_linux_secured_with_ra()
{
	echo "*********** Build for linux With Remote Access & Security ************"
	scons RELEASE=$1 WITH_RA=1 SECURED=1 $2
}

function build_linux_unsecured_with_rd()
{
	echo "*********** Build for linux With Resource Directory *************"
	scons RELEASE=$1 WITH_RD=1 $2
}

function build_linux_secured_with_rd()
{
	echo "*********** Build for linux With Resource Directory & Security ************"
	scons RELEASE=$1 WITH_RD=1 SECURED=1 $2
}

function build_android()
{
	# Note: for android, as oic-resource uses C++11 feature stoi and to_string,
	# it requires gcc-4.9, currently only android-ndk-r10(for linux)
	# and windows android-ndk-r10(64bit target version) support these features.

	build_android_x86 $1 $2
	build_android_armeabi $1 $2
}

function build_android_x86()
{
	echo "*********** Build for android x86 *************"
	scons TARGET_OS=android TARGET_ARCH=x86 RELEASE=$1 TARGET_TRANSPORT=IP $2
	scons TARGET_OS=android TARGET_ARCH=x86 RELEASE=$1 TARGET_TRANSPORT=BT $2
	scons TARGET_OS=android TARGET_ARCH=x86 RELEASE=$1 TARGET_TRANSPORT=BLE $2
}

function build_android_armeabi()
{
	echo "*********** Build for android armeabi *************"
	scons TARGET_OS=android TARGET_ARCH=armeabi RELEASE=$1 TARGET_TRANSPORT=IP $2
	scons TARGET_OS=android TARGET_ARCH=armeabi RELEASE=$1 TARGET_TRANSPORT=BT $2
	scons TARGET_OS=android TARGET_ARCH=armeabi RELEASE=$1 TARGET_TRANSPORT=BLE $2
}

function build_arduino()
{
	echo "*********** Build for arduino avr *************"
	scons resource TARGET_OS=arduino UPLOAD=false BOARD=mega TARGET_ARCH=avr TARGET_TRANSPORT=IP SHIELD=ETH RELEASE=$1 $2
	scons resource TARGET_OS=arduino UPLOAD=false BOARD=mega TARGET_ARCH=avr TARGET_TRANSPORT=IP SHIELD=WIFI RELEASE=$1 $2
	scons resource TARGET_OS=arduino UPLOAD=false BOARD=mega TARGET_ARCH=avr TARGET_TRANSPORT=BLE SHIELD=RBL_NRF8001 RELEASE=$1 $2

	echo "*********** Build for arduino arm *************"
	scons resource TARGET_OS=arduino UPLOAD=false BOARD=arduino_due_x TARGET_ARCH=arm TARGET_TRANSPORT=IP SHIELD=ETH RELEASE=$1 $2
	scons resource TARGET_OS=arduino UPLOAD=false BOARD=arduino_due_x TARGET_ARCH=arm TARGET_TRANSPORT=IP SHIELD=WIFI RELEASE=$1 $2
	# BLE support for the Arduino Due is currently unavailable.
}

function build_tizen()
{
	echo "*********** Build for Tizen *************"
	./gbsbuild.sh

	echo "*********** Build for Tizen CA lib and sample *************"
	scons -f resource/csdk/connectivity/build/tizen/SConscript TARGET_OS=tizen TARGET_TRANSPORT=IP LOGGING=true RELEASE=$1 $2

	echo "*********** Build for Tizen CA lib and sample with Security *************"
	scons -f resource/csdk/connectivity/build/tizen/SConscript TARGET_OS=tizen TARGET_TRANSPORT=IP LOGGING=true SECURED=1 RELEASE=$1 $2
}

function build_darwin() # Mac OSx and iOS
{
	echo "*********** Build for OSX *************"
	scons TARGET_OS=darwin SYS_VERSION=10.9 RELEASE=$1 $2

	echo "*********** Build for IOS i386 *************"
	scons TARGET_OS=ios TARGET_ARCH=i386 SYS_VERSION=7.0 RELEASE=$1 $2

	echo "*********** Build for IOS x86_64 *************"
	scons TARGET_OS=ios TARGET_ARCH=x86_64 SYS_VERSION=7.0 RELEASE=$1 $2

	echo "*********** Build for IOS armv7 *************"
	scons TARGET_OS=ios TARGET_ARCH=armv7 SYS_VERSION=7.0 RELEASE=$1 $2

	echo "*********** Build for IOS armv7s *************"
	scons TARGET_OS=ios TARGET_ARCH=armv7s SYS_VERSION=7.0 RELEASE=$1 $2

	echo "*********** Build for IOS arm64 *************"
	scons TARGET_OS=ios TARGET_ARCH=arm64 SYS_VERSION=7.0 RELEASE=$1 $2
}

function unit_tests()
{
	echo "*********** Unit test Start *************"
	scons resource RELEASE=false -c
	scons resource LOGGING=false RELEASE=false
	scons resource TEST=1 RELEASE=false
	echo "*********** Unit test Stop *************"
}

function  help()
{
	echo "Usage:"
        echo "  build:"
        echo "     `basename $0` <target_build>"
	echo "      Allowed values for <target_build>: all, linux_unsecured, linux_secured, linux_unsecured_with_ra, linux_secured_with_ra, linux_unsecured_with_rd, linux_secured_with_rd, android, arduino, tizen, darwin"
	echo "      Note: \"linux\" will build \"linux_unsecured\", \"linux_secured\", \"linux_unsecured_with_ra\", \"linux_secured_with_ra\", \"linux_secured_with_rd\" & \"linux_unsecured_with_rd\"."
	echo "      Any selection will build both debug and release versions of all available targets in the scope you've"
	echo "      selected. To choose any specific command, please use the SCons commandline directly. Please refer"
	echo "      to [IOTIVITY_REPO]/Readme.scons.txt."
        echo "  clean:"
        echo "     `basename $0` -c"
}

# Suppress "Reading ..." message and enable parallel build
export SCONSFLAGS="-Q -j 4"

if [ $# -eq 1 ]
then
	if [ $1 = '-c' ]
	then
		build_all true $1
		build_all false $1
		exit 0
	elif [ $1 = 'all' ]
	then
		build_all true
		build_all false
		unit_tests
	elif [ $1 = 'linux' ]
	then
		build_linux true
		build_linux false
	elif [ $1 = 'linux_unsecured' ]
	then
		build_linux_unsecured true
		build_linux_unsecured false
	elif [ $1 = 'linux_secured' ]
	then
		build_linux_secured true
		build_linux_secured false
	elif [ $1 = 'linux_unsecured_with_ra' ]
	then
		build_linux_unsecured_with_ra true
		build_linux_unsecured_with_ra false
	elif [ $1 = 'linux_secured_with_ra' ]
	then
		build_linux_secured_with_ra true
		build_linux_secured_with_ra false
	elif [ $1 = 'linux_unsecured_with_rd' ]
	then
		build_linux_unsecured_with_rd true
		build_linux_unsecured_with_rd false
	elif [ $1 = 'linux_secured_with_rd' ]
	then
		build_linux_secured_with_rd true
		build_linux_secured_with_rd false
	elif [ $1 = 'android' ]
	then
		build_android true
		build_android false
	elif [ $1 = 'android_x86' ]
	then
        build_android_x86 true
        build_android_x86 false
	elif [ $1 = 'android_armeabi' ]
	then
        build_android_armeabi true
        build_android_armeabi false
	elif [ $1 = 'arduino' ]
	then
		build_arduino true
		build_arduino false
	elif [ $1 = 'tizen' ]
	then
		build_tizen true
		build_tizen false
	elif [ $1 = 'darwin' ]
	then
		build_darwin true
		build_darwin false
	elif [ $1 = 'unit_tests' ]
	then
		unit_tests
	else
		help
		exit -1
	fi
elif [ $# -eq 0 ]
then
	build_all true
	build_all false
	unit_tests
else
	help
	exit -1
fi

echo "===================== done ====================="
