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
	Sprite *m_Sprites;
	char *m_Animation;

	void SetFrame(int f) {
		m_Frame = f;
	};
	
	void SetAnimation(const char *a, int i) {
		snprintf(m_Animation, 1024, a, i);
	}
	
	AtlasSprite(GLuint t, int spr, int rows);
	void Render();
	
	int m_Count;
		
	float *m_Position;
	void SetPosition(float x,float y) {
		m_Position[0] = x;
		m_Position[1] = y;
	}
	
	float *m_Velocity;
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