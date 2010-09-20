LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sanangeles

TARGET_ARCH=arm
TARGET_ARCH_ABI=arm
LOCAL_ARM_MODE=arm

LOCAL_CFLAGS := -I/Users/jon/iPhone/MemoryLeak/jni -DANDROID_NDK -DEV_STANDALONE=1 -DEV_USE_SELECT=1 -DEV_SELECT_USE_FD_SET -D_iPhoneVersion=1

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
    CaptureTheFlag.cpp \
    MD2_File.cpp \
    MD2_Manager.cpp \
    MD2_Model.cpp \
    OpenSteer/Clock.cpp \
    OpenSteer/Color.cpp \
    OpenSteer/Draw.cpp \
    OpenSteer/Obstacle.cpp \
    OpenSteer/OldPathway.cpp \
    OpenSteer/Path.cpp \
    OpenSteer/Pathway.cpp \
    OpenSteer/PlugIn.cpp \
    OpenSteer/SegmentedPath.cpp \
    OpenSteer/SegmentedPathway.cpp \
    OpenSteer/SimpleVehicle.cpp \
    OpenSteer/TerrainRayTest.cpp \
    OpenSteer/Vec3.cpp \
    OpenSteer/Vec3Utilities.cpp

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
