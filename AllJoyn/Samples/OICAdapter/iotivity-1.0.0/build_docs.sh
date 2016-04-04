#!/bin/bash

# Fail script on any failure
set -e

# Move to script direotory
pushd `dirname $0` > /dev/null

rm -rf docs

# JavaDoc now... from Uze's script
ANDROID_JAR="$ANDROID_HOME/platforms/android-21/android.jar"

if [ ! -e "$ANDROID_JAR" ]; then
    echo "Android platform not found. Expected '$ANDROID_JAR'"
    exit 1
fi

BASE_PATH="android/android_api/base/src/main/java/"
BASE_PKG="org.iotivity.base"

TM_PATH="service/things-manager/sdk/java/src/"
TM_PKG="org.iotivity.service.tm"

SSM_PATH="service/soft-sensor-manager/SDK/java/"
SSM_PKG="org.iotivity.service.ssm"

PPM_PATH="service/protocol-plugin/plugin-manager/src/Android/src"
PPM_PKG="service/protocol-plugin/plugin-manager/src/Android/src/org/iotivity/service/ppm/PluginManager.java service/protocol-plugin/plugin-manager/src/Android/src/org/iotivity/service/ppm/Plugin.java"
# PPM_PKG="org.iotivity.service.ppm"

javadoc -splitindex \
        -d ./docs/java \
        -sourcepath $BASE_PATH:$TM_PATH:$SSM_PATH $BASE_PKG $TM_PKG $SSM_PKG $PPM_PKG \
        -classpath $ANDROID_JAR

# Doxygen now...
# NOTE: For now this is a workaround since I am ready to modify the doxygen setup
pushd resource/docs > /dev/null
doxygen

# Check for warnings or errors
if [ -s doxygen.log ]; then
    echo "Errors running doxygen. Review doxygen.log"
    exit 2
fi

popd > /dev/null
rm -rf docs/cxx
mv resource/docs/docs/html docs/cxx

popd > /dev/null
