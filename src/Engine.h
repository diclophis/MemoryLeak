//
//  Engine.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//

class Engine {


public:


  Engine(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
  virtual ~Engine();
  void ResizeScreen(int width, int height);
  void DrawScreen(float rotation);
  int RunThread();
  void CreateThread();
  static void *EnterThread(void *);
  virtual void Build() = 0;
  virtual int Simulate() = 0;
  virtual void Hit(float x, float y, int hitState) = 0;
  virtual void RenderModelPhase() = 0;
  virtual void RenderSpritePhase() = 0;
  void WaitVsync();
  void WaitAudioSync();
  void DoAudio(short buffer[], int bytes);
  void RenderModelRange(unsigned int s, unsigned int e);
  void RenderSpriteRange(unsigned int s, unsigned int e);
  void glueLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
  void gluePerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
  static void *EnterScriptThread(void *);
  void CreateScriptThread();
  void RunScriptThread();
  bool (*m_WebViewMessagePusher)(const char *);
  const char *(*m_WebViewMessagePopper)();
  void SetWebViewPushAndPop(bool (const char *), const char *(*)());
  char m_WebViewFunctionBuffer[1024];
  char m_WebViewFunctionBufferTwo[1024];
  char *CreateWebViewFunction(const char *fmt, ...);
  const char *PopMessageFromWebView();
  bool PushMessageToWebView(char *messageToPush);
  static Engine *CurrentGame();
  bool Active();
  static bool GameActive();
  void StopSimulation();
  void StartSimulation();
  void PauseSimulation();
  static void Start(int i, int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
  static void Begin();
  static void Stop();
  static void Pause();
  static void Init();
  static void Destroy();
  void LoadSound(int i);


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
  Assimp::Importer *m_Importer;
  std::vector<Model *> m_Models;
  std::vector<SpriteGun *> m_AtlasSprites;
  std::vector<foofoo *> m_FooFoos;
  std::vector<ModPlugFile *>m_Sounds;
  int m_RenderIndex;
  short *m_AudioMixBuffer;
  int m_AudioBufferSize;
  bool m_IsPushingAudio;
  GlobalInfo g;
  CURLMcode rc;
  CURLSH *share;
  float m_Zoom;
  float m_Balance;
  void PingServer();
  float m_PingServerTimeout;
  ConnInfo *m_PingConn;
  float m_AudioTimeout;
  OOLUA::Script *m_Script;
  pthread_mutex_t m_ScriptMutex;
  pthread_t m_ScriptThread;
};
