// Machine Gun Fountain


#include "MemoryLeak.h"
#include "AtlasSprite.h"
#include "Model.h"
#include "Engine.h"
#include "SpriteGun.h"


void SpriteGun::Build() {
	m_NumParticles = 1000;
	m_ShootInterval = 0.01;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_AtlasSprites.push_back(new AtlasSprite(m_Texture, m_SpritesPerRow, m_Rows));
		ResetParticle(idx);
	}
}


void SpriteGun::ResetParticle(int idx) {
	//LOGV("a %f %f\n", m_Position[0], m_AtlasSprites[idx]->m_Position[0]); 
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	//LOGV("b %f %f\n", m_Position[0], m_AtlasSprites[idx]->m_Position[0]); 
	m_AtlasSprites[idx]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[idx]->m_IsAlive = false;
}


void SpriteGun::ShootParticle(int idx) {
	m_AtlasSprites[idx]->SetLife(0.0);
	//LOGV("1 %f %f\n", m_Position[0], m_AtlasSprites[idx]->m_Position[0]); 
	m_AtlasSprites[idx]->SetPosition(m_Position[0], m_Position[1]);
	//LOGV("2 %f %f\n", m_Position[0], m_AtlasSprites[idx]->m_Position[0]); 

	
	//m_AtlasSprites[idx]->SetVelocity(0.0, 700.0);
	
	//m_AtlasSprites[idx]->m_Velocity[0] = ((randf() * 100.0) - 25.0);
	//m_AtlasSprites[idx]->m_Velocity[1] = ((randf() * 100.0) - 25.0);
	
	m_AtlasSprites[idx]->m_Velocity[0] = fastSinf(randf() * 10.0) * 10.0;
	m_AtlasSprites[idx]->m_Velocity[1] = fastSinf(randf() * 10.0) * 10.0;
	
	/*
	if (m_AtlasSprites[idx]->m_Velocity[0] * m_AtlasSprites[idx]->m_Velocity[0] + m_AtlasSprites[idx]->m_Velocity[1] * m_AtlasSprites[idx]->m_Velocity[1] > 700.0 * 700.0) {
		m_AtlasSprites[idx]->m_Velocity[0] *= 0.7;
		m_AtlasSprites[idx]->m_Velocity[1] *= 0.7;
	}
	*/
	
	m_AtlasSprites[idx]->m_IsAlive = true;
}


void SpriteGun::Simulate(float deltaTime) {
	bool shot = false;
	for (unsigned int i=0; i<m_NumParticles; i++) {
		if (!shot && (!m_AtlasSprites[i]->m_IsAlive) && (m_Life > m_ShootInterval)) {
			ShootParticle(i);
			m_Life = 0.0;
			shot = true;
		}
		
		if (m_AtlasSprites[i]->m_Life > 5.0) {
			ResetParticle(i);
		}
		
		m_AtlasSprites[i]->Simulate(deltaTime);
	}
	m_Life += deltaTime;
}


void SpriteGun::Render() {
	for (unsigned int i=0; i<m_NumParticles; i++) {
		if (m_AtlasSprites[i]->m_IsAlive) {
			m_AtlasSprites[i]->Render();
		}
	}
}