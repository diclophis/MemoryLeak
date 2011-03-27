// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"
#include "SuperBarrelBlast.h"


SuperBarrelBlast::SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_CameraOffsetX = 0;
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 10, 11, 1.0, "", 10, 11, 0.5));
	m_AtlasSprites[0]->SetPosition(128 * 50.0, 100.0);
	m_AtlasSprites[0]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[0]->m_IsAlive = true;
	m_AtlasSprites[0]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[0]->SetScale(1.0, 1.0);
	m_AtlasSprites[0]->Build(20);
	
	m_Space = new Octree<int>(512 * 512, -1);
}


SuperBarrelBlast::~SuperBarrelBlast() {
}


void SuperBarrelBlast::Hit(float x, float y, int hitState) {
	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;
	m_AtlasSprites[0]->m_Position[1] = yy * m_SimulationTime;
}


void SuperBarrelBlast::Build() {
  m_IsPushingAudio = true;
}


int SuperBarrelBlast::Simulate() {
	m_CameraOffsetX = m_AtlasSprites[0]->m_Position[0];
	return 1;
}


void SuperBarrelBlast::RenderModelPhase() {
}


void SuperBarrelBlast::RenderSpritePhase() {
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE);
	glTranslatef(-m_CameraOffsetX, 0.0, 0.0);
	RenderSpriteRange(0, 1);
	glDisable(GL_BLEND);
}
