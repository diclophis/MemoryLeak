// MemoryLeak Engine

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
#include <sys/time.h>

#include <sstream>
#include <vector>

#include "importgl.h"
#include "OpenGLCommon.h"

#include "assimp.hpp"
#include "aiPostProcess.h"
#include <include/IOStream.h>
#include <include/IOSystem.h>

#include "foo.h"
#include "modplug.h"


//libcurl
#include <stdio.h>
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

