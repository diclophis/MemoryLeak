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

#define SUBDIVIDE 18

SuperBarrelBlast::SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_Space = new Octree<int>(512 * 512, -1);

	m_CameraOffsetX = 0;
	m_CameraOffsetY = 0;

  m_SpriteCount = 0;
  m_CurrentBarrelIndex = -1;
  m_LastShotBarrelIndex = -1;

  m_LaunchTimeout = 0.0;
  m_RotateTimeout = 0.0;
  m_ReloadTimeout = 0.0;

  int sx = 0;
  int sy = 0;
  float x = 100.0;
  float y = 100.0;
  float r = 0.0;

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 56, 60, 1.0, "", 0, 0, 0.0, 60.0, 60.0));
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
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 0, 2, 1.0, "", 8, 11, 5.0, 65.0, 65.0));
    m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
    m_AtlasSprites[m_SpriteCount]->m_Rotation = r;
    m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
    //m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 10.0);
    m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
    m_AtlasSprites[m_SpriteCount]->Build(1);

    sx = (x / SUBDIVIDE);
    sy = (y / SUBDIVIDE);
    int existing_index = m_Space->at(sx, sy, 0); 
    if (existing_index == -1) {
      m_Space->set(sx, sy, 0, m_SpriteCount);
    }

    x += 100;
    r += 45.0;
  }
  m_BarrelStopIndex = m_SpriteCount;
}


SuperBarrelBlast::~SuperBarrelBlast() {
}


void SuperBarrelBlast::Hit(float x, float y, int hitState) {
  if (m_CurrentBarrelIndex != -1) {
    //LOGV("foo\n");
    if (hitState == 1) {
      if (m_ReloadTimeout > 0.0) {
        LOGV("foo\n");
        m_LaunchTimeout = 0.0;
        float theta = DEGREES_TO_RADIANS(m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation + 90.0);
        m_AtlasSprites[0]->m_Velocity[0] = 100.0 * cos(theta);
        m_AtlasSprites[0]->m_Velocity[1] = 100.0 * fastSinf(theta);
        //LOGV("%f %f \n", m_AtlasSprites[0]->m_Velocity[0], m_AtlasSprites[0]->m_Velocity[1]);
          //float theta = DEGREES_TO_RADIANS(m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation + 90.0);
        float x = 15.0 * cos(theta);
        float y = 15.0 * fastSinf(theta);
        m_AtlasSprites[m_CurrentBarrelIndex]->SetEmitVelocity(x, y);
        m_LastShotBarrelIndex = m_CurrentBarrelIndex;

        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[0] += x * 1.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[1] += y * 1.0;

        m_AtlasSprites[m_CurrentBarrelIndex]->Reset();

        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[0] -= x * 1.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[1] -= y * 1.0;

      } else {
        LOGV("wang\n");
      }
    }
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
  int collide_index = m_Space->at((collide_x / SUBDIVIDE), collide_y / SUBDIVIDE, 0);

  m_AtlasSprites[0]->m_Velocity[1] -= (90.0 * m_DeltaTime);

  if (collide_index != -1 && m_LaunchTimeout > 0.5) {
    //LOGV("collide\n");
    m_ReloadTimeout += m_DeltaTime;
    m_CurrentBarrelIndex = collide_index;
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = 0.0;
    m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[collide_index]->m_Position[0];
    m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[collide_index]->m_Position[1];
    if (m_RotateTimeout > 0.5) {
      //for (unsigned int i=0; i<m_BarrelCount; i++) {
      //  m_AtlasSprites[m_BarrelStartIndex + i]->m_Rotation += 45.0;
      //}
      m_AtlasSprites[collide_index]->m_Rotation += 45.0;
      m_RotateTimeout = 0.0;
    }
  } else {
    m_ReloadTimeout = -1.0;
  }

        if (m_CurrentBarrelIndex != -1) {
          if (m_LaunchTimeout < 1.0) {
            //LOGV("a\n");
            m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive = true;
          } else {
            //LOGV("b\n");
            m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive = false;
            m_AtlasSprites[m_CurrentBarrelIndex]->Reset();
            m_LaunchTimeout = 10.0;
          }
        }






/*
  //int directions[3] = {-1, 0, 1};
      if (m_CurrentBarrelIndex != -1) {
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Life = 0.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive = false;
      }

    if (m_CurrentBarrelIndex != -1 && m_LaunchTimeout > 0.0 && m_LaunchTimeout < 10.0) {
      m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive = true;
    } else {
    }

  //for (unsigned int i=0; i<3; i++) {
    collide_index = m_Space->at((collide_x / 10.0), collide_y / 10.0, 0);
    if (collide_index != -1) {
      m_CurrentBarrelIndex = collide_index;
    //  break;
    }
  //}

  if (m_CurrentBarrelIndex != -1 && m_LaunchTimeout > 0.125) {
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = 0.0;
    m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[0];
    m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[1];
  } else {

    m_AtlasSprites[0]->m_Velocity[1] -= (100.0 * m_DeltaTime);
  }
*/


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
	RenderSpriteRange(0, 1);
	RenderSpriteRange(m_BarrelStartIndex, m_BarrelStopIndex + 1);
	glDisable(GL_BLEND);
}
