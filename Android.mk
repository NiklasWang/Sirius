LOCAL_PATH  := $(call my-dir)
SIRIUS_PATH := $(LOCAL_PATH)

ENABLE_ION_MEM     := yes
ENABLE_ANDROID_JNI := no
ENABLE_LOGCAT_LOG  := yes

include $(CLEAR_VARS)

LOCAL_SRC_FILES :=                      \
    core/SiriusServer.cpp               \
    core/SiriusServerImpl.cpp           \
    core/SiriusClient.cpp               \
    core/SiriusClientImpl.cpp           \
    core/SiriusCore.cpp                 \
    core/SiriusClientCore.cpp           \
    core/RequestHandler.cpp             \
    core/RequestHandlerClient.cpp       \
    core/ServerCallbackThread.cpp       \
    core/ServerClientControl.cpp        \
    core/SiriusData.cpp                 \
    socket/socket_server.cpp            \
    socket/socket_client.cpp            \
    socket/server_client_common.cpp     \
    socket/SocketClientStateMachine.cpp \
    socket/SocketServerStateMachine.cpp \
    buffer/BufferMgr.cpp                \
    buffer/IonBufferMgr.cpp             \
    usecase/EventClient.cpp             \
    usecase/EventServer.cpp             \
    usecase/PreviewClient.cpp           \
    usecase/PreviewServer.cpp           \
    usecase/YuvPictureClient.cpp        \
    usecase/YuvPictureServer.cpp        \
    usecase/BayerPictureClient.cpp      \
    usecase/BayerPictureServer.cpp      \
    usecase/DataClient.cpp              \
    usecase/DataServer.cpp              \
    threads/Thread.cpp                  \
    threads/ThreadPool.cpp              \
    threads/ThreadPoolEx.cpp            \
    threads/ThreadRunner.cpp            \
    memory/MemMgmt.cpp                  \
    memory/PoolImpl.cpp                 \
    memory/MemoryHolder.cpp             \
    log/LogImpl.cpp                     \
    utils/modules.cpp                   \
    utils/Semaphore.cpp                 \
    utils/SemaphoreTimeout.cpp          \
    utils/CQueue.cpp                    \
    utils/SyncType.cpp                  \
    utils/sp/RefBase.cpp                \
    tester/ServerTester.cpp             \
    tester/ClientTester.cpp             \
    tester/ServerTesterImpl.cpp         \
    tester/ClientTesterImpl.cpp         \
    tester/YuvPictureOperator.cpp       \
    tester/TestCases.cpp                \
    tester/PreviewTestCases.cpp         \
    tester/NV21PictureTestCases.cpp     \
    tester/BayerPictureTestCases.cpp    \
    tester/EventTestCases.cpp           \
    tester/CustomDataTestCases.cpp      \
    tester/MixtureTestCases.cpp

LOCAL_C_INCLUDES :=         \
    $(SIRIUS_PATH)/utils    \
    $(SIRIUS_PATH)/utils/sp \
    $(SIRIUS_PATH)/log      \
    $(SIRIUS_PATH)/core     \
    $(SIRIUS_PATH)/threads  \
    $(SIRIUS_PATH)/memory   \
    $(SIRIUS_PATH)/socket   \
    $(SIRIUS_PATH)/buffer   \
    $(SIRIUS_PATH)/usecase

LOCAL_CFLAGS := -std=c++11 -Wall -Wextra -Werror
LOCAL_CFLAGS += -DENABLE_LOGGER -DMEMORY_POOL_MODE

LOCAL_SHARED_LIBRARIES += liblog

LOCAL_STATIC_LIBRARIES +=

ifeq ($(ENABLE_ION_MEM),yes)
  LOCAL_CFLAGS     += -DENABLE_ION_BUFFER
  LOCAL_C_INCLUDES +=            \
      $(SIRIUS_PATH)/android     \
      $(SIRIUS_PATH)/android/ion \
      $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
  LOCAL_STATIC_LIBRARIES        += libsirius_ion
  LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
endif

ifeq ($(ENABLE_ANDROID_JNI),yes)
    LOCAL_SRC_FILES        += com_example_sirius_MainActivity.cpp
    LOCAL_SHARED_LIBRARIES += libandroid_runtime
endif

ifeq ($(ENABLE_LOGCAT_LOG),yes)
    LOCAL_CFLAGS += -DPRINT_LOGCAT_LOG
endif

LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)

LOCAL_MODULE:= libsirius

include $(BUILD_SHARED_LIBRARY)

include $(SIRIUS_PATH)/android/Android.mk
include $(SIRIUS_PATH)/tester/Android.mk
