// Jon Bardin GPL


#include "MemoryLeak.h"


SpriteGun::~SpriteGun() {
  delete m_EmitVelocity;
  for (std::vector<AtlasSprite *>::iterator i = m_AtlasSprites.begin(); i != m_AtlasSprites.end(); ++i) {
    delete *i;
  }
  m_AtlasSprites.clear();
}


SpriteGun::SpriteGun(foofoo *first_ff, foofoo *second_ff) : AtlasSprite(first_ff) {
  m_IsAlive =false;
  m_IsFlags = 0;
  m_ShotFooFoo = second_ff;
  m_IsReady = false;
  m_RenderBullets = true;
  m_EmitVelocity = new float[2];
  m_EmitVelocity[0] = 0;
  m_EmitVelocity[1] = 0;
	m_TimeSinceLastShot = 0.0;
};

void SpriteGun::Build(int n) {
	m_NumParticles = n;
	m_ShootInterval = 1.0;
	m_FrameCounter = 0;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_AtlasSprites.push_back(new AtlasSprite(m_ShotFooFoo));
		ResetParticle(idx);
	}
}


void SpriteGun::Reset() {
  m_IsAlive = false;
  m_Frame = 0;
  m_Life = 0.0;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		ResetParticle(idx);
	}
}


void SpriteGun::ResetParticle(int idx) {
	m_AtlasSprites[idx]->SetLife(0.0);
	m_AtlasSprites[idx]->m_AnimationLife = 0.0;
	m_AtlasSprites[idx]->m_Fps = m_Fps;
	m_AtlasSprites[idx]->m_Frame = 0;
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	m_AtlasSprites[idx]->SetScale(m_Scale[0], m_Scale[1]);
	m_AtlasSprites[idx]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[idx]->m_IsAlive = false;
}


void SpriteGun::ShootParticle(int idx) {
	m_TimeSinceLastShot = 0.0;
	m_AtlasSprites[idx]->SetLife(0.0);
	m_AtlasSprites[idx]->SetVelocity(m_EmitVelocity[0], m_EmitVelocity[1]);
	m_AtlasSprites[idx]->m_IsAlive = true;
}


void SpriteGun::Fire() {
  m_IsAlive = true;
  int shot = 0;
  for (unsigned int i=0; i<m_NumParticles; i++) {
    if (shot == 0 && (m_TimeSinceLastShot > (1.0 / 20.0) && !m_AtlasSprites[i]->m_IsAlive)) {
      ResetParticle(i);
      ShootParticle(i);
      shot++;
    }
  }
}


void SpriteGun::Simulate(float deltaTime) {	
  m_TimeSinceLastShot += deltaTime;
  for (unsigned int i=0; i<m_NumParticles; i++) {
    m_AtlasSprites[i]->m_Rotation = m_Rotation;
    if ((m_AtlasSprites[i]->m_Life > m_ShotFooFoo->m_AnimationDuration)) {
      ResetParticle(i);
    }
    
    m_AtlasSprites[i]->Simulate(deltaTime);
    //LOGV("m_Frame: %d\n", m_AtlasSprites[i]->m_Frame);
  }
  AtlasSprite::Simulate(deltaTime);
}


bool MyDataSortPredicate(const AtlasSprite* d1, const AtlasSprite* d2)
{
  return d1->m_Life > d2->m_Life;
}

void SpriteGun::Render(StateFoo *sf, foofoo *batch_foo) {
  /*
  if (false) { //TODO: reverse render
    int i=(m_NumParticles);
    while (i-- > 0) {
      if (m_AtlasSprites[i]->m_IsAlive && m_RenderBullets) {
        m_AtlasSprites[i]->Render(sf, batch_foo);
      }
    }
  } else {
    unsigned int i=0;
    while (i < (m_NumParticles)) {
      if (m_AtlasSprites[i]->m_IsAlive && m_RenderBullets) {
        m_AtlasSprites[i]->Render(sf, batch_foo);
      }
      i++;
    }
  }
  */

  std::sort(m_AtlasSprites.begin(), m_AtlasSprites.end(), MyDataSortPredicate);

  for (std::vector<AtlasSprite *>::const_iterator citer = m_AtlasSprites.begin(); citer != m_AtlasSprites.end(); ++citer) {
    AtlasSprite *c = *citer;
    if (c->m_IsAlive && m_RenderBullets) {
      c->Render(sf, batch_foo);
    }
  }

  AtlasSprite::Render(sf, batch_foo);
}


void SpriteGun::ResetFoo(foofoo *ff, foofoo *sff) {
  m_FooFoo = ff;
  m_ShotFooFoo = sff;
  for (unsigned int i=0; i<m_NumParticles; i++) {
    m_AtlasSprites[i]->m_FooFoo = m_ShotFooFoo;
  }
}
