#******************************************************************
# JAVA API generation 
#
# BASE
#
# RE ( Resource Encapsulation) : common, client, server
# TM ( Things Manager)
# RH ( Resource Hosting)
# easy setup
# simulator
# RC ( Resource Container)  next release
#
#-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=



#!/bin/bash

BASE_PATH="../../android/android_api/base/src/main/java/"
BASE_PKG="org.iotivity.base"

RE_PATH="../../service/resource-encapsulation/android/service/src/main/java/"
RE_COMMON_PKG=org.iotivity.service
RE_CLINET_PKG=org.iotivity.service.client
RE_SERVER_PKG=org.iotivity.service.server

TM_PATH="../../service/things-manager/sdk/java/src/"
TM_PKG=org.iotivity.service.tm

RH_PATH="../../service/resource-hosting/android/resource_hosting/src/"
RH_PKG=org.iotivity.ResourceHosting


EASYSETUP_PATH="../../service/easy-setup/sdk/mediator/android/EasySetupCore/src/main/java/"
EASYSETUP_PKG=org.iotivity.service.easysetup.mediator

SIMULATOR_PATH="../../service/simulator/java/sdk/src/"
SIMULATOR_COMMON_PKG=org.oic.simulator
SIMULATOR_CLIENT_PKG=org.oic.simulator.clientcontroller
SIMULATOR_SERVER_PKG=org.oic.simulator.serviceprovider

javadoc -public -splitindex -d ./Java_API -sourcepath \
    $BASE_PATH:$RE_PATH:$TM_PATH:$RH_PATH:$EASYSETUP_PATH:$SIMULATOR_PATH \
    $BASE_PKG $RE_COMMON_PKG  $RE_CLINET_PKG $RE_SERVER_PKG $TM_PKG $RH_PKG $EASYSETUP_PKG \
    $SIMULATOR_COMMON_PKG  $SIMULATOR_CLIENT_PKG $SIMULATOR_SERVER_PKG


