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
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

#include <sstream>
#include <vector>
#include <string>
#include <math.h>
#include <iomanip>
#include <sstream>

#ifdef DESKTOP
  #ifndef __APPLE__
    #define GLU_PERSPECTIVE gluPerspective
    #define glOrthof glOrtho
    #define GLfixed GLfloat
    #define glFrustumx glFrustum
  #else
    //#define GLfixed GLfloat
    #define glFrustumx glFrustum
    #define GLU_PERSPECTIVE gluPerspective
  #endif
#else
  #define GLU_PERSPECTIVE gluePerspective
#endif

#define GL_GLEXT_PROTOTYPES

#ifdef __APPLE__
  #ifdef DESKTOP
	  #include <OpenGL/gl.h>    // Header File For The OpenGL32 Library
	  #include <OpenGL/glu.h>   // Header File For The GLu32 Library
	  #include <OpenGL/glext.h>   // Header File For The GLu32 Library
    #include <GLUT/glut.h>    // Header File For The GLut Library
    #define glOrthof glOrtho
  #else
    #ifdef USE_GLES2
      #include <OpenGLES/ES2/gl.h>
      #include <OpenGLES/ES2/glext.h>
    #endif

      #include <OpenGLES/ES1/gl.h>
      #include <OpenGLES/ES1/glext.h>

    #define glFrustum glFrustumf
  #endif
#else
  #ifdef DESKTOP
    #include <GL/gl.h>
    #include <GL/glut.h>
    #include <GL/glext.h>
    #include <GL/glu.h>
  #endif
#endif

#ifdef ANDROID_NDK
  #include <GLES/gl.h>
  #include <GLES/glext.h>
  #define glFrustum glFrustumf
#endif

#ifdef DESKTOP
  #define GL_RGBA8_OES GL_RGBA8
  #ifdef EMSCRIPTEN
    #define glGenFramebuffersOES glGenFramebuffers
    #define glBindFramebufferOES glBindFramebuffer
    #define glGenRenderbuffersOES glGenRenderbuffers
    #define glBindRenderbufferOES glBindRenderbuffer
    #define glRenderbufferStorageOES glRenderbufferStorage
    #define glFramebufferRenderbufferOES glFramebufferRenderbuffer
    #define GL_FRAMEBUFFER_BINDING_OES GL_FRAMEBUFFER_BINDING
    #define GL_RENDERBUFFER_BINDING_OES GL_RENDERBUFFER_BINDING
    #define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER
    #define GL_RENDERBUFFER_OES GL_RENDERBUFFER
    #define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0
    #define GL_DEPTH_ATTACHMENT_OES GL_DEPTH_ATTACHMENT
    #define GL_DEPTH_COMPONENT16_OES GL_DEPTH_COMPONENT16
    #define glCheckFramebufferStatusOES glCheckFramebufferStatus
    #define glFramebufferTexture2DOES glFramebufferTexture2D
    #define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE
    #define glDeleteFramebuffersOES glDeleteFramebuffers
  #else
    #define glGenFramebuffersOES glGenFramebuffersEXT
    #define glBindFramebufferOES glBindFramebufferEXT
    #define glGenRenderbuffersOES glGenRenderbuffersEXT
    #define glBindRenderbufferOES glBindRenderbufferEXT
    #define glRenderbufferStorageOES glRenderbufferStorageEXT
    #define glFramebufferRenderbufferOES glFramebufferRenderbufferEXT
    #define GL_FRAMEBUFFER_BINDING_OES GL_FRAMEBUFFER_BINDING_EXT
    #define GL_RENDERBUFFER_BINDING_OES GL_RENDERBUFFER_BINDING_EXT
    #define GL_FRAMEBUFFER_OES GL_FRAMEBUFFER_EXT
    #define GL_RENDERBUFFER_OES GL_RENDERBUFFER_EXT
    #define GL_COLOR_ATTACHMENT0_OES GL_COLOR_ATTACHMENT0_EXT
    #define GL_DEPTH_ATTACHMENT_OES GL_DEPTH_ATTACHMENT
    #define GL_DEPTH_COMPONENT16_OES GL_DEPTH_COMPONENT16
    #define glCheckFramebufferStatusOES glCheckFramebufferStatusEXT
    #define glFramebufferTexture2DOES glFramebufferTexture2DEXT
    #define GL_FRAMEBUFFER_COMPLETE_OES GL_FRAMEBUFFER_COMPLETE_EXT
    #define glDeleteFramebuffersOES glDeleteFramebuffersEXT
  #endif
 #endif


#include "OpenGLCommon.h"

#include "FileHandle.h"
#include "StateFoo.h"
#include "foo.h"
#include "nodexyz.h"

#include "modplug.h"

#define DPRINT(x...) LOGV(x)

#define MSG_OUT stdout  

#include "Box2D.h"


#include "stdarg.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "Game.h"
#include "GLES-Render.h"
#include "Engine.h"

#define random rand

#include "MLPoint.h"

#include "BulletHellWorld.h"

#include "bulletml/bulletmlparser.h"
#include "bulletml/bulletmlparser-tinyxml.h"
#include "bulletml/bulletmlrunner.h"

#include "BulletCommand.h"

#include "pnglite.h"

