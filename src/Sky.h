// Jon Bardin GPL


class Sky {


public:
	Sky(int ts);
	~Sky();

  AtlasSprite *sprite;
  float offsetX;
  float scale;
  int textureSize;
  int screenW;
  int screenH;
};
