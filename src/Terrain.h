// Jon Bardin GPL



class Terrain {


public:
	Terrain(b2World *w, GLuint t);
	~Terrain();

  int nHillKeyPoints;
  int fromKeyPointI;
  int toKeyPointI;

  MLPoint *hillKeyPoints;
  MLPoint *hillVertices;
  MLPoint *hillTexCoords;
  MLPoint *borderVertices;
  GLshort *hillElements;

  int nHillVertices;
  int nBorderVertices;
  b2World *world;
  b2Body *body;
  int screenW;
  int screenH;
  int textureSize;

  void Reset();
  void GenerateHillKeyPoints();
  void GenerateBorderVertices();
  void CreateBox2DBody();
  void ResetHillVertices();
  void SetOffsetX(float x);

  float offsetX;
  MLPoint position;

  void Render(StateFoo *sf);

  int m_TextureIndex;

  GLuint GenerateStripesTexture();
  ccColor4F GenerateColor();

  std::vector<GLuint> m_Textures;
  
  bool firstTime;

  RenderTexture *rt;

  int prevFromKeyPointI;
  int prevToKeyPointI;

  GLuint m_InterleavedBuffer;
  GLuint m_ElementBuffer;
  GLuint m_TextureInterlacedBuffer;
  GLuint m_TextureElementBuffer;

#ifdef USE_GLES2

  char *vertexInfoLog;
  char *fragmentInfoLog;
  char *shaderProgramInfoLog;
  GLuint vertexshader;
  GLuint fragmentshader;
  GLuint shaderprogram;
  char msg[512];

#endif

};
