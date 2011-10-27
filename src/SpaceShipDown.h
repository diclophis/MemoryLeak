// Jon Bardin GPL

class SpaceShipDown : public Engine {

public:

	SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~SpaceShipDown();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateBox2DWorld();
  b2World *world;
  //Terrain *terrain;
  //Hero *hero;
  int m_LandscapeIndex;
  int m_PlayerIndex;
  b2Body *m_PlayerBody;

  bool m_TouchedLeft;
  bool m_TouchedRight;
};
