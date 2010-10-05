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

#include "OpenSteer/Vec3.h"
#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "CaptureTheFlag.h"
#include "RaptorIsland.h"


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

/*
static FILE *myFile1;
static unsigned int fileOffset1;
static unsigned int fileLength1;

static FILE *myFile2;
static unsigned int fileOffset2;
static unsigned int fileLength2;

static FILE *myFile3;
static unsigned int fileOffset3;
static unsigned int fileLength3;

static FILE *myFile4;
static unsigned int fileOffset4;
static unsigned int fileLength4;
*/

static std::vector<GLuint> sPlayerTextures;
static long sStartTick = 0;
static long sTick = 0;
static RaptorIsland *gameController;
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


void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, int count, jobjectArray fd_sys1, jintArray off1, jintArray len1) { //jobject fd_sys1, unsigned int off1, unsigned int len1, jobject fd_sys2, unsigned int off2, unsigned int len2, jobject fd_sys3, unsigned int off3, unsigned int len3, jobject fd_sys4, unsigned int off4, unsigned int len4) {
  LOGV("initNative 1");

	importGLInit();
	jclass fdClass = env->FindClass("java/io/FileDescriptor");
	if (fdClass != NULL) {
		jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
		jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
		if (fdClassDescriptorFieldID != NULL) { // && fd_sys1 != NULL && fd_sys2 != NULL) {

      for (int i=0; i<count; i++) {
        LOGV("load model 1");

        jint fdx = env->GetIntField(env->GetObjectArrayElement(fd_sys1, i), fdClassDescriptorFieldID); //env->GetIntField(fd_sys4, fdClassDescriptorFieldID);
        int myfdx = dup(fdx);

        foo firstModel;
        firstModel.fp = fdopen(myfdx, "rb");
        //firstModel.fp = fdopen(env->GetIntField(env->GetObjectArrayElement(fd_sys1, i), fdClassDescriptorFieldID), "rb");
        firstModel.off = env->GetIntArrayElements(off1, 0)[i];
        firstModel.len = env->GetIntArrayElements(len1, 0)[i];

        //off1[i];
        //firstModel.len = len1[i];
        
        models.push_back(&firstModel);
      }

      /*
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

			jint fd3 = env->GetIntField(fd_sys3, fdClassDescriptorFieldID);
			int myfd3 = dup(fd3);
			myFile3 = fdopen(myfd3, "rb");
			fileOffset3 = off3;
			fileLength3 = len3;

			jint fd4 = env->GetIntField(fd_sys4, fdClassDescriptorFieldID);
			int myfd4 = dup(fd4);
			myFile4 = fdopen(myfd4, "rb");
			fileOffset4 = off4;
			fileLength4 = len4;
      */
		}
	} 

  LOGV("initNative 2");
} 


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr) {

  LOGV("nativeOnSurfaceCreated 1");

	for (int i=0; i<13; i++) {
		sPlayerTextures.push_back(env->GetIntArrayElements(arr, 0)[i]);
	}
		

/*
	foo secondModel; // = new foo;
	secondModel.fp = myFile2;
	secondModel.off = fileOffset2;
	secondModel.len = fileLength2;
	
	models.push_back(&secondModel);

	foo thirdModel; // = new foo;
	thirdModel.fp = myFile3;
	thirdModel.off = fileOffset3;
	thirdModel.len = fileLength3;
	
	models.push_back(&thirdModel);

	foo fourthModel;
	fourthModel.fp = myFile4;
	fourthModel.off = fileOffset4;
	fourthModel.len = fileLength4;
	
	models.push_back(&fourthModel);
*/

  LOGV("nativeOnSurfaceCreated AAA");

  gameController = new RaptorIsland();
  gameController->build(sWindowWidth, sWindowHeight, sPlayerTextures, models);

  LOGV("nativeOnSurfaceCreated BBB");

  gameState = 1; //gameController->tick();
	models.clear();
	sPlayerTextures.clear();
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
