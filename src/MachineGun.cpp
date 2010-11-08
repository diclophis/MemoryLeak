// Machine Gun Fountain


#include "MemoryLeak.h"
#include "Engine.h"
#include "MachineGun.h"


void MachineGun::build() {
	m_Life = 0.0;
	m_NumParticles = 5;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_Particles.push_back(new Model(m_Scene));
		reset_particle(idx);
		m_Particles[idx]->SetPosition((((float)idx) / (float)m_NumParticles) * (4.0) + m_Position[0], m_Position[1], m_Position[2]);
		m_Last = idx;
	}
}


void MachineGun::reset_particle(int idx) {
	m_Particles[idx]->SetPosition(m_Position[0], m_Position[1], m_Position[2]);
	m_Particles[idx]->SetRotation(0.0, 0.0, 0.0);
	m_Particles[idx]->SetVelocity(0.0, 0.0, 0.0);
	m_Particles[idx]->SetLife(0.0);
}


float MachineGun::Tick(float deltaTime) {
	m_Life += deltaTime;
	for (unsigned int i=0; i<m_NumParticles; i++) {
		m_Particles[i]->Tick(deltaTime);
		float d = m_Position[0] - m_Particles[i]->GetPosition()[0];
		m_Particles[i]->SetScale(d, d, d);
		if ((d * 0.25) > 1.0) {
			reset_particle(i);
		}
	}
	
	return m_Life;
}


void MachineGun::render() {
	for (unsigned int i=0; i<m_NumParticles; i++) {
		m_Particles[i]->render(0);
	}
}
