//

class PixelPusher : public Engine {
	
public:

	PixelPusher(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs);
	~PixelPusher();
	void Hit(float x, float y, int hitState);
	void Build();
	int Simulate();
	void Load(int level_index);
	const char *byte_to_binary(int x);
	void Render();
	
	int m_PlayerIndex;
	int m_TerrainStartIndex;
	int m_TerrainEndIndex;
	float m_CameraRotation;
	float m_CameraHeight;
	float *m_Touches;
	float m_CameraRotationSpeed;
	float m_CameraClimbSpeed;
	std::vector<int>m_SimulatedModels;
	Octree<int> *m_Space;
	Model *m_Menu;
	int m_TargetIndex;
	micropather::MicroPather *m_Pather;
	micropather::ModelOctree *m_ModelOctree;
	int m_CircleIndex;
	int m_AiIndex;
	int m_LastAiSolved;
	float m_LastPumpedComet;
	float m_PumpCometTimeout;
	float m_LastForcePumpedComet;
	float m_PumpCometForceTimeout;
	AtlasSprite *m_AtlasSprite;
	SpriteGun *m_SpriteGun;
	int m_NumComets;
	float m_CometStart;
	float m_CometStop;
	float m_CometDelta;
	std::vector<SpriteGun *> m_IceComets;
};