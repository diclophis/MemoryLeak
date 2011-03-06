//
//  Engine.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//

//#undef __STDC__

#define CHAR_WIDTH 0.1			/* ogl tex coords :: Based on not quite 42 chars per line! */
#define CHAR_HEIGHT 0.1

// Initial setup only.
#define CHAR_PIXEL_W 20 // 8
#define CHAR_PIXEL_H 23 // 9

#define MAX_CHAR_BUFFER 256

#define ONE_CHAR_SIZE_V 12				// Floats making up one TRIANGLE_STRIP Vertices
#define ONE_CHAR_SIZE_T 8				// Floats making up one TRIANGLE_STRIP TexCoords

#define FONT_TEXTURE_ATLAS_WIDTH 10		// Characters per line in Atlas
#define FONT_TEXTURE_ATLAS_LINES 10		// Lines of characters in the Atlas


class Engine {
	
public:
	
	//Engine(const Engine&);
	//Engine& operator=(const Engine&);


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

};
