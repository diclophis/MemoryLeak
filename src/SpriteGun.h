// Machine Gun Particle Effect


class SpriteGun : public AtlasSprite {

public:

	SpriteGun(GLuint t, int spr, int rows, const std::string &str, int s = 0, int e = 0) : AtlasSprite(t, spr, rows, str, s, e) {
		Build();
	};
	
	void Build();
	void ResetParticle(int idx);
	void ShootParticle(int idx);
	void Simulate(float deltaTime);
	void Render();
	
	int m_NumParticles;
	std::vector<AtlasSprite *> m_AtlasSprites;
	
	float m_ShootInterval;
};