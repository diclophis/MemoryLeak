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
  void SetAssets(std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
  void ResizeScreen(int width, int height);
  void DrawScreen(float rotation);
  int RunThread();
  void CreateThread(void ());
  static void *EnterThread(void *);
  virtual int Simulate() = 0;
  virtual void Hit(float x, float y, int hitState) = 0;
  virtual void RenderModelPhase() = 0;
  virtual void RenderSpritePhase() = 0;
  virtual void CreateFoos() = 0;
  virtual void DestroyFoos() = 0;
  bool WaitVsync();
  void DoAudio(short buffer[], int bytes);
  void RenderModelRange(unsigned int s, unsigned int e);
  void RenderSpriteRange(unsigned int s, unsigned int e, foofoo *batch_foo = NULL);
  void glueLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
  void gluePerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
  void (*m_SimulationThreadCleanup)();
  bool (*m_WebViewMessagePusher)(const char *);
  const char *(*m_WebViewMessagePopper)();
  void SetWebViewPushAndPop(bool (const char *), const char *(*)());
  char m_WebViewFunctionBuffer[1024];
  char m_WebViewFunctionBufferTwo[1024];
  char *CreateWebViewFunction(const char *fmt, ...);
  const char *PopMessageFromWebView();
  bool PushMessageToWebView(char *messageToPush);
  bool Active();
  void StopSimulation();
  void StartSimulation();
  void PauseSimulation();
  void LoadSound(int i);
  void LoadModel(int i, int s, int e);

  static void Start(int i, int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, bool (const char *), const char *(*)(), void ());
  static void CurrentGameSetAssets(std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
  static void CurrentGamePause();
  static void CurrentGameHit(float x, float y, int hitState);
  static void CurrentGameResizeScreen(int width, int height);
  static void CurrentGameDrawScreen(float rotation);
  static void CurrentGameDoAudio(short buffer[], int bytes);
  static bool CurrentGame();
  static void CurrentGameStart();
  static void CheckGL(const char *s);

  bool m_IsSceneBuilt;
  bool m_IsScreenResized;
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
  pthread_cond_t m_ResumeCond;
  pthread_mutex_t m_Mutex;
  pthread_mutex_t m_Mutex2;
  pthread_t m_Thread;
  std::vector<GLuint> *m_Textures;
  std::vector<foo *> *m_ModelFoos;
  std::vector<foo *> *m_LevelFoos;
  std::vector<foo *> *m_SoundFoos;
  std::vector<Model *> m_Models;
  std::vector<SpriteGun *> m_AtlasSprites;
  std::vector<foofoo *> m_FooFoos;
  std::vector<ModPlugFile *>m_Sounds;
  int m_RenderIndex;
  short *m_AudioMixBuffer;
  int m_AudioBufferSize;
  bool m_IsPushingAudio;
  float m_Zoom;
  float m_Balance;
  float m_WebViewTimeout;
  pthread_mutex_t m_ScriptMutex;
  pthread_t m_ScriptThread;
  int m_SpriteCount;
  int m_ModelCount;
  bool m_IsThreeD;

  int m_LastDraw;
  int m_CurrentDraw;

  StateFoo *m_StateFoo;


};
