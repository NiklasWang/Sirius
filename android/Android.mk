LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES   := ion/ion.cpp
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_C_INCLUDES  := $(LOCAL_PATH)/kernel-headers
LOCAL_CFLAGS      := -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE      := libsirius_ion

include $(BUILD_STATIC_LIBRARY)
