// AtlasSprite

class AtlasSprite {
	
public:

	typedef struct
	{
		union {
			float dx;
			float width;
		};
		union {
			float dy;
			float height;
		};
		float tx1, ty1;
		float tx2, ty2;
	} Sprite;
	
	GLuint m_Texture;
	int m_SpritesPerRow;
	int m_Rows;
	int m_Frame;
	int m_Start;
	int m_End;
	float m_AnimationSpeed;
	Sprite *m_Sprites;
	int m_AnimationLength;
	float m_AnimationDuration;
	float m_MaxLife;
	//char *m_Animation;
	//char m_Animation[1024];
	std::string m_Animation;
	
	void SetFrame(int f) {
		m_Frame = f;
	};
	
	//void SetAnimation(std::string a);

	
	AtlasSprite(GLuint t, int spr, int rows, const std::string &str, int s = 0, int e = 0, float m = 1.0);
	void Render();
	
	int m_Count;
	
	
	//float *m_Position;
	void SetPosition(float x,float y) {
		m_Position[0] = x;
		m_Position[1] = y;
	}
	
	float *m_Position;
	float *m_Velocity;
	int *m_Frames;
	
	//float m_Position[2];
	//float m_Velocity[2];
	//std::vector<float>m_Position;
	//std::vector<float>m_Velocity;

	void SetVelocity(float x, float y) {
		m_Velocity[0] = x;
		m_Velocity[1] = y;
	}
	
	float m_Life;
	void SetLife(float life) {
		m_Life = life;
	}
	
	bool m_IsAlive;
	void Simulate(float deltaTime);
};