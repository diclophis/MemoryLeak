// Machine Gun Particle Effect


class SpriteGun : public AtlasSprite {

public:

	SpriteGun(GLuint t, int spr, int rows, const std::string &str, int s, int e, float m, const std::string &str2, int s2, int e2, float m2) : AtlasSprite(t, spr, rows, str, s, e, m) {
		m_ShotAnimation = str2;
		m_ShotStart = s2;
		m_ShotEnd = e2;
		m_ShotMaxLife = m2;
	};

	void Build(int n);
	void ResetParticle(int idx);
	void ShootParticle(int idx);
	void Simulate(float deltaTime);
	void Render();
	void Reset();
	
	int m_NumParticles;
	std::vector<AtlasSprite *> m_AtlasSprites;	
	float m_ShootInterval;
	std::string m_ShotAnimation;
	int m_ShotStart;
	int m_ShotEnd;
	float m_ShotMaxLife;
	
	float *m_EmitVelocity;
	void SetEmitVelocity(float x, float y) {
		m_EmitVelocity[0] = x;
		m_EmitVelocity[1] = y;
	}
	
	void Fire() {
		m_IsAlive = true;
	}
};