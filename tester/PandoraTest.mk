LOCAL_PATH := $(call my-dir)
SIRIUS_ROOT := $(LOCAL_PATH)/..

include $(CLEAR_VARS)

LOCAL_SRC_FILES   :=        \
    PandoraServerMain.cpp   \
    PandoraServerTester.cpp

LOCAL_C_INCLUDES  :=        \
    $(SIRIUS_ROOT)          \
    $(SIRIUS_ROOT)/utils    \
    $(SIRIUS_ROOT)/log

LOCAL_SHARED_LIBRARIES := libsirius
LOCAL_CFLAGS      := -Werror
LOCAL_CFLAGS      += -DENABLE_LOGGER
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)
LOCAL_MODULE      := sirius_pandora_tester

include $(BUILD_EXECUTABLE)
