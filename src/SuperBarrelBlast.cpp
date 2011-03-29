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

	m_Space = new Octree<int>(512 * 512, -1);

	m_CameraOffsetX = 0;
	m_CameraOffsetY = 0;

  m_SpriteCount = 0;
  m_CurrentBarrelIndex = -1;

  m_LaunchTimeout = 0.0;
  m_RotateTimeout = 0.0;

  int sx = 0;
  int sy = 0;
  float x = 100.0;
  float y = 100.0;
  float r = 0.0;

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 56, 60, 1.0, "", 0, 0, 0.0, 50.0, 50.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 200.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, -10.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_BarrelStartIndex = m_SpriteCount + 1;
  m_BarrelCount = 5;
  for (unsigned int i=0; i<m_BarrelCount; i++) {
    m_SpriteCount++;
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 0, 2, 1.0, "", 0, 0, 0.0, 50.0, 50.0));
    m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
    m_AtlasSprites[m_SpriteCount]->m_Rotation = r;
    m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
    m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
    m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
    m_AtlasSprites[m_SpriteCount]->Build(0);

    sx = (x / 10.0);
    sy = (y / 10.0);
    int existing_index = m_Space->at(sx, sy, 0); 
    if (existing_index == -1) {
      m_Space->set(sx, sy, 0, m_SpriteCount);
    }

    x += 100;
    r += 45.0;
  }
  m_BarrelStopIndex = m_SpriteCount;


  /*
  x = 0;
  y = 0;
  m_DebugBoxesStartIndex = m_SpriteCount + 1;
  m_DebugBoxesCount = 1000;
  for (unsigned int i=0; i<m_DebugBoxesCount; i++) {
    m_SpriteCount++;
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 28, 29, 1.0, "", 0, 0, 0.0, 10.0, 10.0));
    m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
    m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
    m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
    m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
    m_AtlasSprites[m_SpriteCount]->Build(0);

    sx = (x / 10.0);
    sy = (y / 10.0);
    int existing_index = m_Space->at(sx, sy, 0); 
    if (existing_index == -1) {
      m_Space->set(sx, sy, 0, m_SpriteCount);
    }

    x += 10.0;
    if (x > (50 * 10)) {
      x = 0.0;
      y += 10.0;
    }
  }
  m_DebugBoxesStopIndex = m_SpriteCount;
  */

/*
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 2, 4, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(0.0, 100.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 4, 8, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(0.0, 150.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 8, 11, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 11, 14, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(0.0, 200.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 14, 16, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(50.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 16, 20, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(50.0, 50.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 20, 24, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(50.0, 100.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 24, 26, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(50.0, 150.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 26, 28, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 32, 38, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 50.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 40, 42, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 100.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 42, 44, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 150.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 44, 48, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 200.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 48, 50, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(-50.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 50, 52, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(-50.0, 50.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 52, 54, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(-50.0, 100.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 54, 56, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(-50.0, 150.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 56, 60, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(-100.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 61, 64, 1.0, "", 0, 0, 0.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(-100.0, 50.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);
*/
	
}


SuperBarrelBlast::~SuperBarrelBlast() {
}


void SuperBarrelBlast::Hit(float x, float y, int hitState) {
  if (m_CurrentBarrelIndex != -1) {
    //LOGV("foo\n");
    m_LaunchTimeout = 0.0;
    float theta = DEGREES_TO_RADIANS(m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation + 90.0);
    m_AtlasSprites[0]->m_Velocity[0] = 100.0 * cos(theta);
    m_AtlasSprites[0]->m_Velocity[1] = 100.0 * fastSinf(theta);
    //LOGV("%f %f \n", m_AtlasSprites[0]->m_Velocity[0], m_AtlasSprites[0]->m_Velocity[1]);
    m_CurrentBarrelIndex = -1;
  }
}


void SuperBarrelBlast::Build() {
  m_IsPushingAudio = true;
}


int SuperBarrelBlast::Simulate() {

  m_LaunchTimeout += m_DeltaTime;
  m_RotateTimeout += m_DeltaTime;

  if (m_AtlasSprites[0]->m_Position[1] < 0.0 || m_AtlasSprites[0]->m_Position[0] < 0.0) {
    m_AtlasSprites[0]->SetPosition(100.0, 300.0);
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = 0.0;
  }

	float collide_x = m_AtlasSprites[0]->m_Position[0];
	float collide_y = m_AtlasSprites[0]->m_Position[1];
  int collide_index = -1;

  collide_index = m_Space->at(collide_x / 10.0, collide_y / 10.0, 0);
  
  m_CurrentBarrelIndex = collide_index;

  //if (collide_index != -1) {
    //if (collide_index > m_DebugBoxesStartIndex && collide_index < m_DebugBoxesStopIndex) {
    //  m_CurrentBarrelIndex = -1;
    //  m_AtlasSprites[collide_index]->m_Scale[0] = 2.0;
    //  m_AtlasSprites[collide_index]->m_Scale[1] = 2.0;
    //} else {
      //m_CurrentBarrelIndex = collide_index;
    //}
  //}

  if (m_CurrentBarrelIndex != -1 && m_LaunchTimeout > 0.5) {
    //LOGV("locking into: %d\n", m_CurrentBarrelIndex);
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = 0.0;
    m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[0];
    m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[1];
  } else {
    m_AtlasSprites[0]->m_Velocity[1] -= (110.0 * m_DeltaTime);
  }

  if (m_RotateTimeout > 0.5) {
    for (unsigned int i=0; i<m_BarrelCount; i++) {
      m_AtlasSprites[m_BarrelStartIndex + i]->m_Rotation += 45.0;
    }
    m_RotateTimeout = 0.0;
  }

  for (unsigned int i=0; i<m_SpriteCount+1; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
  }

	m_CameraOffsetX = m_AtlasSprites[0]->m_Position[0];
	m_CameraOffsetY = m_AtlasSprites[0]->m_Position[1];

	return 1;
}


void SuperBarrelBlast::RenderModelPhase() {
}


void SuperBarrelBlast::RenderSpritePhase() {
	//glClearColor(0.8, 0.8, 0.9, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	glTranslatef(-m_CameraOffsetX, -m_CameraOffsetY, 0.0);
	//RenderSpriteRange(m_DebugBoxesStartIndex, m_DebugBoxesStopIndex + 1);
	RenderSpriteRange(m_BarrelStartIndex, m_BarrelStopIndex + 1);
	RenderSpriteRange(0, 1);
	glDisable(GL_BLEND);
}
