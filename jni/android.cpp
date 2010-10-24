/* San Angeles Observation OpenGL ES version example
 * Copyright 2009 The Android Open Source Project
 * All rights reserved.
 *
 * This source is free software; you can redistribute it and/or
 * modify it under the terms of EITHER:
 *   (1) The GNU Lesser General Public License as published by the Free
 *       Software Foundation; either version 2.1 of the License, or (at
 *       your option) any later version. The text of the GNU Lesser
 *       General Public License is included with this source in the
 *       file LICENSE-LGPL.txt.
 *   (2) The BSD-style license that is included with this source in
 *       the file LICENSE-BSD.txt.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 * LICENSE-LGPL.txt and LICENSE-BSD.txt for more details.
 */
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

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__) 

#include "importgl.h"

#include "MemoryLeak.h"


extern "C" {
  void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int count, jobjectArray fd_sys1, jintArray off1, jintArray len1); //, jobject fd_sys2, unsigned int off2, unsigned int len2, jobject fd_sys3, unsigned int off3, unsigned int len3, jobject fd_sys4, unsigned int off4, unsigned int len4);
  void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr);
  void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env );
  void Java_com_example_SanAngeles_DemoRenderer_nativeDone(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env);
}

static std::vector<foo*> models;
static std::vector<GLuint> sPlayerTextures;

static long sStartTick = 0;
static long sTick = 0;
static Engine *gameController;
int gAppAlive   = 1;
static int  sDemoStopped  = 0;
static long sTimeOffset   = 0;
static int  sTimeOffsetInit = 0;
static long sTimeStopped  = 0;
static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static int gameState;


JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM * vm, void * reserved) {
  return JNI_VERSION_1_6;
}


void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int count, jobjectArray fd_sys1, jintArray off1, jintArray len1) {
  LOGV("initNative 1");

	importGLInit();
	jclass fdClass = env->FindClass("java/io/FileDescriptor");
	if (fdClass != NULL) {
		jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
		jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
		if (fdClassDescriptorFieldID != NULL) {
      for (int i=0; i<count; i++) {
        LOGV("load model 1");
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

  LOGV("initNative 2");
} 


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr) {

  LOGV("nativeOnSurfaceCreated 1");

	for (int i=0; i<5; i++) {
  LOGV("texture %d", env->GetIntArrayElements(arr, 0)[i]);
		sPlayerTextures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}
		

  LOGV("nativeOnSurfaceCreated AAA");

  gameController = new RunAndJump(sWindowWidth, sWindowHeight, sPlayerTextures, models);
  gameController->go();

  LOGV("nativeOnSurfaceCreated BBB");

  gameState = 1;
  gAppAlive    = 1;
  sDemoStopped = 0;
  sTimeOffsetInit = 0;

  LOGV("nativeOnSurfaceCreated 2");
}


void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height) {

  LOGV("nativeResize %d %d", width, height);
  gameController->resizeScreen(width, height);

}


void Java_com_example_SanAngeles_DemoRenderer_nativeDone( JNIEnv*  env ) {
    LOGV("appDeinit");
    delete gameController;
    importGLDeinit();
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env ) {
		LOGV("nativePause");
    sDemoStopped = !sDemoStopped;
    if (sDemoStopped) {
    } else {
    }
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env) {
  LOGV("nativeTouch");
	gameController->hitTest(10, 10);
}


void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {
  if (sDemoStopped) {
  } else {
    if (gameState) {
			gameController->draw(0);
    } else {
    }
  }
}
