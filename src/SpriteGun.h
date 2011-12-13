// Machine Gun Particle Effect


class SpriteGun : public AtlasSprite {

public:

  ~SpriteGun();
	SpriteGun(foofoo *first_ff, foofoo *second_ff);

	void Build(int n);
	void ResetParticle(int idx);
	void ShootParticle(int idx);
	void Simulate(float deltaTime);
	void Render(StateFoo *sf, foofoo *batch_foo = NULL);
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
  bool m_RenderBullets;
  int m_IsFlags;

  foofoo *m_ShotFooFoo;
	
};
