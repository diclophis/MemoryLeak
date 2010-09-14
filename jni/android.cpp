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
#include "RaptorIsland.h"


extern "C" {
  void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, jobject fd_sys1, unsigned int off1, unsigned int len1, jobject fd_sys2, unsigned int off2, unsigned int len2);
  void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr);
  void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env );
  void Java_com_example_SanAngeles_DemoRenderer_nativeDone(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env);
}


static FILE *myFile1;
static unsigned int fileOffset1;
static unsigned int fileLength1;

static FILE *myFile2;
static unsigned int fileOffset2;
static unsigned int fileLength2;

static std::vector<GLuint> sPlayerTextures;
static long sStartTick = 0;
static long sTick = 0;
static RaptorIsland *gameController;
int gAppAlive   = 1;
static int  sDemoStopped  = 0;
static long sTimeOffset   = 0;
static int  sTimeOffsetInit = 0;
static long sTimeStopped  = 0;
static int  sWindowWidth  = 320;
static int  sWindowHeight = 480;
static int gameState;


JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM * vm, void * reserved) {
  return JNI_VERSION_1_6;
}


void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, jobject fd_sys1, unsigned int off1, unsigned int len1, jobject fd_sys2, unsigned int off2, unsigned int len2) {
	importGLInit();
	jclass fdClass = env->FindClass("java/io/FileDescriptor");
	if (fdClass != NULL) {
		jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
		jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
		if (fdClassDescriptorFieldID != NULL && fd_sys1 != NULL && fd_sys2 != NULL) {
			jint fd1 = env->GetIntField(fd_sys1, fdClassDescriptorFieldID);
			int myfd1 = dup(fd1);
			myFile1 = fdopen(myfd1, "rb");
			fileOffset1 = off1;
			fileLength1 = len1;

			jint fd2 = env->GetIntField(fd_sys2, fdClassDescriptorFieldID);
			int myfd2 = dup(fd2);
			myFile2 = fdopen(myfd2, "rb");
			fileOffset2 = off2;
			fileLength2 = len2;
		}
	} 
} 


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr) {

  LOGV("nativeInit");

	for (int i=0; i<13; i++) {
		sPlayerTextures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}

	std::vector<foo*> models;
		
	foo firstModel; // = new foo;
	firstModel.fp = myFile1;
	firstModel.off = fileOffset1;
	firstModel.len = fileLength1;
	
	models.push_back(&firstModel);

  LOGV("nativeInit b");

	foo secondModel; // = new foo;
	secondModel.fp = myFile2;
	secondModel.off = fileOffset2;
	secondModel.len = fileLength2;
	
	models.push_back(&secondModel);

  LOGV("nativeInit c");

  gameController = new RaptorIsland();
  gameController->build(sWindowWidth, sWindowHeight, sPlayerTextures, models);
  gameState = gameController->tick();
	models.clear();
	sPlayerTextures.clear();
  gAppAlive    = 1;
  sDemoStopped = 0;
  sTimeOffsetInit = 0;

  LOGV("nativeInit d");
}


void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height) {
  LOGV("nativeResize %d %d", width, height);
  sWindowWidth  = width;
  sWindowHeight = height;
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
  //gameController->playerStartedJumping();
}


/* Call to render the next GL frame */
void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {
  /* NOTE: if sDemoStopped is TRUE, then we re-render the same frame
   *       on each iteration.
   */
  if (sDemoStopped) {
  } else {
    if (gameState) {
      //for (int i=0; i<=gameState; i++) {
      //  gameState = gameController->tick(1.0 / 500.0);
      //  gameState = gameController->tick(1.0 / 500.0);
      //	gameState = gameController->tick(1.0 / 500.0);
      //  gameState = gameController->tick(1.0 / 500.0);
      //}
      gameState = gameController->tick();
			gameController->draw(0);
    } else {
      //if (gameController) {
      //  delete gameController;
      //}
      //gameController = new GLViewController();
      //gameController->build(sWindowWidth, sWindowHeight, sPlayerTextures, myFile, fileOffset, fileLength);
      //gameState = gameController->tick(1.0 / 200.0);
    }
  }
}
