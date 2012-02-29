// Jon Bardin GPL

class SuperStarShooter : public Engine {

public:

	SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~SuperStarShooter();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateFoos();
  void DestroyFoos();

  void CreateCollider(float x, float y, float r, int f);
  void IndexToXY(int index, int *x, int *y);
  int XYToIndex(int x, int y);
  void BlitIntoSpace(int layer, int bottom_left_start, int width, int height, int offset_x, int offset_y);

	Octree<int> *m_Space;

  int m_GridStartIndex;
  int m_GridStopIndex;
  int m_SecondGridStartIndex;
  int m_SecondGridStopIndex;

  int m_GridCount;
  int *m_GridPositions;

  float m_CameraOffsetX;
  float m_CameraOffsetY;

  float m_CameraStopOffsetX;
  float m_CameraStopOffsetY;

  float m_CameraActualOffsetX;
  float m_CameraActualOffsetY;

  float m_LastCenterX;
  float m_LastCenterY;

  float m_PercentThere;

  float m_WarpTimeout;

  int m_PlayerIndex;
  int m_PlayerStartIndex;
  int m_PlayerStopIndex;

  int m_HoleIndex;

  float m_TouchStartX;
  float m_TouchStartY;

  bool m_PlayerCanMove;

  foofoo *m_GridFoo;
  foofoo *m_BatchFoo;
  foofoo **m_PlayerFoos;
  foofoo *m_HoleFoo;

};
