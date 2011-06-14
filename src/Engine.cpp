//JonBardin GPL 2011

#include "MemoryLeak.h"
#include "MainMenu.h"
#include "SuperStarShooter.h"
#include "RadiantFireEightSixOne.h"

#ifdef DESKTOP
  #define GLU_PERSPECTIVE gluPerspective
  #define GLfixed GLfloat
  #define glFrustumx glFrustum
#else
  #define GLU_PERSPECTIVE gluePerspective
#endif

class Wang {
public:
  Wang() {
    //LOGV("wtf\n");
  }

	//Wang(Wang const&);

  int newGroup() {
    return 1;
  }
};

OOLUA_CLASS_NO_BASES(Wang)
  OOLUA_NO_TYPEDEFS
  OOLUA_ONLY_DEFAULT_CONSTRUCTOR
OOLUA_CLASS_END

EXPORT_OOLUA_NO_FUNCTIONS(Wang)

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static std::vector<Game *> games;
static Engine *m_CurrentGame;


static void do_this_in_tick(GlobalInfo *g, int revents);

static void mcode_or_die(const char *where, CURLMcode code)
{
	if ( CURLM_OK != code )
	{
		const char *s;
		switch ( code )
		{
			case CURLM_CALL_MULTI_PERFORM: s="CURLM_CALL_MULTI_PERFORM"; break;
			case CURLM_BAD_HANDLE:         s="CURLM_BAD_HANDLE";         break;
			case CURLM_BAD_EASY_HANDLE:    s="CURLM_BAD_EASY_HANDLE";    break;
			case CURLM_OUT_OF_MEMORY:      s="CURLM_OUT_OF_MEMORY";      break;
			case CURLM_INTERNAL_ERROR:     s="CURLM_INTERNAL_ERROR";     break;
			case CURLM_UNKNOWN_OPTION:     s="CURLM_UNKNOWN_OPTION";     break;
			case CURLM_LAST:               s="CURLM_LAST";               break;
			default: s="CURLM_unknown";
				break;
			case     CURLM_BAD_SOCKET:         s="CURLM_BAD_SOCKET";
				LOGV("ERROR: %s returns %s\n", where, s);
				/* ignore this error */ 
				return;
		}
		LOGV("ERROR: %s returns %s !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", where, s);
	}
}

/* Check for completed transfers, and remove their easy handles */ 
static void check_multi_info(GlobalInfo *g)
{
	char *eff_url;
	CURLMsg *msg;
	int msgs_left;
	ConnInfo *conn;
	CURL *easy;
	CURLcode res;
	
	//LOGV("REMAINING: %d\n", g->still_running);
	while ((msg = curl_multi_info_read(g->multi, &msgs_left))) {
		if (msg->msg == CURLMSG_DONE) {
			easy = msg->easy_handle;
			res = msg->data.result;
			curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
			curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
			
			//TODO: what is the good res????!
		
      if (res != 0) {
        LOGV("DONE: %s => (%d) %s\n", eff_url, res, conn->error);
      }

      //just remove if reuse
			curl_multi_remove_handle(g->multi, easy);
			/*
			free(conn->url);
			curl_easy_cleanup(easy);
			free(conn);
			*/
		}
	}
}


static void do_this_in_tick(GlobalInfo *g, int revents)
{
	//LOGV("%s g %p revents %i\n", __PRETTY_FUNCTION__, g, revents);
	
	//GlobalInfo *g = (GlobalInfo *)w->global;
	CURLMcode rc;
	
	//rc = curl_multi_socket_action(g->multi, CURL_SOCKET_TIMEOUT, 0, &g->still_running);

  /*
  A little note here about the return codes from the multi functions, and especially the curl_multi_perform(3): if you receive CURLM_CALL_MULTI_PERFORM, this basically means that you should call curl_multi_perform(3) again, before you select() on more actions. You don't have to do it immediately, but the return code means that libcurl may have more data available to return or that there may be more data to send off before it is "satisfied". 
  */

	int r = 0;
	rc = curl_multi_perform(g->multi, &r);
	mcode_or_die("timer_cb: curl_multi_socket_action", rc);
	check_multi_info(g);
}


/* Clean up the SockInfo structure */ 
static void remsock(SockInfo *f, GlobalInfo *g)
{
	printf("%s  \n", __PRETTY_FUNCTION__);
	if ( f )
	{
		free(f);
	}
}

/* Assign information to a SockInfo structure */ 
static void setsock(SockInfo*f, curl_socket_t s, CURL*e, int act, GlobalInfo*g)
{
	//printf("%s  \n", __PRETTY_FUNCTION__);
	f->sockfd = s;
	f->action = act;
	f->easy = e;
}


/* Initialize a new SockInfo structure */ 
static void addsock(curl_socket_t s, CURL *easy, int action, GlobalInfo *g)
{
	SockInfo *fdp = (SockInfo *)calloc(sizeof(SockInfo), 1);
	
	fdp->global = g;
	setsock(fdp, s, easy, action, g);
	curl_multi_assign(g->multi, s, fdp);
}


/* CURLMOPT_SOCKETFUNCTION */ 
static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp)
{
	DPRINT("%s e %p s %i what %i cbp %p sockp %p\n", __PRETTY_FUNCTION__, e, s, what, cbp, sockp);
	
	GlobalInfo *g = (GlobalInfo*) cbp;
	SockInfo *fdp = (SockInfo*) sockp;
	const char *whatstr[]={ "none", "IN", "OUT", "INOUT", "REMOVE"};
	
	LOGV("socket callback: s=%d e=%p what=%s ", s, e, whatstr[what]);
	
	if (what == CURL_POLL_REMOVE)
	{
		LOGV("\n");
		remsock(fdp, g);
	} else
	{
		if (!fdp)
		{
			LOGV("Adding data: %s\n", whatstr[what]);
			addsock(s, e, what, g);
		} else
		{
			LOGV("Changing action from %s to %s\n", whatstr[fdp->action], whatstr[what]);
			setsock(fdp, s, e, what, g);
		}
	}
	return 0;
}

/* CURLOPT_WRITEFUNCTION */ 
static size_t write_cb(void *ptr, size_t size, size_t nmemb, void *data)
{
	size_t realsize = size * nmemb;
	//ConnInfo *conn = (ConnInfo*) data;

	char s[realsize];
  memcpy(s, ptr, realsize);
  LOGV("recv: %s\n", s);
	return realsize;
}

/* CURLOPT_PROGRESSFUNCTION */ 
static int prog_cb (void *p, double dltotal, double dlnow, double ult, double uln)
{
	//ConnInfo *conn = (ConnInfo *)p;
	(void)ult;
	(void)uln;
	
	//LOGV("Progress: %s (%g/%g)\n", conn->url, dlnow, dltotal);
	return 0;
}

/* Create a new easy handle, and add it to the global curl_multi */ 
static void new_conn(char *url, GlobalInfo *g, CURLSH *share, ConnInfo *conn)
{

	CURLMcode rc;

  if (conn->easy == NULL) {
    LOGV("MAKE\n");
    
    conn->easy = curl_easy_init();
    if ( !conn->easy )
    {
      LOGV("curl_easy_init() failed, exiting!\n");
      //exit(2);
    }
	  conn->global = g;
	  conn->url = strdup(url);
    curl_easy_setopt(conn->easy, CURLOPT_NOSIGNAL, 1);
    //curl_easy_setopt(conn->easy, CURLOPT_DNS_USE_GLOBAL_CACHE, 1);
    curl_easy_setopt(conn->easy, CURLOPT_DNS_CACHE_TIMEOUT, -1);
    curl_easy_setopt(conn->easy, CURLOPT_URL, conn->url);
    curl_easy_setopt(conn->easy, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(conn->easy, CURLOPT_WRITEDATA, &conn);
    //curl_easy_setopt(conn->easy, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(conn->easy, CURLOPT_ERRORBUFFER, conn->error);
    curl_easy_setopt(conn->easy, CURLOPT_PRIVATE, conn);
    curl_easy_setopt(conn->easy, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(conn->easy, CURLOPT_PROGRESSFUNCTION, prog_cb);
    curl_easy_setopt(conn->easy, CURLOPT_PROGRESSDATA, conn);
    curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_TIME, 1L);
    curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_LIMIT, 32L);
    curl_easy_setopt(conn->easy, CURLOPT_TIMEOUT, 0L);
    curl_easy_setopt(conn->easy, CURLOPT_CONNECTTIMEOUT, 5L);
    //curl_easy_setopt(conn->easy, CURLOPT_SHARE, share);
  } else {
    //conn = prev_conn;
	  //LOGV("reuse\n");
    rc = curl_multi_remove_handle(g->multi, conn->easy);
    mcode_or_die("new_conn: curl_multi_remove_handle", rc);
  }


	//LOGV("Adding easy %p to multi %p (%s)\n", conn->easy, g->multi, url);
	
	rc = curl_multi_add_handle(g->multi, conn->easy);
	mcode_or_die("new_conn: curl_multi_add_handle", rc);

	/* note that the add_handle() will set a time-out to trigger very soon so
     that the necessary socket_action() call will be called by this app */ 
}


Engine::~Engine() {
  if (m_AudioBufferSize > 0) {
    delete m_AudioMixBuffer;
  }
	
  delete m_Importer;

  LOGV("dealloc mofo\n");
}


Engine::Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : m_ScreenWidth(w), m_ScreenHeight(h), m_Textures(&t), m_ModelFoos(&m), m_LevelFoos(&l), m_SoundFoos(&s) {

  m_PingServerTimeout = 0.0;

	m_IsSceneBuilt = false;
	
	pthread_cond_init(&m_VsyncCond, NULL);
	pthread_cond_init(&m_AudioSyncCond, NULL);
    pthread_mutex_init(&m_Mutex, NULL);
    
	m_SimulationTime = 0.0;		
	m_GameState = 2;
  m_Zoom = 1.0;
	
	m_Importer = new Assimp::Importer();
	
	m_Importer->SetIOHandler(new FooSystem(*m_Textures, *m_ModelFoos));
	
	ResizeScreen(m_ScreenWidth, m_ScreenHeight);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	//glShadeModel( GL_SMOOTH );
	glLoadIdentity();

  /*
	for (unsigned int i=0; i<2; i++) {
	}
  */

	m_AudioBufferSize = 0;
	m_IsPushingAudio = false;
  m_AudioTimeout = -1.0;
  m_WebViewTimeout = 0.0;
}


void Engine::CreateThread(void (theCleanup)()) {
  m_SimulationThreadCleanup = theCleanup;
    
    pthread_attr_t attr; // thread attribute    
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    
    // create the thread     
	pthread_create(&m_Thread, &attr, Engine::EnterThread, this);
}


void Engine::CreateScriptThread() {
	pthread_create(&m_ScriptThread, 0, Engine::EnterScriptThread, this);
}


void *Engine::EnterThread(void *obj) {
	reinterpret_cast<Engine *>(obj)->RunThread();
	return NULL;
}


void *Engine::EnterScriptThread(void *obj) {
	reinterpret_cast<Engine *>(obj)->RunScriptThread();
	return NULL;
}


bool Engine::WaitVsync() {
  pthread_mutex_lock(&m_Mutex);
  if (Active()) {
      pthread_cond_wait(&m_VsyncCond, &m_Mutex);
  }
  pthread_mutex_unlock(&m_Mutex);
  return true;
}

void Engine::WaitAudioSync() {
    LOGV("wtf audio lock\n");
 // pthread_mutex_lock(&m_Mutex);
  //pthread_cond_wait(&m_AudioSyncCond, &m_Mutex);
  //pthread_mutex_unlock(&m_Mutex);
}

void Engine::PingServer () {
  //LOGV("ping\n");
	char s[1024];
	sprintf(s, "http://192.168.1.144:10101/foo");

  //if (m_Ha) {
  //  m_PingConn = new_conn(s, &g, &share, NULL);
  //} else {
  //  LOGV("reuse\n");
  //new_conn(s, &g, &share, m_PingConn);
  //}
}

int Engine::RunThread() {
	Build();
	m_IsSceneBuilt = true;
	double t1, t2, averageWait;
	timeval tim;
	gettimeofday(&tim, NULL);
	gettimeofday(&tim, NULL);
	t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	double interp = 1.0;

	while (WaitVsync() && m_GameState > 0) {
        //LOGV("sim\n");

    gettimeofday(&tim, NULL);
    t2=tim.tv_sec+(tim.tv_usec/1000000.0);
    averageWait = t2 - t1;
    gettimeofday(&tim, NULL);
    t1=tim.tv_sec+(tim.tv_usec/1000000.0);
    if (m_GameState > 1) {
      // PAUSE
      LOGV("paused\n");
    } else {
      for (unsigned int i=0; i<interp; i++) {
        m_DeltaTime = (averageWait / interp);
        m_PingServerTimeout += (m_DeltaTime);
        m_SimulationTime += (m_DeltaTime);
        if (m_AudioTimeout >= 0.0) {
          m_AudioTimeout += (m_DeltaTime);
        }
        
        pthread_mutex_lock(&m_Mutex);
          if (Active()) {
              m_GameState = Simulate();
          }
        pthread_mutex_unlock(&m_Mutex);

      }
    }
    if ((m_WebViewTimeout += m_DeltaTime) > 0.125) {
      m_WebViewTimeout = 0.0;
      PopMessageFromWebView();
    }
	}

  if (m_GameState == 0) {
    //LOGV("need to go somewhre\n");
    PushMessageToWebView(CreateWebViewFunction("queue.push('memoryleak://localhost/start?0')"));

    while (m_GameState == 0) {
      LOGV("waiting stop\n");
      PopMessageFromWebView();
    //  //WaitVsync();
    }
    //while (Stopping()) {
    //  LOGV("waiting 222\n");
    //  PopMessageFromWebView();
    //  pthread_cond_signal(&m_CurrentGame->m_VsyncCond);
    //}
  } else {
    //LOGV("told to exit with known dest\n");
  }

  m_SimulationThreadCleanup();
  m_GameState = -3;
  LOGV("!!!!!!!!!!@#!@#!@#!@# wtf EXXIIIIIIIIIIIIIIIT: %d\n", m_GameState);
  pthread_exit(NULL);
	return m_GameState;
}


void Engine::PauseSimulation() {
	//pthread_mutex_lock(&m_Mutex);
	m_GameState = 2;
	//pthread_mutex_unlock(&m_Mutex);
}


void Engine::StopSimulation() {
  pthread_cond_signal(&m_CurrentGame->m_VsyncCond);
  pthread_mutex_lock(&m_Mutex);
  LOGV("stopping simulation\n");
  m_GameState = -1;
  //pthread_cond_signal(&m_VsyncCond);
  pthread_mutex_unlock(&m_Mutex);
  LOGV("done stopping simulation\n");
}


void Engine::ExitSimulation() {
	//pthread_mutex_lock(&m_Mutex);
  //LOGV("exit simulation\n");
	m_GameState = -1;
	//pthread_mutex_unlock(&m_Mutex);
}


void Engine::StartSimulation() {
	//pthread_mutex_lock(&m_Mutex);
  //LOGV("start simulation\n");
	m_GameState = 1;
	//pthread_mutex_unlock(&m_Mutex);
}


void Engine::DoAudio(short buffer[], int size) {

  memset(buffer, 0, size * sizeof(short));

  if (m_IsPushingAudio) {
    if (m_AudioBufferSize == 0) {
      m_AudioBufferSize = size;
      //LOGV("make audio buffer %d\n", size);
      m_AudioMixBuffer = new short[size];
      memset(m_AudioMixBuffer, 0, size * sizeof(short));
    }

    //ModPlug_SetMasterVolume(m_Sounds[0], 100.0);
    //ModPlug_SetMasterVolume(m_Sounds[1], 200.0);
    if (m_AudioTimeout < 0.0) {
    } else if (m_AudioTimeout > 0.0 && m_AudioTimeout < 0.75) {
      //ModPlug_Read(m_Sounds[0], buffer, size * sizeof(short));
      //ModPlug_SetMasterVolume(m_Sounds[0], 200.0);
      //ModPlug_SetMasterVolume(m_Sounds[1], 100.0);
    } else if (m_AudioTimeout > 0.25) {
      m_AudioTimeout = -1.0;
    }


    ModPlug_Read(m_Sounds[0], buffer, size * sizeof(short));

    //for (int i=0; i<size; i++) {
    //  buffer[i] = (buffer[i] + m_AudioMixBuffer[i]);
    //}
  }
}


void Engine::RenderModelRange(unsigned int s, unsigned int e) {
	for (unsigned int i=s; i<e; i++) {
		m_Models[i]->Render();
	}
}


void Engine::RenderSpriteRange(unsigned int s, unsigned int e) {
	for (unsigned int i=s; i<e; i++) {
		m_AtlasSprites[i]->Render();
	}
}


void Engine::DrawScreen(float rotation) {
	//pthread_cond_signal(&m_AudioSyncCond);
	if (m_IsSceneBuilt) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		//glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
		//glEnable(GL_LIGHTING);
		//glDepthFunc(GL_LESS); //redund here
		glMatrixMode(GL_PROJECTION);
		//glPushMatrix();
		//{
			glLoadIdentity();
			//gluPerspective(40.0 + fastAbs(fastSinf(m_SimulationTime * 0.01) * 20.0), (float)m_ScreenWidth / (float)m_ScreenHeight, 0.1, 500.0);		
			//gluePerspective(30, (float)m_ScreenWidth / (float)m_ScreenHeight, -1.0, 100.0);
			GLU_PERSPECTIVE(40.0, (float)m_ScreenWidth / (float)m_ScreenHeight, 1.0, 10000.0);
      //glOrthof(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 100.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				glLoadIdentity();
				//gluLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);
			  glueLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);
				RenderModelPhase();
				Model::ReleaseBuffers();
			}
			glPopMatrix();
		//}
		//glPopMatrix();
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_LIGHTING);
		//glDisable(GL_CULL_FACE);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE, GL_ONE);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			glOrthof((-m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (-m_ScreenHalfHeight) * m_Zoom, m_ScreenHalfHeight * m_Zoom, 1.0f, -1.0f );
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				glLoadIdentity();
				RenderSpritePhase();
				AtlasSprite::ReleaseBuffers();
			}
			glPopMatrix();
		}
		glPopMatrix();
		pthread_cond_signal(&m_VsyncCond);
	}
}


void Engine::ResizeScreen(int width, int height) {
	m_ScreenWidth = width;
	m_ScreenHeight = height;
	m_ScreenAspect = (float)m_ScreenWidth / (float)m_ScreenHeight;
	m_ScreenHalfHeight = (float)m_ScreenHeight * 0.5;
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	//glViewport(0, 0, m_ScreenWidth / 2, m_ScreenHeight / 2);
	//glClearColor(1.0, 0.0, 0.0, 1.0);
}

// This is a modified version of the function of the same name from 
// the Mesa3D project ( http://mesa3d.org/ ), which is  licensed
// under the MIT license, which allows use, modification, and 
// redistribution
void Engine::glueLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz)
{
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;
	
	/* Make rotation matrix */
	
	/* Z vector */
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag) {			/* mpichler, 19950515 */
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}
	
	/* Y vector */
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;
	
	/* X vector = Y cross Z */
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];
	
	/* Recompute Y = Z cross X */
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];
	
	/* mpichler, 19950515 */
	/* cross product gives area of parallelogram, which is < 1.0 for
	 * non-perpendicular unit-length vectors; so normalize x, y here
	 */
	
	mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}
	
	mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}
	
#define M(row,col)  m[col*4+row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M
	glMultMatrixf(m);
	
	/* Translate Eye to Origin */
	glTranslatef(-eyex, -eyey, -eyez);
	
}

/*
void Engine::gluePerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
	GLfloat xmin, xmax, ymin, ymax;
	
	ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	
	glFrustum(
			   (GLint)(xmin * 65536), (GLint)(xmax * 65536),
			   (GLint)(ymin * 65536), (GLint)(ymax * 65536),
			   (GLint)(zNear * 65536), (GLint)(zFar * 65536)
			   );
}
*/

void Engine::gluePerspective(float fovy, float aspect,
                           float zNear, float zFar)
{
    GLfloat xmin, xmax, ymin, ymax;

    ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;

    glFrustumx((GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
               (GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
               (GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536));
}


void Engine::RunScriptThread() {
  m_Script = new OOLUA::Script;
  
  m_Script->register_class<Wang>();

  int lua_code_index = 0;
  char *lua_code = (char *)malloc(sizeof(char) * m_LevelFoos->at(lua_code_index)->len);
  fseek(m_LevelFoos->at(lua_code_index)->fp, m_LevelFoos->at(lua_code_index)->off, SEEK_SET);
  fread(lua_code, sizeof(char), m_LevelFoos->at(lua_code_index)->len, m_LevelFoos->at(lua_code_index)->fp);

  bool result = false;
  result = m_Script->run_chunk(lua_code);
  if (!result) {
    LOGV("%s", OOLUA::get_last_error(m_Script->get_ptr()).c_str());
    exit(1);
  }
}


void Engine::SetWebViewPushAndPop(bool (thePusher)(const char *), const char *(*thePopper)()) {
  m_WebViewMessagePusher = thePusher;
  m_WebViewMessagePopper = thePopper;
}


char *Engine::CreateWebViewFunction(const char *fmt, ...) {
  va_list ap;
  va_start (ap, fmt);
  vsnprintf (m_WebViewFunctionBuffer, 1024 * sizeof(char), fmt, ap);
  va_end (ap);	
  return m_WebViewFunctionBuffer;	
}


const char *Engine::PopMessageFromWebView() {
  const char *s = m_WebViewMessagePopper();
  //LOGV("in engine: %s\n", s);
  return s;
}


bool Engine::PushMessageToWebView(char *messageToPush) {
  sprintf(m_WebViewFunctionBufferTwo, "javascript:(function() { %s })()", messageToPush);
  //LOGV("pushing: %s\n", m_WebViewFunctionBufferTwo);
  return m_WebViewMessagePusher(m_WebViewFunctionBufferTwo);
}


Engine* Engine::CurrentGame() {
  return m_CurrentGame;
}


void Engine::Init() {
	games.push_back(new GameImpl<MainMenu>);
	games.push_back(new GameImpl<SuperStarShooter>);
	games.push_back(new GameImpl<RadiantFireEightSixOne>);
}


void Engine::Destroy() {
  delete m_CurrentGame;
  m_CurrentGame = NULL;
}


void Engine::Start(int i, int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s,bool (thePusher)(const char *), const char *(*thePopper)(), void (theCleanup)()) {
  if (m_CurrentGame) {
    m_CurrentGame->StopSimulation();
    LOGV("gonna joinb\n");
    pthread_join(m_CurrentGame->m_Thread, NULL);
    delete m_CurrentGame;
  }

  m_CurrentGame = (Engine *)games.at(i)->allocate(w, h, t, m, l, s);
  m_CurrentGame->SetWebViewPushAndPop(thePusher, thePopper);
  m_CurrentGame->CreateThread(theCleanup);
  m_CurrentGame->StartSimulation();
}


void Engine::Begin() {
  m_CurrentGame->StartSimulation();
}


void Engine::Stop() {
  if (GameActive() && m_CurrentGame->Active()) {
    LOGV("going to stop sumulation\n");
    m_CurrentGame->StopSimulation();
  }

  /*
  while (m_CurrentGame->Stopping()) {
    sched_yield();
    LOGV("waiting %d\n", m_CurrentGame->m_GameState);
    pthread_cond_signal(&m_CurrentGame->m_VsyncCond);
  }
  */

  /*
  LOGV("waiting stoppped!@#!#!@#!@# %d\n", m_CurrentGame->m_GameState);
  pthread_join(m_CurrentGame->m_Thread, NULL);
  */
  
  //pthread_cond_signal(&m_CurrentGame->m_VsyncCond);
 
}


void Engine::Pause() {
  m_CurrentGame->PauseSimulation();
}


void Engine::Exit() {
  m_CurrentGame->ExitSimulation();
}


bool Engine::Active() {
  return (m_GameState > 0);
}


bool Engine::Stopping() {
  return (m_GameState > -3);
}


bool Engine::GameActive() {
  if (m_CurrentGame != NULL) {
    return true;
  } else {
    return false;
  }
}

void Engine::LoadSound(int i) {
  void *buffer = (void *)malloc(sizeof(char) * m_SoundFoos->at(i)->len);
  fseek(m_SoundFoos->at(i)->fp, m_SoundFoos->at(i)->off, SEEK_SET);
  size_t r = fread(buffer, 1, m_SoundFoos->at(i)->len, m_SoundFoos->at(i)->fp);
  if (r > 0) { 
    m_Sounds.push_back(ModPlug_Load(buffer, m_SoundFoos->at(i)->len));
  }
  free(buffer);
}
