//

class PixelPusher : public Engine {
	
public:

	PixelPusher(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l);
	~PixelPusher();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void Load(int level_index);
	const char *byte_to_binary(int x);
	int m_PlayerIndex;
	int m_TerrainStartIndex;
	int m_TerrainEndIndex;
	float m_CameraRotation;
	float m_CameraHeight;
	float *m_Touches;
	float m_CameraRotationSpeed;
	float m_CameraClimbSpeed;

};
