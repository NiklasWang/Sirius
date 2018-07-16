LOCAL_PATH  := $(call my-dir)
SIRIUS_PATH := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_SRC_FILES   := Main.cpp ServerTester.cpp
LOCAL_C_INCLUDES  := $(SIRIUS_PATH)
LOCAL_SHARED_LIBRARIES := libsirius
LOCAL_CFLAGS      := -Werror
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)
LOCAL_MODULE      := libsirius_server

include $(BUILD_EXECUTABLE)