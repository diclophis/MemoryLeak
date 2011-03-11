//

#include "MemoryLeak.h"

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "Model.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"


#include "Engine.h"



//static void timer_cb(EV_P_ struct ev_timer *w, int revents);
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
		//exit(code);
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
			free(conn->url);
			curl_easy_cleanup(easy);
			free(conn);
		}
	}
}

#define EV_READ 1
#define EV_WRITE 2


/* Update the event timer after curl_multi library calls */ 
static int multi_timer_cb(CURLM *multi, long timeout_ms, GlobalInfo *g)
{
	
	//DPRINT("%s %li\n", __PRETTY_FUNCTION__,  timeout_ms);
	/*
	//ev_timer_stop(g->loop, &g->timer_event);
	if (timeout_ms > 0)
	{
		double  t = timeout_ms / 1000;
		//ev_timer_init(&g->timer_event, timer_cb, t, 0.);
		//ev_timer_start(g->loop, &g->timer_event);
		LOGV("wtf A???\n");
		do_this_in_tick(g, 0);
	} else {
		//timer_cb(g->loop, &g->timer_event, 0);
		LOGV("wtf B??\n");
	}
	*/
	//do_this_in_tick(g, 0);

	return 0;
}


/* Called by libevent when our timeout expires */ 
static void do_this_in_tick(GlobalInfo *g, int revents)
{
	//DPRINT("%s g %p revents %i\n", __PRETTY_FUNCTION__, g, revents);
	
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
	//printf("%s  \n", __PRETTY_FUNCTION__);
	if ( f )
	{
		//if ( f->evset )
		//	ev_io_stop(g->loop, &f->ev);
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
	//DPRINT("%s e %p s %i what %i cbp %p sockp %p\n", __PRETTY_FUNCTION__, e, s, what, cbp, sockp);
	
	GlobalInfo *g = (GlobalInfo*) cbp;
	SockInfo *fdp = (SockInfo*) sockp;
	const char *whatstr[]={ "none", "IN", "OUT", "INOUT", "REMOVE"};
	
	LOGV("socket callback: s=%d e=%p what=%s ", s, e, whatstr[what]);
	
	if ( what == CURL_POLL_REMOVE )
	{
		LOGV("\n");
		remsock(fdp, g);
	} else
	{
		if ( !fdp )
		{
			//LOGV("Adding data: %s\n", whatstr[what]);
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
	ConnInfo *conn = (ConnInfo*) data;
	(void)ptr;
	(void)conn;
	return realsize;
}

/* CURLOPT_PROGRESSFUNCTION */ 
static int prog_cb (void *p, double dltotal, double dlnow, double ult, double uln)
{
	ConnInfo *conn = (ConnInfo *)p;
	(void)ult;
	(void)uln;
	
	//LOGV("Progress: %s (%g/%g)\n", conn->url, dlnow, dltotal);
	return 0;
}

/* Create a new easy handle, and add it to the global curl_multi */ 
static void new_conn(char *url, GlobalInfo *g, CURLSH *share)
{
	ConnInfo *conn;
	CURLMcode rc;
	
	conn = (ConnInfo *)calloc(1, sizeof(ConnInfo));
	memset(conn, 0, sizeof(ConnInfo));
	conn->error[0]='\0';
	
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
	curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_TIME, 3L);
	curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_LIMIT, 10L);
	curl_easy_setopt(conn->easy, CURLOPT_TIMEOUT, 30L);
	curl_easy_setopt(conn->easy, CURLOPT_CONNECTTIMEOUT, 5L);
	//curl_easy_setopt(conn->easy, CURLOPT_SHARE, share);


	//LOGV("Adding easy %p to multi %p (%s)\n", conn->easy, g->multi, url);
	
	rc = curl_multi_add_handle(g->multi, conn->easy);
	mcode_or_die("new_conn: curl_multi_add_handle", rc);
	
	/* note that the add_handle() will set a time-out to trigger very soon so
     that the necessary socket_action() call will be called by this app */ 
}




/* we don't call any curl_multi_socket*() function yet as we have no handles
 added! */ 

//ev_loop(g.loop, 0);
//curl_multi_cleanup(g.multi);


#ifndef DESKTOP
static void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
	GLfloat xmin, xmax, ymin, ymax;
	
	ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	
	glFrustumx(
			   (GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
			   (GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
			   (GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536)
			   );
}
#endif

#ifdef DESKTOP
  #define glOrthof glOrtho
#endif

Engine::~Engine() {
  LOGV("dealloc mofo\n");
}


Engine::Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : m_ScreenWidth(w), m_ScreenHeight(h), m_Textures(&t), m_ModelFoos(&m), m_LevelFoos(&l), m_SoundFoos(&s), m_AudioBufferSize(bs), m_AudioDivisor(sd) {

	m_IsSceneBuilt = false;
	
	pthread_cond_init(&m_VsyncCond, NULL);
	pthread_cond_init(&m_AudioSyncCond, NULL);
	pthread_mutex_init(&m_Mutex, NULL);

	m_SimulationTime = 0.0;		
	m_GameState = -1;
	
	m_Importer.SetIOHandler(new FooSystem(*m_Textures, *m_ModelFoos));
	
	/*
	//m_FooFoos.resize(m_ModelFoos->size());
	int m_PostProcessFlags = aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality; 
	//aiProcess_ImproveCacheLocality | aiProcess_GenNormals; //aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_Triangulate;
	for (unsigned int i = 0; i<m_ModelFoos->size(); i++) {
		char path[128];
		snprintf(path, sizeof(s), "%d", i);
		m_Importer.ReadFile(path, m_PostProcessFlags);	
		if (i>0 && i<3) {
			m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 75));
		} else {
			m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
		}
		m_Importer.FreeScene();
	}
	*/
	

	ResizeScreen(m_ScreenWidth, m_ScreenHeight);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_NORMALIZE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST );
	
	glShadeModel( GL_SMOOTH );
	
	

	
	
	
	glLoadIdentity();

	

	//4458 vs 1114
	//m_AudioDivisor = 3;

	void *buffer = (void *)malloc(sizeof(char) * m_SoundFoos->at(0)->len);
	fseek(m_SoundFoos->at(0)->fp, m_SoundFoos->at(0)->off, SEEK_SET);
	size_t r = fread(buffer, 1, m_SoundFoos->at(0)->len, m_SoundFoos->at(0)->fp);
  if (r > 0) { 
    m_Sounds.push_back(ModPlug_Load(buffer, m_SoundFoos->at(0)->len));
  }

	m_AudioBuffer = new unsigned char[m_AudioBufferSize];
	m_AudioSilenceBuffer = new unsigned char[m_AudioBufferSize];
	memset(m_AudioSilenceBuffer, 0, m_AudioBufferSize);
	m_IsPushingAudio = false;
	m_PumpedAudioLastTick = false;
	m_SkipLimit = 0;
}


void Engine::CreateThread(void *(*sr)(void *, int, int)) {
	start_routine = sr;
	pthread_create(&m_Thread, 0, Engine::EnterThread, this);
}


void *Engine::EnterThread(void *obj) {
	reinterpret_cast<Engine *>(obj)->RunThread();
	return NULL;
}


void Engine::PauseThread() {
	pthread_mutex_lock(&m_Mutex);
	m_GameState = 0;
	pthread_mutex_unlock(&m_Mutex);
}


void Engine::WaitVsync() {
  pthread_mutex_lock(&m_Mutex);
  pthread_cond_wait(&m_VsyncCond, &m_Mutex);
  pthread_mutex_unlock(&m_Mutex);
}

void Engine::WaitAudioSync() {
  pthread_mutex_lock(&m_Mutex);
  pthread_cond_wait(&m_AudioSyncCond, &m_Mutex);
  pthread_mutex_unlock(&m_Mutex);
}


int Engine::RunThread() {
	

	curl_version_info_data*info=curl_version_info(CURLVERSION_NOW);
	if (info->features&CURL_VERSION_ASYNCHDNS) {
		printf( "ares enabled\n");
	} else {
		printf( "ares NOT enabled\n");
	}
	
	memset(&g, 0, sizeof(GlobalInfo));
	
	curl_global_init(CURL_GLOBAL_ALL);

	//share = curl_share_init();
	//curl_share_setopt(share, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS); 
	
	
	g.multi = curl_multi_init();
	
	curl_multi_setopt(g.multi, CURLMOPT_SOCKETFUNCTION, sock_cb);
	curl_multi_setopt(g.multi, CURLMOPT_SOCKETDATA, &g);
	curl_multi_setopt(g.multi, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
	curl_multi_setopt(g.multi, CURLMOPT_TIMERDATA, &g);
	
	char s[1024];
	//GlobalInfo *g = (GlobalInfo *)w->data;
	sprintf(s, "http://qa.api.openfeint.com/internal/revision");

	//new_conn(s, &g, &share);  /* if we read a URL, go get it! */ 
	
	Build();
	
	m_IsSceneBuilt = true;

	double t1, t2, averageWait;
	timeval tim;
	gettimeofday(&tim, NULL);

	int buffer_position = 0;

	gettimeofday(&tim, NULL);
	t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	double interp = 1.0;

	while (m_GameState != 0) {
				
		do_this_in_tick(&g, 0);
		
		if (g.still_running < 1) {
			//LOGV("new url\n");
			//new_conn(s, &g, &share);  /* if we read a URL, go get it! */ 
		}

		gettimeofday(&tim, NULL);
		t2=tim.tv_sec+(tim.tv_usec/1000000.0);
		averageWait = t2 - t1;
		gettimeofday(&tim, NULL);
		t1=tim.tv_sec+(tim.tv_usec/1000000.0);
		
		if (averageWait > (1.0 / 30.0)) {
			LOGV("avg: %f %f\n", averageWait, 1.0 / 30.0);
		} else {
			for (unsigned int i=0; i<interp; i++) {
				m_DeltaTime = (averageWait / interp);
				m_SimulationTime += (m_DeltaTime);
				m_GameState = Simulate();
			}
		}
		
		//WaitAudioSync();

		if (m_PumpedAudioLastTick) {			
			if (m_IsPushingAudio) {
				int len = m_AudioBufferSize / m_AudioDivisor;
				
				ModPlug_Read(m_Sounds[0], m_AudioBuffer, len);
				m_PumpedAudioLastTick = start_routine(m_AudioBuffer, buffer_position, m_AudioDivisor);
			} else {
				m_PumpedAudioLastTick = start_routine(m_AudioSilenceBuffer, buffer_position, m_AudioDivisor);
			}
		} else {
			if (m_SkipLimit++ > 0) {
				//LOGV("resume\n");
				//WaitAudioSync();
				m_PumpedAudioLastTick = true;
				m_SkipLimit = 0;
			}
		}
		
		WaitVsync();
	}	
	return m_GameState;
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
	pthread_cond_signal(&m_AudioSyncCond);
	if (m_IsSceneBuilt) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		//glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		//glEnable(GL_LIGHTING);
		glDepthFunc(GL_LESS);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			//gluPerspective(40.0 + fastAbs(fastSinf(m_SimulationTime * 0.01) * 20.0), (float)m_ScreenWidth / (float)m_ScreenHeight, 0.1, 500.0);		
			gluPerspective(50, (float)m_ScreenWidth / (float)m_ScreenHeight, 0.5, 1000.0);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				glLoadIdentity();
				gluLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);
				RenderModelPhase();
				Model::ReleaseBuffers();
			}
			glPopMatrix();
		}
		glPopMatrix();
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
			float zoom = m_SimulationTime; //2.0 + (fastAbs(fastSinf(m_SimulationTime * 0.5)) * 10.0);
			glOrthof((-m_ScreenHalfHeight*m_ScreenAspect) * zoom, (m_ScreenHalfHeight*m_ScreenAspect) * zoom, (-m_ScreenHalfHeight) * zoom, m_ScreenHalfHeight * zoom, 1.0f, -1.0f );
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
}
