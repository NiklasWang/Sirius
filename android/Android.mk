LOCAL_PATH  := $(call my-dir)
SIRIUS_ROOT := $(LOCAL_PATH)../

include $(CLEAR_VARS)

LOCAL_SRC_FILES   := ion/ion.cpp
LOCAL_C_INCLUDES  :=     \
    $(SIRIUS_ROOT)       \
    $(SIRIUS_ROOT)/utils \
    $(SIRIUS_ROOT)/log
LOCAL_SHARED_LIBRARIES := liblog
LOCAL_CFLAGS      := -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)
LOCAL_MODULE      := libsirius_ion

include $(BUILD_STATIC_LIBRARY)
