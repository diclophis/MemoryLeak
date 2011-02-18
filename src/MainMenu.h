// Jon Bardin GPL

class MainMenu : public Engine {

public:

	float m_CameraRotation;
	float m_CameraHeight;
	float *m_Touches;
	float m_CameraRotationSpeed;
	float m_CameraClimbSpeed;
	
	float leftSliderValue;
	float rightSliderValue;
	
	int m_CameraIndex;
	
	MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd);
	~MainMenu();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void Render();

};
