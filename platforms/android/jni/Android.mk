LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sanangeles

TARGET_ARCH=arm
TARGET_ARCH_ABI=arm
LOCAL_ARM_MODE=arm

LOCAL_CFLAGS := -DDEBUGBUILD -I../../src -I../../src/include -I../../src/OpenSteer -I../../src/Box2D -I../../src/assimp/BoostWorkaround -DMODPLUG_BASIC_SUPPORT -DMODPLUG_FASTSOUNDLIB -D_iPhoneVersion -DANDROID -DANDROID_NDK -DEV_STANDALONE=1 -DEV_USE_SELECT=1 -DEV_SELECT_USE_FD_SET -I../../src/octree -I../../src/include/libmodplug -DSMALLER_READS -DHAVE_SETENV -DMODPLUG_TRACKER -fexceptions

CG_SUBDIRS := \
. \
../../../src \
../../../src/libmodplug \
../../../src/octree \
../../../src/assimp \
../../../src/contrib/irrXML \
../../../src/contrib/ConvertUTF \
../../../src/contrib/unzip \
../../../src/contrib/zlib \
../../../src/Box2D \
../../../src/Box2D/Collision \
../../../src/Box2D/Collision/Shapes \
../../../src/Box2D/Common \
../../../src/Box2D/Dynamics \
../../../src/Box2D/Dynamics/Contacts \
../../../src/Box2D/Dynamics/Joints \
../../../src/Box2D/Rope \
../../../src/OpenSteer

LOCAL_SRC_FILES := $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lc -lgcc -lm -ldl -lstdc++

include $(BUILD_SHARED_LIBRARY)
