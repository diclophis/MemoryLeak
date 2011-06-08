// Jon Bardin GPL

class FlightControl : public Engine {

public:

	FlightControl(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~FlightControl();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateCollider(float x, float y, float r, int f);
	
	Octree<int> *m_Space;

  float m_Gravity;

  float m_PadCenters[2][2];

  int m_LastPadTouched;

  float m_LastTouchX;
  float m_LastTouchY;

  float m_CameraX;
  float m_CameraY;
  float m_CameraZ;
  float m_CameraR;

  float m_OpenFeintTimeout;

};
