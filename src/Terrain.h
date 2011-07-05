// Jon Bardin GPL


#define kMaxHillKeyPoints 101
#define kMaxHillVertices 1000
#define kMaxBorderVertices 5000
#define kHillSegmentWidth 15

class Terrain {


public:
	Terrain(b2World *w, GLuint t);
	~Terrain();

  MLPoint hillKeyPoints[kMaxHillKeyPoints];
  int nHillKeyPoints;
  int fromKeyPointI;
  int toKeyPointI;
  MLPoint hillVertices[kMaxHillVertices];
  MLPoint hillTexCoords[kMaxHillVertices];
  int nHillVertices;
  MLPoint borderVertices[kMaxBorderVertices];
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
  
};
