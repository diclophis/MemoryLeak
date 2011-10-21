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
  /*
  MLPoint hillKeyPoints[kMaxHillKeyPoints];
  MLPoint hillVertices[kMaxHillVertices];
  MLPoint hillTexCoords[kMaxHillVertices];
  MLPoint borderVertices[kMaxBorderVertices];
  */

  int nHillVertices;
  int nBorderVertices;
  SpriteGun *stripes;
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

  void Render();

  int m_TextureIndex;

  GLuint GenerateStripesTexture();
  ccColor4F GenerateColor();

  std::vector<GLuint> m_Textures;
  
  bool firstTime;

  RenderTexture *rt;

  int prevFromKeyPointI;
  int prevToKeyPointI;
};
