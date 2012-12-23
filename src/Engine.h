//
//  Engine.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//


//#ifndef HAVE_BUILTIN_SINCOS
//#define sincos _sincos
//static void sincos (double a, double *s, double *c) {
//  *s = sin (a);
//  *c = cos (a);
//}
//#endif


class Engine {


public:


  Engine(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s);
  virtual ~Engine();
  void ResetStateFoo();
  void ResizeScreen(int width, int height);
  void DrawScreen(float rotation);
  int Run();
  virtual int Simulate() = 0;
  virtual void Hit(float x, float y, int hitState) = 0;
  virtual void RenderModelPhase() = 0;
  virtual void RenderSpritePhase() = 0;
  virtual void CreateFoos() = 0;
  virtual void DestroyFoos() = 0;
  void DoAudio(short buffer[], int bytes);
  void RenderSpriteRange(unsigned int s, unsigned int e, foofoo *batch_foo = NULL);
  void glueLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz);
  void gluePerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
  bool Active();
  void StopSimulation();
  void StartSimulation();
  void PauseSimulation();
  void LoadSound(int i);
  void LoadTexture(int i);
  void ClearSprites();
  int isExtensionSupported(const char *extension);
 
  static void PushBackFileHandle(int collection, FILE *file, unsigned int offset, unsigned int length);
  static void Start(int i, int w, int h);
  static void CurrentGameDestroyFoos();
  static void CurrentGameCreateFoos();
  static void CurrentGamePause();
  static void CurrentGameHit(float x, float y, int hitState);
  static void CurrentGameResizeScreen(int width, int height);
  static void CurrentGameDrawScreen(float rotation);
  static void CurrentGameDoAudio(short buffer[], int bytes);
  static bool CurrentGame();
  static void CheckGL(const char *s);
  static void CurrentGameStart();
  static void WarnAboutGameFailure(const char *s);

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
  float m_CameraPosition[3];
  float m_CameraTarget[3];

  std::vector<FileHandle *> *m_TextureFileHandles;
  std::vector<FileHandle *> *m_ModelFileHandles;
  std::vector<FileHandle *> *m_LevelFileHandles;
  std::vector<FileHandle *> *m_SoundFileHandles;

  std::vector<GLuint> m_Textures;
  std::vector<SpriteGun *> m_AtlasSprites;
  std::vector<foofoo *> m_FooFoos;
  std::vector<ModPlugFile *> m_Sounds;

  // Draw foos, really need a better name than foofoo
  std::vector<foofoo *> m_Batches;

  bool m_IsPushingAudio;
  float m_Zoom;

  int m_SpriteCount;
  int m_ModelCount;

  StateFoo *m_StateFoo;

  int m_CurrentSound;
  float m_Fov;
	double t1, t2;

  GLuint program;
  
  void ortho(GLfloat *m, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearZ, GLfloat farZ);
  void identity(GLfloat *m);
      
  void glTranslatef(float x, float y, float z);
  GLfloat ProjectionMatrix[16];
  GLuint v;
  GLuint f;
  const char *p;
  char msg[512];

  float ltx;
  float lty;
  float ltz;

  //network stuff
  int done;
  int SocketFD;
  unsigned int get_all_buf(int sock, char* output, unsigned int maxsize);
  void iter(void *arg);
  int ConnectNetwork(void);
  void StopNetwork();

  //irr::io::IrrXMLReader* xmlReader;
  //irr::io::StreamXMLReader* xmlReader;
  irr::io::StreamXMLReader<char, irr::io::IXMLBase>* xmlReader;

};
