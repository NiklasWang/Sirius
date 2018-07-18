LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES  := $(LOCAL_PATH)/../ \
                     $(LOCAL_PATH)/../android/ion \
                     $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_32_BIT_ONLY             := $(BOARD_QTI_CAMERA_32BIT_ONLY)

LOCAL_CFLAGS           := -std=c++11 -Wall -Wextra -Werror \
                          -DENABLE_LOGGER -DBUILD_ANDROID_AP -DMEMORY_POOL_MODE
LOCAL_SRC_FILES        := ion_cond_server.cpp
LOCAL_STATIC_LIBRARIES := libsirius_ion
LOCAL_SHARED_LIBRARIES := libsirius
LOCAL_MODULE           := ion_cond_server

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)

LOCAL_C_INCLUDES  := $(LOCAL_PATH)/../ \
                     $(LOCAL_PATH)/../android/ion \
                     $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_32_BIT_ONLY := $(BOARD_QTI_CAMERA_32BIT_ONLY)
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr

LOCAL_CFLAGS           := -std=c++11 -Wall -Wextra -Werror \
                          -DENABLE_LOGGER -DBUILD_ANDROID_AP -DMEMORY_POOL_MODE
LOCAL_SRC_FILES        := ion_cond_client.cpp
LOCAL_STATIC_LIBRARIES := libsirius_ion
LOCAL_SHARED_LIBRARIES := libsirius
LOCAL_MODULE           := ion_cond_client

include $(BUILD_EXECUTABLE)
