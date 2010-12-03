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
#include "Engine.h"
#include "MachineGun.h"
#include "octree.h"
#include "micropather.h"
#include "ModelOctree.h"
#include "PixelPusher.h"

extern "C" {
  void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1, int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2);
  void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, int count, jintArray arr);
  void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env );
  void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env, jobject thiz, jfloat x, jfloat y, jint hitState);
}

static std::vector<GLuint> textures;
static std::vector<foo*> models;
static std::vector<foo*> levels;

static Engine *gameController;
static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static int gameState;


JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM * vm, void * reserved) {
  return JNI_VERSION_1_6;
}


void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int model_count, jobjectArray fd_sys1, jintArray off1, jintArray len1, int level_count, jobjectArray fd_sys2, jintArray off2, jintArray len2) {
	importGLInit();
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
		}
	} 
} 


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, int count, jintArray arr) {
	for (int i=0; i<count; i++) {
		textures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}

  Audio *foo = new Audio();
  gameController = new PixelPusher(sWindowWidth, sWindowHeight, textures, models, levels);
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
	gameController->Hit(x, y, (int)hitState);
}


void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {
  if (gameState) {
    gameController->DrawScreen(0);
  } 
}
