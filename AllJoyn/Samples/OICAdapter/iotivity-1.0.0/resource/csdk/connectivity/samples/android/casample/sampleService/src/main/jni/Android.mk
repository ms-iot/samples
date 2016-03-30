APP_PATH := $(call my-dir)
#specify project root path here wrt make file directory
PROJECT_ROOT_PATH	= ../../../../../../
include $(CLEAR_VARS)
LOCAL_PATH= $(PROJECT_ROOT_PATH)/../../../out/android/$(APP_ABI)/$(APP_OPTIM)/resource/csdk/connectivity/src
LOCAL_SRC_FILES = libconnectivity_abstraction.so
LOCAL_MODULE = CA
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_PATH= $(PROJECT_ROOT_PATH)/../../../out/android/$(APP_ABI)/$(APP_OPTIM)/resource/csdk/connectivity/samples/android
LOCAL_SRC_FILES = libRMInterface.so
LOCAL_MODULE = CASAMPLE
include $(PREBUILT_SHARED_LIBRARY)
