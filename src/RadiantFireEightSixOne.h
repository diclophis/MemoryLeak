// Jon Bardin GPL

class RadiantFireEightSixOne : public Engine {

public:

	RadiantFireEightSixOne(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s);
	~RadiantFireEightSixOne();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateFoos();
  void DestroyFoos();
  void CreateBox2DWorld();
  void PlayerSleep();
  void PlayerWake();
  void PlayerDive();
  void PlayerLimitVelocity();
  void PlayerUpdateNodePosition();
  void PlayerReset();
  void PlayerCreateBox2DBody();

  b2World *m_World;
  b2Body *m_PlayerBody;

  Terrain *m_Terrain;

  bool m_Touched;
  bool m_RequestedFullscreen;

  float m_PlayerRadius;
  bool m_PlayerIsAwake;
  float m_PlayerRotation;
  MLPoint m_PlayerPosition;

  int m_PlayerIndex;

  foofoo *m_PlayerFoo;
  foofoo *m_BatchFoo;

};
