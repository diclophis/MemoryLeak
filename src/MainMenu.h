// Jon Bardin GPL

class MainMenu : public Engine {

public:

	MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~MainMenu();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateFoos();
  void DestroyFoos();
  bool m_RequestedFullscreen;
  float m_CameraX;
  float m_CameraY;
  float m_CameraZ;
  float m_CameraR;
  foofoo *m_BatchFoo;
  bool m_TouchingLeft;
  bool m_TouchingRight;
  int m_CurrentTempo;

};
