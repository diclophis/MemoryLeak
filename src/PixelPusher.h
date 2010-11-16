//

class PixelPusher : public Engine {
	
public:

	PixelPusher(int width, int height, std::vector<GLuint> &x_textures, std::vector<foo*> &x_models);
	~PixelPusher();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();

	int m_PlayerIndex;
	int m_EnemyStartIndex;
	int m_EnemyEndIndex;
	int m_TerrainStartIndex;
	int m_TerrainEndIndex;
	int m_SwipeState;
	float m_CameraRotation;
	float *m_Touches;
	float m_CameraRotationSpeed;
};
