class AtlasSprite {
	public:

	
	typedef struct
	{
		union {
			float dx;
			int width;
		};
		union {
			float dy;
			int height;
		};
		float tx1, ty1;
		float tx2, ty2;
	} Sprite;
	
	
	GLuint m_Texture;
	int m_TextureWidth;
	int m_TextureHeight;
	int m_SpriteWidth;
	int m_SpriteHeight;
	int m_SpriteIndexStart;
	int m_SpriteIndexEnd;
	int m_SpritesPerRow;
	int m_Rows;
	int m_Frame;
	Sprite *m_Sprites;
	char m_Animation[1024];

	void SetFrame(int f) {
		m_Frame = f;
	};
	
	void SetAnimation(const char *a, int i) {
		snprintf(m_Animation, 1024, a, i);
		//memcpy(m_Animation, a, sizeof(char) * 1024);
	}
	
	AtlasSprite(GLuint t, int tw, int th, int sw, int sh, int s, int e, int spr, int rows);
	void Render();
	
	int m_Count;
};