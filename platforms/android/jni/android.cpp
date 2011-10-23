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
static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;
static std::vector<foo*> sounds;

static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static jobject activity;
jmethodID android_push_webview;
jmethodID android_pop_webview;

jstring messagePush = NULL;

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
  int sounds_count, jobjectArray fd_sys3, jintArray off3, jintArray len3
);


void Java_com_example_SanAngeles_DemoActivity_setMinBuffer(JNIEnv * env, jclass envClass, int size);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeStartGame(JNIEnv * env, jclass envClass, int g);
void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, int count, jintArray arr);
void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause(JNIEnv*  env);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeResume(JNIEnv*  env);
void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState);


void *pump_audio(void *) {
  playing_audio = true;

  if (g_Env == NULL) {
    g_Vm->AttachCurrentThread(&g_Env, NULL);
  }

  if (ab == NULL) {
    jobject tmp;
    ab = g_Env->NewShortArray(min_buffer);
    tmp = ab;
    ab = (jshortArray)g_Env->NewGlobalRef(ab);
    LOGV("AAA\n");
    g_Env->DeleteLocalRef(tmp);
  }

  if (android_dumpAudio == NULL) {
    android_dumpAudio = g_Env->GetStaticMethodID(player, "writeAudio", "([SII)V");
  }

  short *b;
  b = new short[min_buffer];
  memset(b, 0, min_buffer * sizeof(short));

  while (playing_audio) {
    Engine::CurrentGameDoAudio(b, min_buffer / sizeof(short));
    g_Env->SetShortArrayRegion(ab, 0, min_buffer / sizeof(short), b);
    g_Env->CallStaticVoidMethod(player, android_dumpAudio, ab, 0, min_buffer / sizeof(short));
  }

  g_Vm->DetachCurrentThread();
}


void SimulationThreadCleanup() {
  g_Env3 = NULL;
  android_pop_webview = NULL;
  g_Env = NULL;
  android_dumpAudio = NULL;
  /*
  g_Env2 = NULL;
  g_Env3 = NULL;
  android_push_webview = NULL;
  android_pop_webview = NULL;
  ab = NULL;
  */
  g_Vm->DetachCurrentThread();
}


bool pushMessageToWebView(const char *messageToPush) {
  if (g_Env2 == NULL) {
    g_Vm->AttachCurrentThread(&g_Env2, NULL);
  }

  if (g_Env2 == NULL || g_Env2 == 0) {
    return false;
  }

  if (android_push_webview == NULL) {
    android_push_webview = g_Env2->GetMethodID(player, "pushMessageToWebView", "(Ljava/lang/String;)Z");
    if (android_push_webview == 0) {
      LOGV("failed to find method\n");
      return false;
    }
  }

  jstring js = g_Env2->NewStringUTF(messageToPush);

  jboolean jbool = g_Env2->CallBooleanMethod(activity, android_push_webview, js);

  g_Env2->DeleteLocalRef(js);

  return jbool;
}


const char *popMessageFromWebView() {
  if (activity == NULL) {
    return "broke";
  }

  if (g_Vm == NULL || g_Vm == 0) {
    return "broke";
  }

  if (g_Env3 == NULL) {
    g_Vm->GetEnv((void **)&g_Env3, JNI_VERSION_1_6);
    g_Vm->AttachCurrentThread(&g_Env3, NULL);
  }

  if (g_Env3 == NULL || g_Env3 == 0) {
    return "broke";
  }

  if (android_pop_webview == NULL) {
    android_pop_webview = g_Env3->GetMethodID(player, "popMessageFromWebView", "()Ljava/lang/String;");
    if (android_pop_webview == 0) {
      LOGV("failed to find method\n");
      return "broke";
    }
  } else {
    jstring rv = (jstring) g_Env3->CallObjectMethod(activity, android_pop_webview, 0); 
    jboolean isCopy = false;
    const char *r = g_Env3->GetStringUTFChars(rv, &isCopy);
    g_Env3->DeleteLocalRef(rv);
    return r;
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

/*
void *start_game( void *ptr ) {
  int g = (int)ptr;  
  playing_audio = false;
  pthread_join(audio_thread, NULL);
  Engine::Start(g, sWindowWidth, sWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);
  create_audio_thread();
  g_Vm->DetachCurrentThread();
}
*/


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeStartGame(JNIEnv * env, jclass envClass, int g) {
  playing_audio = false;
  pthread_join(audio_thread, NULL);
  Engine::Start(g, sWindowWidth, sWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);
  create_audio_thread();
}


int Java_com_example_SanAngeles_DemoActivity_initNative(
  JNIEnv * env, jobject thiz,
  int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1,
  int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2,
  int sound_count, jobjectArray fd_sys3, jintArray off3, jintArray len3
) {
  activity = (jobject)env->NewGlobalRef(thiz);
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

  if (Engine::CurrentGame()) {
    Engine::CurrentGameStart();
  } else {
    Engine::Start(2, sWindowWidth, sWindowHeight, textures, models, levels, sounds, pushMessageToWebView, popMessageFromWebView, SimulationThreadCleanup);
    pthread_create(&audio_thread, 0, pump_audio, NULL);
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
