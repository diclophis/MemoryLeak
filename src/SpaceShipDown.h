// Jon Bardin GPL

class SpaceShipDown : public Engine {

public:

	SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~SpaceShipDown();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateWorld();
  void CreatePlayer();
  void CreateSpaceShipPart(float x, float y);
  void CreatePlatform(float x, float y, float w, float h);

  b2World *world;
  b2Body *m_PlayerBody;

  int m_LandscapeIndex;
  int m_PlayerIndex;
  int m_SpaceShipPartsStartIndex;
  int m_SpaceShipPartsStopIndex;
  int m_PickedUpPartIndex;

  bool m_TouchedLeft;
  bool m_TouchedRight;

  GLESDebugDraw *m_DebugDraw;

  SpaceShipDownContactListener *m_ContactListener;
  

};
