//

#include "MemoryLeak.h"

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Model.h"
#include "Engine.h"


Engine::~Engine() {
  LOGV("dealloc mofo\n");
}


Engine::Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l) : m_ScreenWidth(w), m_ScreenHeight(h), m_Textures(&t), m_ModelFoos(&m), m_LevelFoos(&l) {
	LOGV("Engine::Engine\n");
	
	pthread_mutex_init(&m_Mutex, 0);

	m_SimulationTime = 0.0;		
  m_GameState = -1;

	m_Importer.SetIOHandler(new FooSystem(*m_Textures, *m_ModelFoos));

  char s[128];

  int m_PostProcessFlags =  aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_Triangulate;

  for (unsigned int i = 0; i<m_ModelFoos->size(); i++) {
    snprintf(s, sizeof(s), "%d", i);
    m_Importer.ReadFile(s, m_PostProcessFlags);	
    m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene()));
    m_Importer.FreeScene();
  }

/*
  m_Importer.ReadFile("1", m_PostProcessFlags);	
  m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene()));
  m_Importer.FreeScene();

  m_Importer.ReadFile("2", m_PostProcessFlags);	
  m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene()));
  m_Importer.FreeScene();
*/

	ResizeScreen(m_ScreenWidth, m_ScreenHeight);
	
	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glLoadIdentity();
}


void Engine::CreateThread() {
  LOGV("CreateThread()\n");
	pthread_create(&m_Thread, 0, Engine::EnterThread, this);
}


void *Engine::EnterThread(void *obj) {
  LOGV("EnterThread()\n");
	reinterpret_cast<Engine *>(obj)->RunThread();
	return NULL;
}


void Engine::PauseThread() {
  LOGV("PauseThread()\n");
  pthread_mutex_lock(&m_Mutex);
  m_GameState = 0;
  pthread_mutex_unlock(&m_Mutex);
}


int Engine::RunThread() {
  LOGV("RunThread()\n");

	Build();
	
  double t1, t2;
  timeval tim;
  gettimeofday(&tim, NULL);

  int waitedCount = 5;
  int waitedIndex = 0;

  double waitSum = 0.0;
  double averageWait = 0.0;

  for (unsigned int i=0; i<waitedCount; i++) {
    m_Waits[i] = 0.0;
  }

  gettimeofday(&tim, NULL);
  t1=tim.tv_sec+(tim.tv_usec/1000000.0);

	while (m_GameState != 0) {
    if (pthread_mutex_lock(&m_Mutex) == 0) {

      gettimeofday(&tim, NULL);
      t2=tim.tv_sec+(tim.tv_usec/1000000.0);

      m_Waits[waitedIndex] = t2 - t1;

      gettimeofday(&tim, NULL);
      t1=tim.tv_sec+(tim.tv_usec/1000000.0);
      
      waitedIndex++;
      if ((waitedIndex % waitedCount) == 0) {
        waitedIndex = 0;
      }

      waitSum = 0.0;
      for (unsigned int i=0; i<waitedCount; i++) {
        waitSum += m_Waits[i];
      }

      averageWait = waitSum / (double)waitedCount;

      if (averageWait > (1.0 / 10.0)) {
        LOGV("slow avg: %f\n", averageWait);
      } else {
        m_DeltaTime = averageWait;
        m_SimulationTime += (m_DeltaTime);
        m_GameState = Simulate();
      }
      
      pthread_mutex_unlock(&m_Mutex);
    }
  }

  LOGV("exiting tick thread\n");
	
	return m_GameState;
}


void Engine::DrawScreen(float rotation) {
	if (pthread_mutex_lock(&m_Mutex) == 0) {
    glPushMatrix();
    {
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      glRotatef(rotation, 0.0, 0.0, 1.0);

      gluLookAt(
        m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2],
        m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2],
        0.0, 1.0, 0.0
      );

      for (unsigned int i=0; i<m_Models.size(); i++) {
        m_Models[i]->Render();
      }
    }
    glPopMatrix();
    pthread_mutex_unlock(&m_Mutex);
  }
}


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


void Engine::ResizeScreen(int width, int height) {
	m_ScreenWidth = width;
	m_ScreenHeight = height;
	glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(20.0, (float)width / (float)height, 0.1, 500.0);
}
