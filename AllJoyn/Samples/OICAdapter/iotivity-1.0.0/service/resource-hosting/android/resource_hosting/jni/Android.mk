LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../android/android_api/base/libs/$(TARGET_ARCH_ABI)
LOCAL_MODULE := ca_interface
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libca-interface.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := ca
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libconnectivity_abstraction.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := oc_logger_core
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboc_logger_core.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := oc_logger
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboc_logger.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := octbstack
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboctbstack.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := oc
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../android/android_api/base/libs/$(TARGET_ARCH_ABI)
LOCAL_MODULE := ocstack-jni
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libocstack-jni.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := rcsCommon
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/librcs_common.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := rcsClient
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/librcs_client.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := rcsServer
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/librcs_server.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := resourceHosting
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libresource_hosting.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ResourceHosing_JNI
LOCAL_CPPFLAGS := -std=c++0x -frtti -fexceptions

LOCAL_STATIC_LIBRARIES := ca_interface
LOCAL_STATIC_LIBRARIES += ca
LOCAL_STATIC_LIBRARIES += oc_logger_core
LOCAL_STATIC_LIBRARIES += oc_logger
LOCAL_STATIC_LIBRARIES += octbstack
LOCAL_STATIC_LIBRARIES += oc
LOCAL_STATIC_LIBRARIES += ocstack-jni
LOCAL_STATIC_LIBRARIES += rcsCommon
LOCAL_STATIC_LIBRARIES += rcsClient
LOCAL_STATIC_LIBRARIES += rcsServer
LOCAL_STATIC_LIBRARIES += resourceHosting


OIC_SRC_DIR := ../../../..

LOCAL_C_INCLUDES := $(OIC_SRC_DIR)/resource/csdk/stack/include \
                    $(OIC_SRC_DIR)/resource/csdk/logger/include \
                    $(OIC_SRC_DIR)/resource/include \
                    $(OIC_SRC_DIR)/resource/c_common \
                    $(OIC_SRC_DIR)/resource/oc_logger/include \
                    $(OIC_SRC_DIR)/extlibs/boost/boost_1_58_0 \
                    $(OIC_SRC_DIR)/service/resource-encapsulation/include \
                    $(OIC_SRC_DIR)/service/resource-hosting/include \

LOCAL_SRC_FILES := ResourceHosing_JNI.cpp
include $(BUILD_SHARED_LIBRARY)
