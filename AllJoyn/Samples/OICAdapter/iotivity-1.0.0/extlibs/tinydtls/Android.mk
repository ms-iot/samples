APP_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_PATH = $(APP_PATH)
LOCAL_MODULE := TinyDtls
LOCAL_SRC_FILES := dtls.c crypto.c ccm.c hmac.c netq.c peer.c dtls_time.c session.c
#LOCAL_SRC_FILES += debug.c
LOCAL_SRC_FILES += aes/rijndael.c
LOCAL_SRC_FILES += ecc/ecc.c
LOCAL_SRC_FILES += sha2/sha2.c

LOCAL_C_INCLUDES := $(APP_PATH) $(APP_PATH)/aes $(APP_PATH)/ecc $(APP_PATH)/sha2

#LOCAL_CFLAGS := -DWITH_OICSTACK -fPIC
LOCAL_CFLAGS += -DDTLSv12 -DWITH_SHA256 -DDTLS_CHECK_CONTENTTYPE -DHAVE_SYS_TIME_H -DNDEBUG

include $(BUILD_SHARED_LIBRARY)
