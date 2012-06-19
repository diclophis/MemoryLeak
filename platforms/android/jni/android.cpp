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

#include "stdafx.h"

#include "MemoryLeak.h"

#ifdef __cplusplus
extern "C" {
#endif

static JavaVM *g_Vm;
static JNIEnv *g_Env;
static JNIEnv *g_Env2;
static JNIEnv *g_Env3;
static jshortArray ab = NULL;
static jclass player;
jmethodID android_dumpAudio;
static int min_buffer;
static pthread_t audio_thread;

/*
static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;
static std::vector<foo*> sounds;
*/

static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static jobject activity;

bool playing_audio = false;

void *pump_audio(void *);
void create_audio_thread();

void create_audio_thread() {
  playing_audio = false;
  pthread_attr_t attr; 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&audio_thread, 0, pump_audio, NULL);
}


int Java_com_example_SanAngeles_DemoActivity_initNative(
  JNIEnv * env, jobject thiz,
  int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1,
  int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2,
  int sounds_count, jobjectArray fd_sys3, jintArray off3, jintArray len3,
  int textures_count, jobjectArray fd_sys4, jintArray off4, jintArray len4
);


void Java_com_example_SanAngeles_DemoActivity_setMinBuffer(JNIEnv * env, jclass envClass, int size);
//void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeStartGame(JNIEnv * env, jclass envClass, int g);
void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz); //, int count, jintArray arr);
void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause(JNIEnv*  env);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeResume(JNIEnv*  env);
void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState);


void *pump_audio(void *) {
  playing_audio = true;
  short *b = new short[min_buffer];

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

  while (playing_audio) {
    Engine::CurrentGameDoAudio(b, min_buffer * sizeof(short));
    g_Env->SetShortArrayRegion(ab, 0, min_buffer, b);
    g_Env->CallStaticVoidMethod(player, android_dumpAudio, ab, 0, min_buffer);
  }

  delete b;

  g_Vm->DetachCurrentThread();
}


void SimulationThreadCleanup() {
  g_Env3 = NULL;
  g_Env = NULL;
  android_dumpAudio = NULL;
  g_Vm->DetachCurrentThread();
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
  min_buffer = size / 4;
}


//void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeStartGame(JNIEnv * env, jclass envClass, int g) {
//  Engine::Start(g, sWindowWidth, sWindowHeight, textures, models, levels, sounds, SimulationThreadCleanup);
//}


int Java_com_example_SanAngeles_DemoActivity_initNative(
  JNIEnv * env, jobject thiz,
  int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1,
  int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2,
  int sound_count, jobjectArray fd_sys3, jintArray off3, jintArray len3,
  int textures_count, jobjectArray fd_sys4, jintArray off4, jintArray len4
) {

  /*
  if (Engine::CurrentGame()) {
    for (std::vector<foo *>::iterator i = models.begin(); i != models.end(); ++i) {
      delete *i;
    }
    models.clear();

    for (std::vector<foo *>::iterator i = levels.begin(); i != levels.end(); ++i) {
      delete *i;
    }
    levels.clear();

    for (std::vector<foo *>::iterator i = sounds.begin(); i != sounds.end(); ++i) {
      delete *i;
    }
    sounds.clear();
  }
  */

  LOGV("\n\n models:%d levels:%d sounds:%d textures:%d\n\n", model_count, level_count, sound_count, textures_count);

  activity = (jobject)env->NewGlobalRef(thiz);
	jclass fdClass = env->FindClass("java/io/FileDescriptor");
	if (fdClass != NULL) {
		jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
		jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
		if (fdClassDescriptorFieldID != NULL) {
      for (int i=0; i<model_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys1, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        //foo *f = new foo;
        //f->fp = fdopen(myfdx, "rb");
        //f->off = env->GetIntArrayElements(off1, 0)[i];
        //f->len = env->GetIntArrayElements(len1, 0)[i];
        //models.push_back(f);
        Engine::PushBackFileHandle(MODELS, fdopen(myfdx, "rb"), env->GetIntArrayElements(off1, 0)[i], env->GetIntArrayElements(len1, 0)[i]);
      }
      for (int i=0; i<level_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys2, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        //foo *f = new foo;
        //f->fp = fdopen(myfdx, "rb");
        //f->off = env->GetIntArrayElements(off2, 0)[i];
        //f->len = env->GetIntArrayElements(len2, 0)[i];
        //levels.push_back(f);
        Engine::PushBackFileHandle(LEVELS, fdopen(myfdx, "rb"), env->GetIntArrayElements(off2, 0)[i], env->GetIntArrayElements(len2, 0)[i]);
      }
      for (int i=0; i<sound_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys3, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        //foo *f = new foo;
        //f->fp = fdopen(myfdx, "rb");
        //f->off = env->GetIntArrayElements(off3, 0)[i];
        //f->len = env->GetIntArrayElements(len3, 0)[i];
        //sounds.push_back(f);
        Engine::PushBackFileHandle(SOUNDS, fdopen(myfdx, "rb"), env->GetIntArrayElements(off3, 0)[i], env->GetIntArrayElements(len3, 0)[i]);
      }
      for (int i=0; i<textures_count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys4, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        //foo *f = new foo;
        //f->fp = fdopen(myfdx, "rb");
        //f->off = env->GetIntArrayElements(off3, 0)[i];
        //f->len = env->GetIntArrayElements(len3, 0)[i];
        //sounds.push_back(f);
        Engine::PushBackFileHandle(TEXTURES, fdopen(myfdx, "rb"), env->GetIntArrayElements(off4, 0)[i], env->GetIntArrayElements(len4, 0)[i]);
      }
		}
	}
}


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz) { //, int count, jintArray arr) {
  /*
  if (Engine::CurrentGame()) {
    textures.clear();
  }

	for (int i=0; i<count; i++) {
  LOGV("texture :%d\n", env->GetIntArrayElements(arr, 0)[i]);
		textures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}
  */

  if (Engine::CurrentGame()) {
    //Engine::CurrentGameDestroyFoos();
    //Engine::CurrentGameSetAssets(textures, models, levels, sounds);
    //Engine::CurrentGameCreateFoos();
    //Engine::CurrentGameStart();
  } else {
    Engine::Start(4, sWindowWidth, sWindowHeight);
    create_audio_thread();
  }
}


void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height) {
  sWindowWidth = width;
  sWindowHeight = height;
  Engine::CurrentGameResizeScreen(width, height);
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env ) {
  Engine::CurrentGamePause();
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeResume( JNIEnv*  env ) {
  LOGV("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n");
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState) {
  Engine::CurrentGameHit(x, y, (int)hitState);
}


void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {
  Engine::CurrentGameDrawScreen(0);
}

#ifdef __cplusplus
}
#endif
