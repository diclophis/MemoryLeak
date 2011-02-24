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
	void RenderModelPhase();
	void RenderSpritePhase();
	void DrawWater();
	void DrawRipples();
	void DrawPlayer(float yScale);
	void BuildParticles(int n);
	void ResetParticles();
	void ResetParticle(int idx);
	void ShootParticle(int idx);
	unsigned int m_NumParticles;
	int m_ParticlesOffset;
	int m_ParticleStreamIndex;
	std::vector<Model *> m_ModelParticles;	
	float m_ShootInterval;
	
	
	
	
	
	
	
	
	
	
	


	Octree<int> *m_Space;

	
	
	
	
	
	
	
	
};
