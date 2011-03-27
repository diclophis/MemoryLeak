// Jon Bardin GPL

class SuperBarrelBlast : public Engine {

public:

	SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd);
	~SuperBarrelBlast();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
	
	Octree<int> *m_Space;
	
	float m_CameraOffsetX;
};
