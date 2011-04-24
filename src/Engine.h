//
//  Engine.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//

class Engine {
	
public:
	
	Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd);
	virtual ~Engine();

 
	void ResizeScreen(int width, int height);
	void DrawScreen(float rotation);


	int RunThread();
  void PauseThread();


	void CreateThread(void *(*sr)(void *, int, int));
	static void *EnterThread(void *);
	

  virtual void Build() = 0;
	virtual int Simulate() = 0;
	virtual void Hit(float x, float y, int hitState) = 0;
	virtual void RenderModelPhase() = 0;
	virtual void RenderSpritePhase() = 0;
  void WaitVsync();
  void WaitAudioSync();
	
	void RenderModelRange(unsigned int s, unsigned int e);

	void RenderSpriteRange(unsigned int s, unsigned int e);

	bool m_PumpedAudioLastTick;
	int m_SkipLimit;

	// World Engine
	bool m_IsSceneBuilt;
	bool m_IsViewportSet;
	float m_SimulationTime;
	float m_DeltaTime;
	int m_ScreenWidth;
	int m_ScreenHeight;
	float m_ScreenAspect;
	float m_ScreenHalfHeight;
	
  int m_GameState;
  double m_Waits[1];
  float m_CameraPosition[3];
  float m_CameraTarget[3];

  pthread_cond_t m_VsyncCond;
  pthread_cond_t m_AudioSyncCond;
	pthread_mutex_t m_Mutex;
	pthread_t m_Thread;

	std::vector<GLuint> *m_Textures;
	std::vector<foo *> *m_ModelFoos;
	std::vector<foo *> *m_LevelFoos;
	std::vector<foo *> *m_SoundFoos;
	Assimp::Importer m_Importer;
  std::vector<Model *> m_Models;
  std::vector<SpriteGun *> m_AtlasSprites;
  std::vector<foofoo *> m_FooFoos;
  std::vector<ModPlugFile *>m_Sounds;

	int m_RenderIndex;

  int m_AudioBufferSize;
  unsigned char *m_AudioBuffer;
  unsigned char *m_AudioSilenceBuffer;
	void *(* start_routine)(void *, int, int);

  bool m_IsPushingAudio;
	
	int m_AudioDivisor;
	
	GlobalInfo g;
	CURLMcode rc;
  CURLSH *share;

  float m_Zoom;

  void PingServer();

  float m_PingServerTimeout;

  ConnInfo *m_PingConn;

// This is a modified version of the function of the same name from 
// the Mesa3D project ( http://mesa3d.org/ ), which is  licensed
// under the MIT license, which allows use, modification, and 
// redistribution

  void glueLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
  void gluePerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);

//#ifdef DESKTOP
//  #define glOrthof glOrtho
//#endif

};
