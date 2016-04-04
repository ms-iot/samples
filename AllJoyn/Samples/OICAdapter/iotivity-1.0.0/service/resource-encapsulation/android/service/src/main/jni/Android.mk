LOCAL_PATH := $(call my-dir)

ROOT_PATH := ../../../../../../..
IOTIVITY_LIB_PATH := $(ROOT_PATH)/out/android/$(TARGET_ARCH_ABI)/$(APP_OPTIM)

include $(CLEAR_VARS)
LOCAL_MODULE := rcs_common
LOCAL_SRC_FILES := $(IOTIVITY_LIB_PATH)/librcs_common.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rcs_client
LOCAL_SRC_FILES := $(IOTIVITY_LIB_PATH)/librcs_client.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := rcs_server
LOCAL_SRC_FILES := $(IOTIVITY_LIB_PATH)/librcs_server.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
OIC_SRC_DIR := ../../../../../..
LOCAL_MODULE := rcs_jni

LOCAL_C_INCLUDES := $(LOCAL_PATH)/util
LOCAL_C_INCLUDES += $(OIC_SRC_DIR)/resource/c_common
LOCAL_C_INCLUDES += $(OIC_SRC_DIR)/resource/csdk/stack/include
LOCAL_C_INCLUDES += $(OIC_SRC_DIR)/extlibs/boost/boost_1_58_0
LOCAL_C_INCLUDES += $(OIC_SRC_DIR)/service/resource-encapsulation/include
LOCAL_C_INCLUDES += $(OIC_SRC_DIR)/service/resource-encapsulation/src/serverBuilder/include

LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/util/*.cpp))
LOCAL_SRC_FILES += $(patsubst $(LOCAL_PATH)/%, %, $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_CPPFLAGS := -std=c++0x -frtti -fexceptions

LOCAL_LDLIBS := -llog

LOCAL_SHARED_LIBRARIES += rcs_common
LOCAL_SHARED_LIBRARIES += rcs_client
LOCAL_SHARED_LIBRARIES += rcs_server

include $(BUILD_SHARED_LIBRARY)
