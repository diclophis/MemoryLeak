//JonBardin GPL 2011


#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "MemoryLeak.h"
#include "MainMenu.h"
#include "SuperStarShooter.h"
#include "RadiantFireEightSixOne.h"
#include "SpaceShipDown.h"


static std::vector<Game *> games;
static Engine *m_CurrentGame;
static pthread_mutex_t m_GameSwitchLock;

namespace OpenSteer {
	bool updatePhaseActive = false;
	bool drawPhaseActive = false;
}


Engine::~Engine() {
  LOGV("Engine::dealloc\n");
  
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


  free(m_StateFoo);
}


Engine::Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : m_ScreenWidth(w), m_ScreenHeight(h), m_Textures(&t), m_ModelFoos(&m), m_LevelFoos(&l), m_SoundFoos(&s) {
  m_SpriteCount = 0;
  m_ModelCount = 0;

	m_IsSceneBuilt = false;
	m_IsScreenResized = false;
	
	pthread_cond_init(&m_VsyncCond, NULL);
	pthread_cond_init(&m_AudioSyncCond, NULL);
	pthread_cond_init(&m_ResumeCond, NULL);
  pthread_mutex_init(&m_Mutex, NULL);
  pthread_mutex_init(&m_Mutex2, NULL);

	m_SimulationTime = 0.0;		
	m_GameState = 2;
  m_Zoom = 1.0;
  m_Fov = 10.0;

	m_IsPushingAudio = false;

  m_StateFoo = (StateFoo *)malloc(1 * sizeof(StateFoo));

  m_CurrentSound = 0;
}


void Engine::ResetStateFoo() {

  /*
	glColor4f(1.0, 1.0, 1.0, 1.0);
  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glShadeModel(GL_FLAT);

  glEnableClientState(GL_NORMAL_ARRAY);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  */

  m_StateFoo->g_lastTexture = -1;
  m_StateFoo->g_lastElementBuffer = -1;
  m_StateFoo->g_lastInterleavedBuffer = -1;
  m_StateFoo->g_lastVertexArrayObject = -1;
}


void Engine::CreateThread(void (theCleanup)()) {
  m_SimulationThreadCleanup = theCleanup;

  /*
  pthread_attr_t attr; 
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  pthread_create(&m_Thread, &attr, Engine::EnterThread, this);
  */

	timeval tim;
	gettimeofday(&tim, NULL);
	t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  StartSimulation();
}


void Engine::SetAssets(std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) {
  m_Textures = &t;
  m_ModelFoos = &m;
  m_LevelFoos = &l;
  m_SoundFoos = &s;
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


int Engine::isExtensionSupported(const char *extension) {
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;

  /* Extension names should not have spaces. */
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;
  extensions = glGetString(GL_EXTENSIONS);
  LOGV("%s\n", extensions);
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;
    start = terminator;
  }
  return 0;
}


void Engine::DrawScreen(float rotation) {
  RunThread();
	if (m_IsSceneBuilt && m_IsScreenResized) {
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    // Render 3D
    GLU_PERSPECTIVE(m_Fov, (float)m_ScreenWidth / (float)m_ScreenHeight, 1.0, 200.0);
    glueLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);
    RenderModelPhase();
    glLoadIdentity();
    // Render 2D
    glOrthof((-m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (-m_ScreenHalfHeight) * m_Zoom, m_ScreenHalfHeight * m_Zoom, 1.0f, -1.0f);
    RenderSpritePhase();
	} else {
    ResizeScreen(m_ScreenWidth, m_ScreenHeight);
  }
}


int Engine::RunThread() {
	timeval tim;
  gettimeofday(&tim, NULL);
  t2=tim.tv_sec+(tim.tv_usec/1000000.0);
  m_DeltaTime = t2 - t1;
  gettimeofday(&tim, NULL);
  t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  if (m_DeltaTime > 0.6) {
    LOGV("SKIPPP m_DeltaTime: %f\n", m_DeltaTime);
  }
  if (m_GameState > 1) {
    //pthread_mutex_unlock(&m_Mutex);
    //pthread_cond_wait(&m_ResumeCond, &m_Mutex2);
  } else {
    m_SimulationTime += (m_DeltaTime);
    if (Active()) {
      Simulate();
    } else {
      LOGV("not active\n");
    }
  }
  m_IsSceneBuilt = true;
	return m_GameState;
}


void Engine::PauseSimulation() {
	m_GameState = 2;
}


void Engine::StopSimulation() {
  pthread_mutex_lock(&m_Mutex);
  m_IsSceneBuilt = false;
  m_GameState = -1;
  pthread_mutex_unlock(&m_Mutex);
}


void Engine::StartSimulation() {
  if (m_GameState == 2) {
    pthread_mutex_lock(&m_Mutex);
    m_GameState = 1;
    pthread_cond_signal(&m_CurrentGame->m_ResumeCond);
    pthread_mutex_unlock(&m_Mutex);
  }
}


void Engine::DoAudio(short buffer[], int size) {
  memset(buffer, 0, size);
  if (Active() && m_IsPushingAudio) {
    int read = ModPlug_Read(m_Sounds[m_CurrentSound], buffer, size);
    if (read == 0) {
      ModPlug_Seek(m_Sounds[m_CurrentSound], 0);
    }
  }
}


void Engine::RenderModelRange(unsigned int s, unsigned int e, foofoo *batch_foo) {
	for (unsigned int i=s; i<e; i++) {
		m_Models[i]->Render(m_StateFoo, batch_foo);
	}
}


void Engine::RenderSpriteRange(unsigned int s, unsigned int e, foofoo *batch_foo) {
	for (unsigned int i=s; i<e; i++) {
		m_AtlasSprites[i]->Render(m_StateFoo, batch_foo);
	}
}


void Engine::ResizeScreen(int width, int height) {
  m_ScreenWidth = width;
  m_ScreenHeight = height;
	m_ScreenAspect = (float)m_ScreenWidth / (float)m_ScreenHeight;
	m_ScreenHalfHeight = (float)m_ScreenHeight * 0.5;
  glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  m_IsScreenResized = true;
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


void Engine::Start(int i, int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, void (theCleanup)()) {
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
  m_CurrentGame->CreateThread(theCleanup);
  
  pthread_mutex_unlock(&m_GameSwitchLock);

}


void Engine::CurrentGameSetAssets(std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->SetAssets(t, m, l, s);
  } else {
    LOGV("current game is not to set assets\n");
  }
}


void Engine::CurrentGameDestroyFoos() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DestroyFoos();
  } else {
    LOGV("current game is not set to destroy foos\n\n");
  }
}


void Engine::CurrentGameCreateFoos() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->CreateFoos();
  } else {
    LOGV("current game is not set to create foos\n\n");
  }
}


void Engine::CurrentGamePause() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->PauseSimulation();
  } else {
    LOGV("current game is not set to pause\n\n");
  }
}


void Engine::CurrentGameHit(float x, float y, int hitState) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->Hit(x, y, hitState);
  } else {
    LOGV("\n\nFOOOOOOOOOOOOOOOOO\n\n\n");
  }
}


void Engine::CurrentGameResizeScreen(int width, int height) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->ResizeScreen(width, height);
  } else {
    LOGV("really??\n\n");
  }
}


void Engine::CurrentGameDrawScreen(float rotation) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DrawScreen(rotation);
  } else {
    LOGV("foooo man chuuu\n");
  }
}


void Engine::CurrentGameDoAudio(short buffer[], int bytes) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DoAudio(buffer, bytes);
  } else {
    LOGV("foooo man chuuu\n");
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
    LOGV("WTF!!!!!!!\n");
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
  m_IsPushingAudio = true;
}


void Engine::LoadModel(int i, int s, int e) {
  //aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph  cause memoryleak
  Assimp::Importer m_Importer;
	m_Importer.SetIOHandler(new FooSystem(*m_Textures, *m_ModelFoos));
	int m_PostProcessFlags = aiProcess_FlipUVs | aiProcess_ImproveCacheLocality;
	char path[128];
	snprintf(path, sizeof(s), "%d", i);
	m_Importer.ReadFile(path, m_PostProcessFlags);
  const aiScene *scene = m_Importer.GetScene();
	m_FooFoos.push_back(Model::GetFoo(scene, s, e));
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
