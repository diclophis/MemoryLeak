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

	std::string m_Animation;
	
	GLuint m_Texture;
  int m_Fps;
	int m_SpritesPerRow;
	int m_Rows;
	int m_Frame;
	int m_Start;
	int m_End;
	float m_AnimationSpeed;
	Sprite *m_Sprites;
	unsigned int m_AnimationLength;
	float m_AnimationDuration;
	float m_AnimationLife;
	float m_MaxLife;
	float m_Rotation;
  float m_LastRotation;
  GLshort *vertices;
  GLfloat *texture;
  GLushort *indices;

	
	void SetFrame(int f) {
		m_Frame = f;
	};
	
  ~AtlasSprite();
	AtlasSprite(GLuint t, int spr, int rows, int s = 0, int e = 0, float m = 1.0, float w = 50.0, float h = 50.0);
	void Render();
	
	unsigned int m_Count;
		
	void SetPosition(float x,float y) {
		m_Position[0] = x;
		m_Position[1] = y;
	}
	
	static void ReleaseBuffers();

	
	float *m_Position;
	float *m_Velocity;
	int *m_Frames;

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
	
	float *m_Scale;

  void SetScale(float, float);
  static void Scrub();

  GLuint m_IndexBuffer;
	foofoo *m_FooFoo;

};
