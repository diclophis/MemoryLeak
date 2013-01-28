// Jon Bardin GPL

class SuperStarShooter : public Engine, public micropather::Graph, public MazeNetworkDelegate {

public:

	SuperStarShooter(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s);
	~SuperStarShooter();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateFoos();
  void DestroyFoos();

  void BlitIntoSpace(int layer, int bottom_left_start, int width, int height, int offset_x, int offset_y);

	float LeastCostEstimate(void* nodeStart, void* nodeEnd);
	void AdjacentCost(void* node, std::vector<micropather::StateCost> *neighbors);
	void PrintStateInfo(void* node) {};

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

  float m_WarpTimeout;

  int m_PlayerIndex;
  int m_PlayerStartIndex;
  int m_PlayerStopIndex;

  unsigned int m_TrailCount;
  int m_TrailStartIndex;
  int m_TrailStopIndex;

  float m_TouchStartX;
  float m_TouchStartY;

  foofoo *m_GridFoo;
  std::vector<foofoo *>m_PlayerFoos;
  foofoo *m_TrailFoo;

	micropather::MicroPather *m_Pather;
	std::vector<void *> *m_Steps;

  int m_MaxStatePointers;
  int m_StatePointer;
  std::vector<nodexyz *>m_States;

  int m_TargetX;
  int m_TargetY;
 
  int StatePointerFor(int x, int y, int z);

  bool m_TargetIsDirty;

  float m_GotLastSwipeAt;
  bool m_SwipedBeforeUp;
  bool m_StartedSwipe;

  int m_CenterOfWorldX;
  int m_CenterOfWorldY;

  void LoadMaze(int level_index);

  void BlitMazeCell(int row, int col, int mask);

  bool m_NeedsTerrainRebatched;

  int GRID_X;
  int GRID_Y;

  bool Passable(int index);

  float m_SelectTimeout;

  MazeNetwork *m_Network;
  //bool UpdatePlayerAtIndex(int i, int online, float x, float y);
  bool RequestRegistration(int i);
  //  return true;
  //};
  float m_NetworkTickTimeout;
};
