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
#include "Engine.h"


extern "C" {
  void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, jobject fd_sys, unsigned int off, unsigned int len);
  void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr);
  void Java_com_example_SanAngeles_DemoRenderer_nativeResize(JNIEnv* env, jobject thiz, jint width, jint height);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativePause( JNIEnv*  env );
  void Java_com_example_SanAngeles_DemoRenderer_nativeDone(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoRenderer_nativeRender(JNIEnv* env);
  void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env);
}


static FILE *myFile;
static unsigned int fileOffset;
static unsigned int fileLength;
static GLuint *sPlayerTextures;
static long sStartTick = 0;
static long sTick = 0;
static GLViewController *gameController;
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


void Java_com_example_SanAngeles_DemoActivity_initNative(JNIEnv * env, jclass envClass, jobject fd_sys, unsigned int off, unsigned int len) {
  importGLInit();
  jclass fdClass = env->FindClass("java/io/FileDescriptor");
  if (fdClass != NULL) {
    jclass fdClassRef = (jclass) env->NewGlobalRef(fdClass); 
    jfieldID fdClassDescriptorFieldID = env->GetFieldID(fdClassRef, "descriptor", "I");
    if (fdClassDescriptorFieldID != NULL && fd_sys != NULL) {
      jint fd = env->GetIntField(fd_sys, fdClassDescriptorFieldID);
      int myfd = dup(fd);
      myFile = fdopen(myfd, "rb");
      fileOffset = off;
      fileLength = len;
    }
  } 
} 


void Java_com_example_SanAngeles_DemoRenderer_nativeOnSurfaceCreated(JNIEnv* env, jobject thiz, jintArray arr) {
  sPlayerTextures = (GLuint *) env->GetIntArrayElements(arr, 0);
  gameController = new GLViewController();
  gameController->build(sWindowWidth, sWindowHeight, sPlayerTextures, myFile, fileOffset, fileLength);
  gameState = gameController->tick(1.0 / 200.0);
  gAppAlive    = 1;
  sDemoStopped = 0;
  sTimeOffsetInit = 0;
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
    sDemoStopped = !sDemoStopped;
    if (sDemoStopped) {
    } else {
    }
}


void Java_com_example_SanAngeles_DemoGLSurfaceView_nativeTouch(JNIEnv* env) {
  LOGV("nativeTouch");
  gameController->playerStartedJumping();
}


/* Call to render the next GL frame */
void Java_com_example_SanAngeles_DemoRenderer_nativeRender( JNIEnv*  env ) {

  /* NOTE: if sDemoStopped is TRUE, then we re-render the same frame
   *       on each iteration.
   */
  if (sDemoStopped) {
  } else {
    if (gameState) {
      for (int i=0; i<=gameState; i++) {
        gameState = gameController->tick(1.0 / 500.0);
        gameState = gameController->tick(1.0 / 500.0);
        gameState = gameController->tick(1.0 / 500.0);
        gameState = gameController->tick(1.0 / 500.0);
      }
    } else {
      if (gameController) {
        delete gameController;
      }
      gameController = new GLViewController();
      gameController->build(sWindowWidth, sWindowHeight, sPlayerTextures, myFile, fileOffset, fileLength);
      gameState = gameController->tick(1.0 / 200.0);
    }
  }

  gameController->draw(0);
}
