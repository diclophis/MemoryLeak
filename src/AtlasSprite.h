// AtlasSprite

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


class AtlasSprite {
	
public:


	std::string m_Animation;
	
	GLuint m_Texture;
  int m_Fps;
	int m_SpritesPerRow;
	int m_Rows;
	int m_Frame;
	int m_Start;
	int m_End;
	unsigned int m_AnimationLength;
	float m_AnimationDuration;
	float m_AnimationLife;
	float m_MaxLife;
	float m_Rotation;
  float m_LastRotation;

  float *m_TargetPosition;

  bool MoveToTargetPosition(float dt);

	void SetFrame(int f) {
		m_Frame = f;
	};
	
  ~AtlasSprite();
	AtlasSprite(foofoo *ff);
	void Render(StateFoo *sf, foofoo *batch_foo = NULL);
	static void RenderFoo(StateFoo *sf, foofoo *foo);
	
	unsigned int m_Count;
		
	void SetPosition(float x,float y);

	void SetScale(float x,float y) {
		m_Scale[0] = x;
		m_Scale[1] = y;
	}
	
	static void ReleaseBuffers();
	
	float *m_Position;
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
	
	float *m_Scale;

  GLuint m_IndexBuffer;
	foofoo *m_FooFoo;

  static foofoo *GetFoo(GLuint t, int spr, int rows, int s, int e, float m);
  static foofoo *GetBatchFoo(GLuint t, int m);

  bool m_IsNinePatch;
  void BlitVertice(foofoo *batch_foo, int i, float vx, float vy, float tx, float ty, float cos_r, float sin_r, bool use_r);

  AtlasSprite *m_Parent;

};
