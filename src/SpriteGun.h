// Machine Gun Particle Effect


class SpriteGun : public AtlasSprite {

public:

	SpriteGun(GLuint t, int spr, int rows) : AtlasSprite(t, spr, rows) {
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