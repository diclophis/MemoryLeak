// Jon Bardin GPL

class BlockBuster : public Engine {

public:


	float m_LastFloorX;
	float m_LastFloorY;
	
	BlockBuster(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd);
	~BlockBuster();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
	void SweepFloorAt(int i);
	
	int m_FloorBufferStartIndex;
	int m_FloorBufferCount;
	
	Octree<int> *m_Space;
	
	float m_FloorSize;
	bool m_PlayerIsFalling;
	
	int *colliding_indexes;
	
	float m_CameraOffsetX;
	float m_CameraSpeed;
	
	int m_LastSwept;
	float m_SweepTimeout;
	
};
