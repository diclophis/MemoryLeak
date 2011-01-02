// Machine Gun Fountain


#include "MemoryLeak.h"
#include "AtlasSprite.h"
#include "Model.h"
#include "Engine.h"
#include "SpriteGun.h"


void SpriteGun::Build() {
	m_NumParticles = 150; //225;
	m_ShootInterval = 1.0;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_AtlasSprites.push_back(new AtlasSprite(m_Texture, m_SpritesPerRow, m_Rows, m_Animation, m_Start, m_End));
		ResetParticle(idx);
	}
	m_IsAlive = true;
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
	
	m_AtlasSprites[idx]->m_Velocity[0] = (randf() - 0.0) * 50.0; //fastSinf(randf() * 2.0) * 500.0;
	m_AtlasSprites[idx]->m_Velocity[1] = (randf() - 0.0) * 50.0; //fastSinf(randf() * 2.0) * 500.0;

	//m_AtlasSprites[idx]->m_Velocity[1] = fastSinf(randf() * 10.0) * 500.0;
	
	//m_AtlasSprites[idx]->m_Velocity[1] = (randf() * 10.0) + 400.0;

	/*
	if (m_AtlasSprites[idx]->m_Velocity[0] * m_AtlasSprites[idx]->m_Velocity[0] + m_AtlasSprites[idx]->m_Velocity[1] * m_AtlasSprites[idx]->m_Velocity[1] > 700.0 * 700.0) {
		m_AtlasSprites[idx]->m_Velocity[0] *= 0.7;
		m_AtlasSprites[idx]->m_Velocity[1] *= 0.7;
	}
	*/
	
	m_AtlasSprites[idx]->m_IsAlive = true;
}


void SpriteGun::Simulate(float deltaTime) {
	if (m_IsAlive) {
		//m_NumParticles = ((int)(m_Life * 10) % 300);
		bool shot = false;
		int shot_this_tick = 0;
		int not_shot_this_tick = 0;
		for (unsigned int i=0; i<m_NumParticles; i++) {
			//if (!shot && (!m_AtlasSprites[i]->m_IsAlive) && (m_Life > m_ShootInterval)) {
			if ((shot_this_tick < 2) && (!m_AtlasSprites[i]->m_IsAlive)) {
				ShootParticle(i);
				shot_this_tick++;
				//m_Life = 0.0;
				//shot = true;
			} else {
				not_shot_this_tick++;
			}
			
			if (m_AtlasSprites[i]->m_Life > m_AtlasSprites[i]->m_MaxLife) {
				ResetParticle(i);
			}
			
			m_AtlasSprites[i]->Simulate(deltaTime);
		}
		//if (shot_this_tick == 0) {
		//	LOGV("shot/not %d/%d\n", shot_this_tick, not_shot_this_tick);
		//}
	}
	AtlasSprite::Simulate(deltaTime);
}


void SpriteGun::Render() {
	if (m_IsAlive) {
		if (true) { //
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
		m_AtlasSprites[0]->Render();
	}
}