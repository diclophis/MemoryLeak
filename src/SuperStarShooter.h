// Jon Bardin GPL

class SuperStarShooter : public Engine {

public:

	SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~SuperStarShooter();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateCollider(float x, float y, float r, int f);
  void IndexToXY(int index, int *x, int *y);
  int XYToIndex(int x, int y);

	Octree<int> *m_Space;

  int m_GridStartIndex;
  int m_GridStopIndex;

  int m_GridCount;
  int *m_GridPositions;

  float m_CameraOffsetX;
  float m_CameraOffsetY;

  float m_CameraStopOffsetX;
  float m_CameraStopOffsetY;

  float m_CameraActualOffsetX;
  float m_CameraActualOffsetY;

  float m_PercentThere;

  float m_WarpTimeout;

};
