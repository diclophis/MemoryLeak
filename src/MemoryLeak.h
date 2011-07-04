// MemoryLeak Engine

#define PTM_RATIO 32 // pixels to metre ratio

#ifdef ANDROID_NDK
#include <android/log.h>
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libnav", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libnav", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libnav", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libnav", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libnav", __VA_ARGS__) 
#else
#define LOGV printf
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sys/poll.h>
#include <curl/curl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include <sstream>
#include <vector>
#include <string>
#include <math.h>

#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
  #ifdef DESKTOP
	#include <OpenGL/gl.h>    // Header File For The OpenGL32 Library
	#include <OpenGL/glu.h>   // Header File For The GLu32 Library
	#include <OpenGL/glext.h>   // Header File For The GLu32 Library
    #include <GLUT/glut.h>    // Header File For The GLut Library
    #define glOrthof glOrtho
  #else
    #include <OpenGLES/ES1/gl.h>
    #include <OpenGLES/ES1/glext.h>
    #define glFrustum glFrustumf
  #endif
#else
  #ifdef DESKTOP
    #include <GL/gl.h>
    #include <GL/glext.h>
    #define glOrthof glOrtho
  #endif
#endif

#ifdef ANDROID_NDK
  #include <GLES/gl.h>
  #include <GLES/glext.h>
  #define glFrustum glFrustumf
  //#include <GLES2/gl2.h>
  //#include <GLES2/gl2ext.h>

#endif


#include "OpenGLCommon.h"
#include "assimp.hpp"
#include "aiPostProcess.h"
#include <include/IOStream.h>
#include <include/IOSystem.h>
#include "foo.h"
#include "modplug.h"

#define DPRINT(x...) LOGV(x)

#define MSG_OUT stdout  

/* Global information, common to all connections */ 
typedef struct _GlobalInfo
{
	//struct ev_loop *loop;
	//struct ev_io fifo_event;
	//struct ev_timer timer_event;
	CURLM *multi;
	int still_running;
	FILE* input;
} GlobalInfo;


/* Information associated with a specific easy handle */ 
typedef struct _ConnInfo
{
	CURL *easy;
	char *url;
	GlobalInfo *global;
	char error[CURL_ERROR_SIZE];
} ConnInfo;

typedef struct _EventInfo
{
	GlobalInfo *global;
	curl_socket_t fd;
} EventInfo;


/* Information associated with a specific socket */ 
typedef struct _SockInfo
{
	curl_socket_t sockfd;
	CURL *easy;
	int action;
	long timeout;
	//struct ev_io ev;
	//int evset;
	GlobalInfo *global;
} SockInfo;

/*
extern "C" {
  #define loslib_c
  #define LUA_LIB
  #include "lua.h"
  #include "lauxlib.h"
  #include "lualib.h"
}

 #include "oolua.h"

*/


#include "Box2D.h"

#include "aiScene.h"

#include "stdarg.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Game.h"
#include "FooIO.h"
#include "RenderTexture.h"
#include "Engine.h"

#include "MLPoint.h"
#include "Sky.h"
#include "Terrain.h"
#include "Hero.h"

