// Machine Gun Fountain


#include "MemoryLeak.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "Engine.h"


void SpriteGun::Build(int n) {

	m_NumParticles = n;
	m_ShootInterval = 1.0;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_AtlasSprites.push_back(new AtlasSprite(m_Texture, m_SpritesPerRow, m_Rows, m_ShotStart, m_ShotEnd, m_ShotMaxLife));
		ResetParticle(idx);
	}
}


void SpriteGun::Reset() {
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		ResetParticle(idx);
	}
}


void SpriteGun::ResetParticle(int idx) {
	m_AtlasSprites[idx]->SetLife(0.0 - (randf() * 20));
	m_AtlasSprites[idx]->SetScale(1.0, 1.0);
	m_AtlasSprites[idx]->m_Frame = 0;
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	m_AtlasSprites[idx]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[idx]->m_IsAlive = false;
}


void SpriteGun::ShootParticle(int idx) {
	m_TimeSinceLastShot = 0.0;
	m_AtlasSprites[idx]->SetLife(0.0);
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	m_AtlasSprites[idx]->SetVelocity(m_EmitVelocity[0], m_EmitVelocity[1]);
	m_AtlasSprites[idx]->m_IsAlive = true;
}


void SpriteGun::Simulate(float deltaTime) {	
	if (m_IsAlive) {
		m_TimeSinceLastShot += deltaTime;
		int shot_this_tick = 0;
		int not_shot_this_tick = 0;
		for (unsigned int i=0; i<m_NumParticles; i++) {
			//(shot_this_tick < (randf() * 2.0)) && 
			if ((shot_this_tick < 1) && ((m_AtlasSprites[i]->m_Life > m_MaxLife) || !m_AtlasSprites[i]->m_IsAlive) && m_TimeSinceLastShot > 0.1) {
				//LOGV("shoot %d\n", i);
				ShootParticle(i);
				shot_this_tick++;
			} else {
				not_shot_this_tick++;
				if ((m_AtlasSprites[i]->m_Life > m_ShotMaxLife)) {
					ResetParticle(i);
					//LOGV("reset %d\n", i);
				}
				
				//m_AtlasSprites[i]->m_Velocity[1] -= 60.0;
				/*
				m_AtlasSprites[i]->m_Scale[0] -= 0.01;
				m_AtlasSprites[i]->m_Scale[1] -= 0.01;
				if (m_AtlasSprites[i]->m_Scale[0] < 0.25) {
					m_AtlasSprites[i]->m_Scale[0] = 0.25;
					m_AtlasSprites[i]->m_Scale[1] = 0.25;
				}
				*/
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
		AtlasSprite::Render();
	} else {
		AtlasSprite::Render();
	}
}
