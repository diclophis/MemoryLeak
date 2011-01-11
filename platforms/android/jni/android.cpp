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
#include "com_example_SanAngeles_PlayerThread.h"

#include "importgl.h"
#include "MemoryLeak.h"

#define BUFFER_SIZE 10000

static JavaVM *g_Vm;
static JNIEnv *g_Env;
static jshortArray ab = NULL;
ModPlugFile *currmodFile;
unsigned char samplebuffer[BUFFER_SIZE];
int currsample;
void *Cbuffer;
static jclass player;
jmethodID android_dumpAudio;

class Callbacks {
public:
  static void *PumpAudio(void *) {
    
    jint pumped = 0;

    if (g_Env == NULL) {
      g_Vm->AttachCurrentThread(&g_Env, NULL);
    }

    if (ab == NULL) {
      jobject tmp;
      ab = g_Env->NewShortArray(BUFFER_SIZE);
      tmp = ab;
      ab = (jshortArray)g_Env->NewGlobalRef(ab);
      g_Env->DeleteLocalRef(tmp);
    }

    if (android_dumpAudio == NULL) {
      android_dumpAudio = g_Env->GetStaticMethodID(player, "writeAudio", "([SI)V");
    }

    pumped = ModPlug_Read(currmodFile, samplebuffer, BUFFER_SIZE);

    if (samplebuffer && pumped) {
      //LOGV("DDD\n");
      g_Env->SetShortArrayRegion(ab, 0, pumped, (jshort *) (((char *)samplebuffer)+0));
      //LOGV("%p %p %d %p EEE\n", player, android_dumpAudio, ab, ab);
      g_Env->CallStaticVoidMethod(player, android_dumpAudio, ab, pumped);
      //LOGV("FFF\n");
    } else {
      LOGV("Error\n");
    }
    //LOGV("GGG\n");
    //g_Vm->DetachCurrentThread();
  };
};


#include "Audio.h"
#include "Model.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
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

static PixelPusher *gameController;
static int  sWindowWidth  = 0;
static int  sWindowHeight = 0;
static int gameState;

	const ModPlug_Settings gSettings8000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2,
		16,
		8000,
		MODPLUG_RESAMPLE_LINEAR,
		0,		0,		0,		0,		0,		0,		0
	};

	const ModPlug_Settings gSettings16000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2,
		16,
		16000,
		MODPLUG_RESAMPLE_LINEAR,
		0,		0,		0,		0,		0,		0,		0
	};

	const ModPlug_Settings gSettings22000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2,
		16,
		22000,
		MODPLUG_RESAMPLE_LINEAR,
		0,		0,		0,		0,		0,		0,		0
	};

	const ModPlug_Settings gSettings32000 =
	{
		MODPLUG_ENABLE_OVERSAMPLING | MODPLUG_ENABLE_NOISE_REDUCTION,

		2,
		16,
		32000,
		MODPLUG_RESAMPLE_LINEAR,
		0,		0,		0,		0,		0,		0,		0
	};



JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM * vm, void * reserved) {
  g_Vm = vm;

      JNIEnv *env;
      jobject tmp;

      g_Vm->GetEnv((void **)&env, JNI_VERSION_1_6);
      player = env->FindClass("com/example/SanAngeles/PlayerThread");
      //LOGV("AAAAAAAAAAA  %p  FOOOOOOOOOOOOOOOOOOO\n", player);

      tmp = player;
      //LOGV("6666123123\n");
      player = (jclass)env->NewGlobalRef(player);
      //LOGV("7777\n");
      env->DeleteLocalRef(tmp);

      //LOGV("AAAAAAAAAAA  %p  FOOOOOOOOOOOOOOOOOOO\n", player);

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

  //LOGV("1111111111111111  %p  FOOOOOOOOOOOOOOOOOOO\n", Callbacks::PumpAudio);

  gameController->CreateThread(Callbacks::PumpAudio);

  //LOGV("2222222222222222222222    FOOOOOOOOOOOOOOOOOOO\n");

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






/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_Init
 * Signature: ()Z
 */
//JNIEXPORT jboolean JNICALL Java_com_examples_modplayer_PlayerThread_ModPlug_1Init
// (JNIEnv *env, jclass cls)
/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_Init
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1Init(JNIEnv *env, jclass cls, jint rate)
{
  // if trying to make this truly re-entrant, separate buffers could be allocated here

	//__android_log_print(ANDROID_LOG_INFO, "PA_AND_lib", "Initializing modplug with rate %d", rate);

	//usleep(16*1000);

	switch (rate) {
	case 8000:
		ModPlug_SetSettings(&gSettings8000);
		break;
	case 16000:
		ModPlug_SetSettings(&gSettings16000);
		break;
	case 22000:
		ModPlug_SetSettings(&gSettings22000);
		break;
	case 32000:
		ModPlug_SetSettings(&gSettings32000);
		break;
	case 44100:
		// this is the default, so settings needn't be changed
		break;
	}


  return 1;
}

/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_JLoad
 * Signature: ([BI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1JLoad(JNIEnv *env, jobject obj, jbyteArray buffer, jint size)
{
  int csize = (int) size;

  // set the current sample as already beyond end of buffer (so a reload happens immediately)
  //currsample = SAMPLEBUFFERSIZE+1;
  // convert from Java buffer into a C buffer
  Cbuffer = (void *) env->GetByteArrayElements(buffer, 0);
  currmodFile = ModPlug_Load(Cbuffer, csize);
  
  if (currmodFile) {
    return 1;
  }
  else {
    return 0;
  }
}

/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_JGetName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1JGetName(JNIEnv *env, jobject obj)
{
  return env->NewStringUTF(ModPlug_GetName(currmodFile));
}

/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_JNumChannels
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1JNumChannels(JNIEnv *env, jobject obj)
{
  jint numchannels;

  numchannels = ModPlug_NumChannels(currmodFile);
  return numchannels;
}


/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_JGetSoundData
 * Signature: ([SI)I
 */
JNIEXPORT jint JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1JGetSoundData(JNIEnv *env, jobject obj, jshortArray jbuffer, jint size)
{
  LOGV("FUUUUUUUUUUUUUUUUUUUUUUUUUUU    FOOOOOOOOOOOOOOOOOOO\n");
  /*
  jint smpsize = 0;

   if (currmodFile == 0)
     return 0;

  // IN THIS MODE, WE READ IN EXACTLY HOW MUCH JAVA requested to improve frame rate
  smpsize = ModPlug_Read(currmodFile, samplebuffer, size*sizeof(jshort));
  currsample = 0;

  // now convert the C sample buffer data to a java short array
  if (size && samplebuffer && (smpsize || currsample < SAMPLEBUFFERSIZE)) {
    env->SetShortArrayRegion(jbuffer, 0 ,size, (jshort *) (((char *) samplebuffer)+currsample));
    currsample += size*sizeof(jshort);
    return size;
  }
  else {
    return 0;
  }
  */
}

/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_JUnload
 * Signature: ([BI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1JUnload(JNIEnv *env, jclass cls, jbyteArray notused, jint size)
{
  if (currmodFile) {
    ModPlug_Unload(currmodFile);
    currmodFile = 0;
  }

  return 1;
}

/*
 * Class:     com_examples_modplayer_PlayerThread
 * Method:    ModPlug_CloseDown
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_example_SanAngeles_PlayerThread_ModPlug_1CloseDown(JNIEnv *env, jclass cls) {
  // maybe for a proper re-entrant library, need to handle shutdown stuff, deallocting buffers
  // etc.  but for my crappy, hacky single-entry version, do nothing much...
  return 1;
}
