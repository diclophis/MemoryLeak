//

#include "MemoryLeak.h"

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Model.h"
#include "AtlasSprite.h"


#include "Engine.h"

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
	glEnable(GL_CULL_FACE);
    glEnable(GL_LIGHTING);


	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glLoadIdentity();

	

	//4458 vs 1114
	//m_AudioDivisor = 3;

	void *buffer = (void *)malloc(sizeof(char) * m_SoundFoos->at(0)->len);
	fseek(m_SoundFoos->at(0)->fp, m_SoundFoos->at(0)->off, SEEK_SET);
	size_t r = fread(buffer, 1, m_SoundFoos->at(0)->len, m_SoundFoos->at(0)->fp);
	m_Sounds.push_back(ModPlug_Load(buffer, m_SoundFoos->at(0)->len));

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
	
	Build();
	
	m_IsSceneBuilt = true;

	double t1, t2;
	timeval tim;
	gettimeofday(&tim, NULL);

	int waitedCount = 1;

	double averageWait = 0.0;

	int buffer_position = 0;

	for (unsigned int i=0; i<waitedCount; i++) {
		m_Waits[i] = 0.0;
	}

	gettimeofday(&tim, NULL);
	t1=tim.tv_sec+(tim.tv_usec/1000000.0);
	
	double interp = 1.0;

	while (m_GameState != 0) {
		
		gettimeofday(&tim, NULL);
		t2=tim.tv_sec+(tim.tv_usec/1000000.0);
		averageWait = t2 - t1;
		gettimeofday(&tim, NULL);
		t1=tim.tv_sec+(tim.tv_usec/1000000.0);
			
		if (averageWait > (1.0 / 30.0)) {
			//LOGV("slow\n");
		}
		
		for (unsigned int i=0; i<interp; i++) {
			m_DeltaTime = (averageWait / interp);
			m_SimulationTime += (m_DeltaTime);
			m_GameState = Simulate();
		}		
		
		//WaitAudioSync();

		if (m_PumpedAudioLastTick) {			
			if (m_IsPushingAudio) {
				int len = m_AudioBufferSize / m_AudioDivisor;
				//LOGV("modplug read: %d\n", len);
				ModPlug_Read(m_Sounds[0], m_AudioBuffer, len);
				//LOGV("just read this much size/len: %d %d\n", m_AudioBufferSize, len);
				m_PumpedAudioLastTick = start_routine(m_AudioBuffer, buffer_position, m_AudioDivisor);
			} else {
				m_PumpedAudioLastTick = start_routine(m_AudioSilenceBuffer, buffer_position, m_AudioDivisor);
			}
		} else {
			if (m_SkipLimit++ > 0) {
				LOGV("resume\n");
				//WaitAudioSync();
				m_PumpedAudioLastTick = true;
				m_SkipLimit = 0;
			}
		}
		
		WaitVsync();
	}	
	return m_GameState;
}


void Engine::DrawScreen(float rotation) {
	pthread_cond_signal(&m_AudioSyncCond);
	if (m_IsSceneBuilt) {
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		{
			glLoadIdentity();
			//gluPerspective(40.0 + fastAbs(fastSinf(m_SimulationTime * 0.01) * 20.0), (float)m_ScreenWidth / (float)m_ScreenHeight, 0.1, 500.0);		
			gluPerspective(60.0, (float)m_ScreenWidth / (float)m_ScreenHeight, 0.1, 500.0);		

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				
				glLoadIdentity();
				//TODO: screen rotation
				//glRotatef(m_SimulationTime, 0.0, 0.0, 1.0);
				
				
				GLfloat global_ambient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
				glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);

				
				// Define the ambient component of the first light
				GLfloat lightDiffuseLamp[] = {0.5, 0.5, 0.5, 1.0};
				GLfloat lightAmbientLamp[] = {1.0, 1.0, 1.0, 1.0};
				GLfloat lightPositionLamp[] = {0.0 + fastSinf(m_SimulationTime), 0.0, 0.0};


				glEnable(GL_LIGHT0 );
				glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuseLamp  );
				glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbientLamp  );
				glLightfv(GL_LIGHT0, GL_SPECULAR, lightDiffuseLamp  );
				glLightfv(GL_LIGHT0, GL_POSITION, lightPositionLamp );


				
				
				
				gluLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);


				
				
				for (unsigned int i=0; i<m_Models.size(); i++) {
					m_Models[i]->Render();
				}
				Model::ReleaseBuffers();
			}
			glPopMatrix();
		}
		glPopMatrix();
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE, GL_ONE);
		glMatrixMode(GL_PROJECTION);

		glPushMatrix();
		{
			glLoadIdentity();
			glOrthof(-m_ScreenHalfHeight*m_ScreenAspect, m_ScreenHalfHeight*m_ScreenAspect, -m_ScreenHalfHeight, m_ScreenHalfHeight, 1.0f, -1.0f );
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			{
				glLoadIdentity();
				Render();
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
}