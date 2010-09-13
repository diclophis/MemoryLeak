LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sanangeles

TARGET_ARCH=arm
TARGET_ARCH_ABI=arm
LOCAL_ARM_MODE=arm

LOCAL_CFLAGS := -I/Users/jon/iPhone/MemoryLeak/jni/libev -DANDROID_NDK -DEV_STANDALONE=1 -DEV_USE_SELECT=1 -DEV_SELECT_USE_FD_SET

#-DANDROID \
#-D_REENTRANT \

#-DBOOST_HAS_PTHREADS \
#-DBOOST_THREAD_LINUX \
#-DBOOST_THREAD_LINUX \
#-D_GLIBCXX__PTHREADS \
#-D__SGI_STL_INTERNAL_PAIR_H \
#-D__NEW__ \

#LOCAL_CFLAGS := -DANDROID_NDK -DDISABLE_IMPORTGL
#-D__arm__ \
#-fno-short-enums \

LOCAL_SRC_FILES := \
    importgl.c \
    android.cpp \
    Engine.cpp \
    RaptorIsland.cpp \
    MD2_File.cpp \
    MD2_Manager.cpp \
    MD2_Model.cpp \

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
