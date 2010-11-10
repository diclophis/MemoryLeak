LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sanangeles

TARGET_ARCH=arm
TARGET_ARCH_ABI=arm
LOCAL_ARM_MODE=arm

LOCAL_CFLAGS := -I../../src -I../../src/include -I../../src/assimp/BoostWorkaround -DANDROID_NDK -DEV_STANDALONE=1 -DEV_USE_SELECT=1 -DEV_SELECT_USE_FD_SET -D_iPhoneVersion=1

#LOCAL_CFLAGS += \
#	-fPIC -DPIC -D_POSIX_SOURCE \
#	-DALSA_CONFIG_DIR=\"/system/usr/share/alsa\" \
#	-DALSA_PLUGIN_DIR=\"/system/usr/lib/alsa-lib\" \
#	-DALSA_DEVICE_DIRECTORY=\"/dev/snd/\"

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

#LOCAL_SRC_FILES := android.cpp

#LOCAL_SRC_FILES += $(wildcard jni/assimp/*.cpp)

#MY_PREFIX := ../../../
#MY_SOURCES := $(wildcard $(MY_PREFIX)/*.cpp)
#MY_SOURCES := $(wildcard $(MY_PREFIX)/*.c)
#MY_SOURCES := $(wildcard $(MY_PREFIX)/assimp/*.cpp)
#MY_SOURCES += $(wildcard $(MY_PREFIX)/contrib/irrXML/*.cpp)
#MY_SOURCES += $(wildcard $(MY_PREFIX)/contrib/ConvertUTF/*.c)
#MY_SOURCES += $(wildcard $(MY_PREFIX)/contrib/unzip/*.c)
#MY_SOURCES += $(wildcard $(MY_PREFIX)/contrib/zlib/*.c)

CG_SUBDIRS := \
. \
../../../src \
../../../src/assimp \
../../../src/contrib/irrXML \
../../../src/contrib/ConvertUTF \
../../../src/contrib/unzip \
../../../src/contrib/zlib


LOCAL_SRC_FILES := $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(CG_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/alisp/alisp_snd.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/compat/hsearch_r.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/control/control_shm.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/pcm/pcm_d%.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/pcm/pcm_ladspa.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/pcm/pcm_shm.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/pcm/scopes/level.c, $(LOCAL_SRC_FILES))
#LOCAL_SRC_FILES := $(filter-out ../../../src/alsa-lib/src/shmarea.c, $(LOCAL_SRC_FILES))


LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog

include $(BUILD_SHARED_LIBRARY)
#include $(BUILD_STATIC_LIBRARY)
