LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := android-oc_logger
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboc_logger.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := android-octbstack
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboctbstack.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := android-oc
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/liboc.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := android-connectivity_abstraction
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libconnectivity_abstraction.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../android/android_api/base/libs/$(TARGET_ARCH_ABI)
LOCAL_MODULE := android-ocstack-jni
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libocstack-jni.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../android/android_api/base/libs/$(TARGET_ARCH_ABI)
LOCAL_MODULE := android-ca-interface
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libca-interface.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_LIB_PATH := ../../../../../out/android/$(TARGET_ARCH_ABI)/release
LOCAL_MODULE := android-thingsmanager
LOCAL_SRC_FILES := $(OIC_LIB_PATH)/libTGMSDKLibrary.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_SRC_DIR := ../../../..
LOCAL_MODULE    := things-manager-jni

LOCAL_C_INCLUDES := $(LOCAL_PATH)/jniutil/inc \
                    $(OIC_SRC_DIR)/android/android_api/base/jni \
                    $(OIC_SRC_DIR)/resource/include \
                    $(OIC_SRC_DIR)/resource/c_common \
                    $(OIC_SRC_DIR)/resource/csdk/stack/include \
                    $(OIC_SRC_DIR)/resource/oc_logger/include \
                    $(OIC_SRC_DIR)/extlibs/boost/boost_1_58_0 \
                    $(OIC_SRC_DIR)/extlibs/timer \
                    $(OIC_SRC_DIR)/service/things-manager/sdk/inc \
                    $(LOCAL_PATH)/tm/inc

LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/jniutil/src/*.cpp))
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/tm/src/*.cpp))

LOCAL_CPPFLAGS := -std=c++0x -frtti -fexceptions

LOCAL_LDLIBS := -llog
LOCAL_SHARED_LIBRARIES := android-ocstack-jni
LOCAL_SHARED_LIBRARIES += android-thingsmanager
LOCAL_SHARED_LIBRARIES += android-oc

include $(BUILD_SHARED_LIBRARY)
