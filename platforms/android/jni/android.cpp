#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <assert.h>

#include "modplug.h"
#include "stdafx.h"
#include "sndfile.h"

#include "MemoryLeak.h"

#include "Model.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Engine.h"
#include "octree.h"
#include "micropather.h"
#include "ModelOctree.h"
#include "SuperBarrelBlast.h"

#ifdef __cplusplus
extern "C" {
#endif

static JavaVM *g_Vm;
static JNIEnv *g_Env;
static jshortArray ab = NULL;
static jclass player;
jmethodID android_dumpAudio;
static int min_buffer;
static pthread_t audio_thread;
static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;
static std::vector<foo*> sounds;
static Engine *game;
static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static int gameState;


int Java_com_example_SanAngeles_DemoActivity_initNative(
  JNIEnv * env, jclass envClass,
  int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1,
  int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2,
  int sounds_count, jobjectArray fd_sys3, jintArray off3, jintArray len3
);


void Java_com_example_SanAngeles_DemoActivity_setMinBuffer(
  JNIEnv * env, jclass envClass,
  int size
);


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, int count, jintArray arr);
void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause(JNIEnv*  env);
void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState);


void *pump_audio(void *) {
  if (g_Env == NULL) {
    g_Vm->AttachCurrentThread(&g_Env, NULL);
  }

  if (ab == NULL) {
    jobject tmp;
    ab = g_Env->NewShortArray(min_buffer);
    tmp = ab;
    ab = (jshortArray)g_Env->NewGlobalRef(ab);
    g_Env->DeleteLocalRef(tmp);
  }

  if (android_dumpAudio == NULL) {
    android_dumpAudio = g_Env->GetStaticMethodID(player, "writeAudio", "([SII)V");
  }

  short *b;
  b = new short[min_buffer];
  memset(b, 0, min_buffer * sizeof(short));

  while (gameState) {
    if (game) {
      game->DoAudio(b, min_buffer / sizeof(short));
      g_Env->SetShortArrayRegion(ab, 0, min_buffer / sizeof(short), b);
      g_Env->CallStaticVoidMethod(player, android_dumpAudio, ab, 0, min_buffer / sizeof(short));
    }
  }
}


JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM * vm, void * reserved) {
  g_Vm = vm;
  JNIEnv *env;
  jobject tmp;
  g_Vm->GetEnv((void **)&env, JNI_VERSION_1_6);
  player = env->FindClass("com/example/SanAngeles/DemoActivity");
  tmp = player;
  player = (jclass)env->NewGlobalRef(player);
  env->DeleteLocalRef(tmp);
  return JNI_VERSION_1_6;
}


void Java_com_example_SanAngeles_DemoActivity_setMinBuffer(
  JNIEnv * env, jclass envClass,
  int size
) {
  min_buffer = size;
}


int Java_com_example_SanAngeles_DemoActivity_initNative(
  JNIEnv * env, jclass envClass,
  int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1,
  int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2,
  int sound_count, jobjectArray fd_sys3, jintArray off3, jintArray len3
) {
	jclass fdClass = env->FindClass("java/io/FileDescriptor");
	if (fdClass != NULL) {
		jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
		jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
		if (fdClassDescriptorFieldID != NULL) {
      for (int i=0; i<model_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys1, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        foo *f = new foo;
        f->fp = fdopen(myfdx, "rb");
        f->off = env->GetIntArrayElements(off1, 0)[i];
        f->len = env->GetIntArrayElements(len1, 0)[i];
        models.push_back(f);
      }
      for (int i=0; i<level_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys2, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        foo *f = new foo;
        f->fp = fdopen(myfdx, "rb");
        f->off = env->GetIntArrayElements(off2, 0)[i];
        f->len = env->GetIntArrayElements(len2, 0)[i];
        levels.push_back(f);
      }
      for (int i=0; i<sound_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys3, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        foo *f = new foo;
        f->fp = fdopen(myfdx, "rb");
        f->off = env->GetIntArrayElements(off3, 0)[i];
        f->len = env->GetIntArrayElements(len3, 0)[i];
        sounds.push_back(f);
      }
		}
	}
}


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, int count, jintArray arr) {
	for (int i=0; i<count; i++) {
		textures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}

  game = new SuperBarrelBlast(sWindowWidth, sWindowHeight, textures, models, levels, sounds);
  game->CreateThread();
	pthread_create(&audio_thread, 0, pump_audio, NULL);
  
  gameState = 1;
}


void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height) {
  game->ResizeScreen(width, height);
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env ) {
  game->PauseThread();
  gameState = 0;
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState) {
	game->Hit(x, y, (int)hitState);
}


void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {
  if (gameState) {
    game->DrawScreen(0);
  } 
}

#ifdef __cplusplus
}
#endif
