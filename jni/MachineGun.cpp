// Machine Gun Fountain


#include "MemoryLeak.h"
#include "Engine.h"
#include "MachineGun.h"


void MachineGun::build() {
	m_NumParticles = 16;
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		m_Particles.push_back(new Model(m_Scene));
		reset_particle(idx);
		m_Particles[idx]->SetLife(((float)idx / (float)m_NumParticles) * 0.075);
	}	
}


void MachineGun::reset_particle(int idx) {
	m_Particles[idx]->SetPosition(m_Position[0], m_Position[1], m_Position[2]);
	m_Particles[idx]->SetScale(0.25, 0.25, 0.25);
	m_Particles[idx]->SetRotation(0.0, 0.0, 0.0);
	m_Particles[idx]->SetLife(0.0);
	
	float q = 0.0; //(sfrand() - 0.5) * 0.01;
	float r = (sfrand() - 0.5) * 0.015;
	float s = (sfrand() - 0.5) * 0.015;
	//(m_Velocity[0] * 2.0) + 0.08 + fabs(q)
	m_Particles[idx]->SetVelocity(q, 0.0 + r, 0.0 + s);
}


void MachineGun::tickFountain() {	
	for (unsigned int i=0; i<m_NumParticles; i++) {
		//LOGV("%f\n", m_Particles[i]->GetLife());
		if (m_Particles[i]->Tick(0.009) > 0.075) {
			reset_particle(i);
		}
	}
}


void MachineGun::drawFountain() {
	for (unsigned int i=0; i<m_NumParticles; i++) {
		m_Particles[i]->render(0);
	}
}
