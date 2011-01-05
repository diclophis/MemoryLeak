// Machine Gun Fountain


#include "MemoryLeak.h"
#include "AtlasSprite.h"
#include "Model.h"
#include "Engine.h"
#include "SpriteGun.h"


void SpriteGun::Build(int n) {
	m_NumParticles = n;
	m_ShootInterval = 1.0;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_AtlasSprites.push_back(new AtlasSprite(m_Texture, m_SpritesPerRow, m_Rows, m_ShotAnimation, m_ShotStart, m_ShotEnd, m_ShotMaxLife));
		ResetParticle(idx);
	}
}


void SpriteGun::Reset() {
	m_Life = 0.0;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		ResetParticle(idx);
	}
}


void SpriteGun::ResetParticle(int idx) {
	m_AtlasSprites[idx]->SetLife(0.0);
	m_AtlasSprites[idx]->m_Frame = 0;
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	m_AtlasSprites[idx]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[idx]->m_IsAlive = false;
}


void SpriteGun::ShootParticle(int idx) {
	m_AtlasSprites[idx]->SetLife(0.0);
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	m_AtlasSprites[idx]->m_Velocity[0] = ((randf() - 0.0) * 500.0); //fastSinf(randf() * 2.0) * 500.0;
	m_AtlasSprites[idx]->m_Velocity[1] = +2000.0 + (randf() * 500.0); //((randf() - 0.0) * 500.0); //fastSinf(randf() * 2.0) * 500.0;
	m_AtlasSprites[idx]->m_IsAlive = true;
}


void SpriteGun::Simulate(float deltaTime) {	
	if (m_IsAlive) {
		int shot_this_tick = 0;
		int not_shot_this_tick = 0;
		for (unsigned int i=0; i<m_NumParticles; i++) {
			if ((shot_this_tick < (randf() * 2.0)) && ((m_AtlasSprites[i]->m_Life > m_MaxLife) || !m_AtlasSprites[i]->m_IsAlive)) {
				ShootParticle(i);
				shot_this_tick++;
			} else {
				not_shot_this_tick++;
				if ((m_AtlasSprites[i]->m_Life > m_ShotMaxLife)) {
					ResetParticle(i);
				}
			}
			m_AtlasSprites[i]->Simulate(deltaTime);
		}
	}
	AtlasSprite::Simulate(deltaTime);
}


void SpriteGun::Render() {
	if (m_IsAlive) {
		if (true) { //TODO: reverse render
			int i=(m_NumParticles);
			while (i-- > 0) {
				if (m_AtlasSprites[i]->m_IsAlive) {
					m_AtlasSprites[i]->Render();
				}
			}
		} else {
			int i=0;
			while (i++ < (m_NumParticles - 1)) {
				if (m_AtlasSprites[i]->m_IsAlive) {
					m_AtlasSprites[i]->Render();
				}
			}
		}
	} else {
		AtlasSprite::Render();
	}
}