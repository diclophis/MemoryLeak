// Jon Bardin GPL

class SuperBarrelBlast : public Engine {

public:

	SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~SuperBarrelBlast();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateCollider(float x, float y, float r, int f);
	
	Octree<int> *m_Space;

  int m_SpriteCount;

  int m_BarrelStartIndex;
  int m_BarrelStopIndex;
  int m_BarrelCount;

  int m_DebugBoxesStartIndex;
  int m_DebugBoxesStopIndex;
  int m_DebugBoxesCount;

  int m_CurrentBarrelIndex;
  int m_LastShotBarrelIndex;

	float m_CameraOffsetX;
	float m_CameraOffsetY;

  float m_LaunchTimeout;
  float m_ReloadTimeout;
  float m_SwipeTimeout;
  float m_RotateTimeout;
  float m_MirrorRotateTimeout;

  float m_PlayerLastX;
  float m_PlayerLastY;

  int m_LastCollideIndex;
  int m_LastFailedCollideIndex;

  float m_Gravity;

  int m_LastTouchedIndex;

  bool m_DidDrag;

  void IndexToXY(int index, int *x, int *y);
  int XYToIndex(int x, int y);

  float m_CameraPanX;
  float m_CameraPanY;

  float m_PanStartX;
  float m_PanStartY;
  float m_PanSpeedX;
  float m_PanSpeedY;

  float m_CollideTimeout;

  float m_TouchTimeout;

  int m_SoundOffset;

  int *m_DebugBoxesPositions;

  float m_Rotation;

};
