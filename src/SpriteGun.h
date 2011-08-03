// Machine Gun Particle Effect


class SpriteGun : public AtlasSprite {

public:

  ~SpriteGun();
	SpriteGun(GLuint t, int spr, int rows, int s, int e, float m, const std::string &str2, int s2, int e2, float m2, float w, float h);

	void Build(int n);
	void ResetParticle(int idx);
	void ShootParticle(int idx);
	void Simulate(float deltaTime);
	void Render();
	void Reset();
	void Fire();
	
	unsigned int m_NumParticles;
	std::vector<AtlasSprite *> m_AtlasSprites;	
	float m_ShootInterval;
	std::string m_ShotAnimation;
	int m_ShotStart;
	int m_ShotEnd;
	int m_FrameCounter;
	float m_ShotMaxLife;
	
	float *m_EmitVelocity;
	void SetEmitVelocity(float x, float y) {
		m_EmitVelocity[0] = x;
		m_EmitVelocity[1] = y;
	}
	
	float m_TimeSinceLastShot;
	
	bool m_IsReady;
  int m_IsFlags;
	
};
