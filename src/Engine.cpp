//JonBardin GPL 2011



#include "MemoryLeak.h"
#include "MainMenu.h"
#include "SuperStarShooter.h"
#include "RadiantFireEightSixOne.h"
#include "SpaceShipDown.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static std::vector<Game *> games;
static Engine *m_CurrentGame;
static pthread_mutex_t m_GameSwitchLock;


Engine::~Engine() {

  if (m_AudioBufferSize > 0) {
    delete m_AudioMixBuffer;
  }

  for (std::vector<foofoo *>::iterator i = m_FooFoos.begin(); i != m_FooFoos.end(); ++i) {
    delete *i;
  }
  m_FooFoos.clear();

  for (std::vector<SpriteGun *>::iterator i = m_AtlasSprites.begin(); i != m_AtlasSprites.end(); ++i) {
    delete *i;
  }
  m_AtlasSprites.clear();

  for (std::vector<Model *>::iterator i = m_Models.begin(); i != m_Models.end(); ++i) {
    delete *i;
  }
  m_Models.clear();

  for (std::vector<ModPlugFile *>::iterator i = m_Sounds.begin(); i != m_Sounds.end(); ++i) {
    ModPlug_Unload(*i);
  }
  m_Sounds.clear();

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_TEXTURE_2D);
  //glPopMatrix();

  LOGV("dealloc Engine\n");
}


Engine::Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : m_ScreenWidth(w), m_ScreenHeight(h), m_Textures(&t), m_ModelFoos(&m), m_LevelFoos(&l), m_SoundFoos(&s) {
  m_SpriteCount = 0;
  m_ModelCount = 1;

	m_IsSceneBuilt = false;
	m_IsScreenResized = false;
	
	pthread_cond_init(&m_VsyncCond, NULL);
	pthread_cond_init(&m_AudioSyncCond, NULL);
  pthread_mutex_init(&m_Mutex, NULL);
  pthread_mutex_init(&m_Mutex2, NULL);

	m_SimulationTime = 0.0;		
	m_GameState = 2;
  m_Zoom = 1.0;

	
	m_AudioBufferSize = 0;
	m_IsPushingAudio = false;
  m_WebViewTimeout = 0.0;

	glColor4f(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  //glPushMatrix();

  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  m_IsThreeD = false;

}


void Engine::CreateThread(void (theCleanup)()) {
  m_SimulationThreadCleanup = theCleanup;
  pthread_attr_t attr; 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&m_Thread, &attr, Engine::EnterThread, this);
}


void *Engine::EnterThread(void *obj) {
  //TODO: figure out how to fucking name a thread
  reinterpret_cast<Engine *>(obj)->RunThread();
  return NULL;
}


bool Engine::WaitVsync() {
  pthread_cond_wait(&m_VsyncCond, &m_Mutex2);
  return true;
}


int Engine::RunThread() {
	double t1, t2, averageWait;
	timeval tim;
	gettimeofday(&tim, NULL);
	gettimeofday(&tim, NULL);
	t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	double interp = 1.0;
  StartSimulation();
	while (m_GameState > 0) {
    if (pthread_mutex_trylock(&m_Mutex) == 0) {
      gettimeofday(&tim, NULL);
      t2=tim.tv_sec+(tim.tv_usec/1000000.0);
      averageWait = t2 - t1;
      gettimeofday(&tim, NULL);
      t1=tim.tv_sec+(tim.tv_usec/1000000.0);
      if (m_GameState > 1) {
        LOGV("paused\n");
      } else {
        for (unsigned int i=0; i<interp; i++) {
          m_DeltaTime = (averageWait / interp);
          m_SimulationTime += (m_DeltaTime);
          if (Active()) {
            Simulate();
          }
        }
      }
      if ((m_WebViewTimeout += m_DeltaTime) > 0.05) {
        m_WebViewTimeout = 0.0;
        PopMessageFromWebView();
        //PushMessageToWebView(CreateWebViewFunction("update(%f)", m_SimulationTime));
      }
      if (m_SimulationTime > 10.0 && m_GameState != 4) {
        //m_GameState = 4;
        //PushMessageToWebView(CreateWebViewFunction("start(0)"));
      }
      m_IsSceneBuilt = true;
      pthread_mutex_unlock(&m_Mutex);
    }
    WaitVsync();
	}
  m_GameState = -3;
  m_SimulationThreadCleanup();
	return m_GameState;
}


void Engine::PauseSimulation() {
	m_GameState = 2;
}


void Engine::StopSimulation() {
  pthread_mutex_lock(&m_Mutex);

  m_IsSceneBuilt = false;
  m_GameState = -1;
  while (m_GameState != -3) {
    LOGV("trying to signal vsync\n");
    pthread_cond_signal(&m_CurrentGame->m_VsyncCond);
  }
  pthread_join(m_CurrentGame->m_Thread, NULL);

  pthread_mutex_unlock(&m_Mutex);
}


void Engine::StartSimulation() {
  if (m_GameState == 2) {
    pthread_mutex_lock(&m_Mutex);
    m_GameState = 1;
    pthread_mutex_unlock(&m_Mutex);
  }
}


void Engine::DoAudio(short buffer[], int size) {
  int buffer_size = size * sizeof(short) * 2;
  memset(buffer, 0, buffer_size);
  if (Active() && m_IsPushingAudio) {
    ModPlug_Read(m_Sounds[0], buffer, size * sizeof(short) * 2);
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
  pthread_mutex_lock(&m_Mutex);
	if (m_IsSceneBuilt && m_IsScreenResized) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    if (m_IsThreeD) {
      glueLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);
      RenderModelPhase();
    } else {
      RenderSpritePhase();
    }
	} else {
    ResizeScreen(m_ScreenWidth, m_ScreenHeight);
  }
  pthread_cond_signal(&m_VsyncCond);
  pthread_mutex_unlock(&m_Mutex);
}


void Engine::ResizeScreen(int width, int height) {
  m_ScreenWidth = width;
  m_ScreenHeight = height;
	m_ScreenAspect = (float)m_ScreenWidth / (float)m_ScreenHeight;
	m_ScreenHalfHeight = (float)m_ScreenHeight * 0.5;
  //LOGV("ResizeTo %d %d\n", m_ScreenWidth, m_ScreenHeight);
  glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  m_IsScreenResized = true;
  if (m_IsThreeD) {
		glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    GLU_PERSPECTIVE(80.0, (float)m_ScreenWidth / (float)m_ScreenHeight, 1.0, 1000.0);
  } else {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthof((-m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (-m_ScreenHalfHeight) * m_Zoom, m_ScreenHalfHeight * m_Zoom, 1.0f, -1.0f);
  }
  glMatrixMode(GL_MODELVIEW);
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
  return s;
}


bool Engine::PushMessageToWebView(char *messageToPush) {
  sprintf(m_WebViewFunctionBufferTwo, "javascript:(function() { %s })()", messageToPush);
  return m_WebViewMessagePusher(m_WebViewFunctionBufferTwo);
}


void Engine::Start(int i, int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s,bool (thePusher)(const char *), const char *(*thePopper)(), void (theCleanup)()) {
  if (games.size() == 0) {
    games.push_back(new GameImpl<MainMenu>);
    games.push_back(new GameImpl<SuperStarShooter>);
    games.push_back(new GameImpl<RadiantFireEightSixOne>);
    games.push_back(new GameImpl<SpaceShipDown>);
    pthread_mutex_init(&m_GameSwitchLock, NULL);
  }

  pthread_mutex_lock(&m_GameSwitchLock);

  if (m_CurrentGame) {
    m_CurrentGame->StopSimulation();
    delete m_CurrentGame;
  }

  m_CurrentGame = (Engine *)games.at(i)->allocate(w, h, t, m, l, s);
  m_CurrentGame->SetWebViewPushAndPop(thePusher, thePopper);
  m_CurrentGame->CreateThread(theCleanup);
  
  pthread_mutex_unlock(&m_GameSwitchLock);

}


void Engine::CurrentGamePause() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->PauseSimulation();
  } else {
    LOGV("wtsdfasdasdasdasdasdasd\n\n");
  }
}


void Engine::CurrentGameHit(float x, float y, int hitState) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->Hit(x, y, hitState);
  } else {
    //LOGV("\n\nFOOOOOOOOOOOOOOOOO\n\n\n");
  }
}


void Engine::CurrentGameResizeScreen(int width, int height) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->ResizeScreen(width, height);
  } else {
    //LOGV("\n really??\n");
  }
}


void Engine::CurrentGameDrawScreen(float rotation) {
  if (pthread_mutex_trylock(&m_GameSwitchLock) == 0) {
    if (m_CurrentGame != NULL) {
      m_CurrentGame->DrawScreen(rotation);
    } else {
      //LOGV("foooo man chuuu\n");
    }
    pthread_mutex_unlock(&m_GameSwitchLock);
  }
}


void Engine::CurrentGameDoAudio(short buffer[], int bytes) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DoAudio(buffer, bytes);
  }
}


bool Engine::CurrentGame() {
  if (m_CurrentGame != NULL) {
    return true;
  } else {
    return false;
  }
}


void Engine::CurrentGameStart() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->StartSimulation();
  } else {
    //LOGV("WTF!!!!!!!\n");
  }
}


bool Engine::Active() {
  return (m_GameState == 1);
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


void Engine::LoadModel(int i, int s, int e) {
  //aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph  cause memoryleak
  Assimp::Importer m_Importer;
	m_Importer.SetIOHandler(new FooSystem(*m_Textures, *m_ModelFoos));
	int m_PostProcessFlags = aiProcess_FlipUVs | aiProcess_ImproveCacheLocality;
	char path[128];
	snprintf(path, sizeof(s), "%d", i);
	m_Importer.ReadFile(path, m_PostProcessFlags);
	//LOGV("%s\n", m_Importer.GetErrorString());
  const aiScene *scene = m_Importer.GetScene();
	m_FooFoos.push_back(Model::GetFoo(scene, s, e));
  //delete scene;
	m_Importer.FreeScene();	
}


void Engine::CheckGL(const char *s) {
  // normally (when no error) just return
  const int lastGlError = glGetError();
  if (lastGlError == GL_NO_ERROR) return;
  
  LOGV("\n%s caused\n", s);
  switch (lastGlError)
  {
    case GL_INVALID_ENUM:      LOGV("GL_INVALID_ENUM\n\n");      break;
    case GL_INVALID_VALUE:     LOGV("GL_INVALID_VALUE\n\n");     break;
    case GL_INVALID_OPERATION: LOGV("GL_INVALID_OPERATION\n\n"); break;
    case GL_STACK_OVERFLOW:    LOGV("GL_STACK_OVERFLOW\n\n");    break;
    case GL_STACK_UNDERFLOW:   LOGV("GL_STACK_UNDERFLOW\n\n");   break;
    case GL_OUT_OF_MEMORY:     LOGV("GL_OUT_OF_MEMORY\n\n");     break;
  }
}
