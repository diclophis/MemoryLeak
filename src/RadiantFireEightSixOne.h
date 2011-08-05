// Jon Bardin GPL

class RadiantFireEightSixOne : public Engine {

public:

	RadiantFireEightSixOne(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~RadiantFireEightSixOne();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();


  b2World *world;
  Terrain *terrain;
  Hero *hero;
  //GLESDebugDraw *render;

  void CreateBox2DWorld();

  bool m_Touched;
  bool m_RequestedFullscreen;

};
