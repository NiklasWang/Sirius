LOCAL_PATH  := $(call my-dir)
SIRIUS_PATH := $(LOCAL_PATH)

ENABLE_ION_MEM     := yes
ENABLE_ANDROID_JNI := no

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    modules.cpp \
    Sirius.cpp \
    SiriusCore.cpp \
    SiriusImpl.cpp \
    socket_server.cpp \
    socket_client.cpp \
    server_client_common.cpp \
    SocketClientStateMachine.cpp \
    SocketServerStateMachine.cpp \
    SiriusClientCore.cpp \
    ServerClientControl.cpp \
    RequestHandler.cpp \
    UserBufferMgr.cpp \
    ServerCallbackThread.cpp \
    IonBufferMgr.cpp \
    RunOnceThread.cpp \
    log_impl.cpp \
    YuvPictureRequestClient.cpp \
    YuvPictureRequestServer.cpp \
    PreviewRequestServer.cpp \
    PreviewRequestClient.cpp \
    EventRequestServer.cpp \
    EventRequestClient.cpp \
    utils/Semaphore.cpp \
    utils/CQueue.cpp \
    utils/SyncType.cpp \
    threadpool/WorkerThread.cpp \
    threadpool/QueuedThread.cpp \
    threadpool/ThreadPool.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/utils \
    $(LOCAL_PATH)/threadpool

LOCAL_CFLAGS := -std=c++11 -Wall -Wextra -Werror
LOCAL_CFLAGS += -DENABLE_LOGGER -DBUILD_ANDROID_AP -DMEMORY_DEBUG_MODE

LOCAL_SHARED_LIBRARIES += liblog libutils libcutils

LOCAL_STATIC_LIBRARIES +=

ifeq ($(ENABLE_ION_MEM),yes)
  LOCAL_C_INCLUDES       += $(LOCAL_PATH)/android/ion
  LOCAL_STATIC_LIBRARIES += libsirius_ion
  LOCAL_C_INCLUDES       += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
  LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(ENABLE_ANDROID_JNI),yes)
    LOCAL_SRC_FILES        += com_example_sirius_MainActivity.cpp
    LOCAL_SHARED_LIBRARIES += libandroid_runtime
endif

LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)

LOCAL_MODULE:= libsirius

include $(BUILD_SHARED_LIBRARY)

include $(SIRIUS_PATH)/android/Android.mk
include $(SIRIUS_PATH)/tester/Android.mk
