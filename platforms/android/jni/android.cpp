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


#include "importgl.h"
#include "MemoryLeak.h"
#include "Audio.h"
#include "Model.h"
#include "MachineGun.h"
#include "Engine.h"
#include "PixelPusher.h"

extern "C" {
  void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int count, jobjectArray fd_sys1, jintArray off1, jintArray len1);
  void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr);
  void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env );
  void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState);
}

static std::vector<GLuint> sPlayerTextures;
static std::vector<foo*> models;

static Engine *gameController;
static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static int gameState;


JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM * vm, void * reserved) {
  return JNI_VERSION_1_6;
}


void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int count, jobjectArray fd_sys1, jintArray off1, jintArray len1) {
	importGLInit();
	jclass fdClass = env->FindClass("java/io/FileDescriptor");
	if (fdClass != NULL) {
		jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
		jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
		if (fdClassDescriptorFieldID != NULL) {
      for (int i=0; i<count; i++) {
        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys1, i), fdClassDescriptorFieldID);
        int myfdx = dup(fdx);
        foo *firstModel = new foo;
        firstModel->fp = fdopen(myfdx, "rb");
        firstModel->off = env->GetIntArrayElements(off1, 0)[i];
        firstModel->len = env->GetIntArrayElements(len1, 0)[i];
        models.push_back(firstModel);
      }
		}
	} 
} 


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr) {
	for (int i=0; i<5; i++) {
		sPlayerTextures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}

  //LOGV("nativeSurfaceCreated %d %d", sPlayerTextures.size(), models.size());

  Audio *foo = new Audio();

  gameController = new PixelPusher(sWindowWidth, sWindowHeight, sPlayerTextures, models);
  gameController->CreateThread();

  gameState = 1;
}


void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height) {
  //LOGV("nativeResize");
  gameController->ResizeScreen(width, height);
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env ) {
  //LOGV("nativePause");
  gameController->PauseThread();
  gameState = 0;
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState) {
  //LOGV("nativeTouch");
	gameController->Hit(x, -y, (int)hitState);
}


void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {
  if (gameState) {
    gameController->DrawScreen(0);
  } 
}
