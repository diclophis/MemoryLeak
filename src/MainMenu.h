// Jon Bardin GPL

class MainMenu : public Engine {

public:

	MainMenu(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s);
	~MainMenu();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateFoos();
  void DestroyFoos();
  foofoo *m_NinePatchFoo;
  foofoo *m_BatchFoo;
  float m_SwapTimeout;
};
