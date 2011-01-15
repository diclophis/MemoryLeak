//

#include "MemoryLeak.h"

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Model.h"
#include "AtlasSprite.h"


#include "Engine.h"


Engine::~Engine() {
  LOGV("dealloc mofo\n");
}


Engine::Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs) : m_ScreenWidth(w), m_ScreenHeight(h), m_Textures(&t), m_ModelFoos(&m), m_LevelFoos(&l), m_SoundFoos(&s), m_AudioBufferSize(bs) {

	m_IsSceneBuilt = false;
	
  pthread_cond_init(&m_VsyncCond, NULL);
  pthread_cond_init(&m_AudioSyncCond, NULL);
	pthread_mutex_init(&m_Mutex, NULL);

	m_SimulationTime = 0.0;		
	m_GameState = -1;

	m_Importer.SetIOHandler(new FooSystem(*m_Textures, *m_ModelFoos));

	char path[128];
	int m_PostProcessFlags =  aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_Triangulate;
	for (unsigned int i = 0; i<m_ModelFoos->size(); i++) {
		snprintf(path, sizeof(s), "%d", i);
		m_Importer.ReadFile(path, m_PostProcessFlags);	
		m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene()));
		m_Importer.FreeScene();
	}

	ResizeScreen(m_ScreenWidth, m_ScreenHeight);
	
	glMatrixMode(GL_MODELVIEW);
	
	glEnable(GL_TEXTURE_2D);
	//glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE);

	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glLoadIdentity();


  void *buffer = (void *)malloc(sizeof(char) * m_SoundFoos->at(0)->len);
	fseek(m_SoundFoos->at(0)->fp, m_SoundFoos->at(0)->off, SEEK_SET);
	size_t r = fread(buffer, 1, m_SoundFoos->at(0)->len, m_SoundFoos->at(0)->fp);
  m_Sounds.push_back(ModPlug_Load(buffer, m_SoundFoos->at(0)->len));

  //m_AudioBuffer = (unsigned char *)malloc(m_AudioBufferSize * sizeof(unsigned char));
  //m_AudioSilenceBuffer = (unsigned char *)malloc(m_AudioBufferSize * sizeof(unsigned char));
	
	
	//std::fill(m_AudioBuffer, m_AudioBuffer+m_AudioBufferSize, 0);
	//std::fill(m_AudioSilenceBuffer, m_AudioBuffer+m_AudioBufferSize, 0);
	
	
	m_AudioBuffer = new unsigned char[m_AudioBufferSize];
	m_AudioSilenceBuffer = new unsigned char[m_AudioBufferSize];



  m_IsPushingAudio = false;

}


void Engine::CreateThread(void *(*sr)(void *, int)) {
  //LOGV("3333333333333333333333333  %p  FOOOOOOOOOOOOOOOOOOO\n", sr);
	start_routine = sr;
	pthread_create(&m_Thread, 0, Engine::EnterThread, this);
  //LOGV("4444444444444444444444444444444 %p %p   FOOOOOOOOOOOOOOOOOOO\n", this, sr);
  //LOGV("4444444444444444444444444444444 %p   FOOOOOOOOOOOOOOOOOOO\n", start_routine);
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

	Build();
	
	start_routine(m_AudioSilenceBuffer, 0);
	start_routine(m_AudioSilenceBuffer, 0);
	start_routine(m_AudioSilenceBuffer, 0);
	
	
	m_IsSceneBuilt = true;

  int ticks = 0;
	
  double t1, t2;
  timeval tim;
  gettimeofday(&tim, NULL);

  int waitedCount = 1;
  int waitedIndex = 0;

  double waitSum = 0.0;
  double averageWait = 0.0;

  int buffer_position = 0;

  for (unsigned int i=0; i<waitedCount; i++) {
    m_Waits[i] = 0.0;
  }

  gettimeofday(&tim, NULL);
  t1=tim.tv_sec+(tim.tv_usec/1000000.0);

	while (m_GameState != 0) {
    //if (pthread_mutex_lock(&m_Mutex) == 0) {

      gettimeofday(&tim, NULL);
      t2=tim.tv_sec+(tim.tv_usec/1000000.0);

      averageWait = t2 - t1;

      gettimeofday(&tim, NULL);
      t1=tim.tv_sec+(tim.tv_usec/1000000.0);

      /*
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
      */

      if (averageWait > (1.0 / 19.0)) {
        LOGV("slow avg: %f %f\n", averageWait, 1.0 / 19.0);
      }

        double interp = 1.0;
        for (unsigned int i=0; i<interp; i++) {
          m_DeltaTime = averageWait / interp;
          m_SimulationTime += (m_DeltaTime);
          m_GameState = Simulate();
        }

      //if ((buffer_position++ % div) == 0) {
      //}

      //WaitAudioSync();

      if (m_IsPushingAudio) {
        //LOGV("pump audio %f\n", m_DeltaTime);
        //if ((tick++ % 2) == 0) {
        //  LOGV("pump\n");
        //  ModPlug_Read(m_Sounds[0], m_AudioBuffer, (m_AudioBufferSize / 2) * sizeof(short));
        //}
        int div = 32;
        int len = m_AudioBufferSize / div;
        //LOGV("pump audio %d %d\n", m_AudioBufferSize, div);
        ModPlug_Read(m_Sounds[0], m_AudioBuffer, len);
        //LOGV("write audio %d\n", buffer_position);
        start_routine(m_AudioBuffer, buffer_position);
      } else {
        start_routine(m_AudioSilenceBuffer, buffer_position);
      }

    WaitVsync();
  }

  LOGV("exiting tick thread\n");
	
	return m_GameState;
}


void Engine::DrawScreen(float rotation) {
	pthread_cond_signal(&m_AudioSyncCond);
	if (m_IsSceneBuilt) {
		glPushMatrix();
		{
			
			glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glRotatef(rotation, 0.0, 0.0, 1.0);

			/*
			gluLookAt(
			m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2],
			m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2],
			0.0, 1.0, 0.0
			);
			
			for (unsigned int i=0; i<m_Models.size(); i++) {
				m_Models[i]->Render();
			}
			*/
			
			Render();
		}
		glPopMatrix();
	  pthread_cond_signal(&m_VsyncCond);
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
	//glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrthof(0, 480, 320, 0, 1, -1);
	
	float aspect = (float)m_ScreenWidth / (float)m_ScreenHeight;
    float halfHeight = m_ScreenHeight * 0.5;
    glOrthof( -halfHeight*aspect, halfHeight*aspect, -halfHeight, halfHeight, 1.0f, -1.0f );
	
	
	//gluPerspective(20.0, (float)width / (float)height, 0.1, 500.0);
}
