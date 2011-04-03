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

#define SUBDIVIDE 30

SuperBarrelBlast::SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_Space = new Octree<int>(256 * 256, -1);

	m_Gravity = 100.0;

	m_CameraOffsetX = 0.0;
	m_CameraOffsetY = 0.0;

  m_PlayerLastX = 0.0;
  m_PlayerLastY = 0.0;

  m_LastCollideIndex = -1;
  m_LastFailedCollideIndex = -1;

  m_SpriteCount = 0;
  m_CurrentBarrelIndex = -1;
  m_LastShotBarrelIndex = -1;

  m_LaunchTimeout = 0.0;
  m_RotateTimeout = 0.0;
  m_ReloadTimeout = 0.0;
  m_SwipeTimeout = 0.0;

  float x = 100.0;
  float y = 100.0;
  float r = 0.0;

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 56, 60, 1.0, "", 0, 0, 0.0, 60.0, 60.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 500.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, -100.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_BarrelStartIndex = m_SpriteCount + 1;
  m_BarrelCount = 5;
  for (unsigned int i=0; i<m_BarrelCount; i++) {
    CreateBarrel(x, y, r);
    x += 125;
    r += 45.0;
  }
  CreateBarrel(100.0, 250.0, 360.0 + 45.0);
  m_BarrelStopIndex = m_SpriteCount;

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, 512.0, 512.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(256.0, 256.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);
}


void SuperBarrelBlast::CreateBarrel(float x, float y, float r) {
  int sx = 0;
  int sy = 0;
  m_SpriteCount++;
  float l = 30 * (1.0 / 60.0);
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 0, 2, l * 0.5, "", 8, 11, l, 70.0, 70.0));
  //m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 20, 21, l * 0.5, "", 8, 11, l, 70.0, 70.0));
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
}


SuperBarrelBlast::~SuperBarrelBlast() {
}


void SuperBarrelBlast::Hit(float x, float y, int hitState) {


  if (m_CurrentBarrelIndex != -1) {
    float theta = DEGREES_TO_RADIANS(m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation + 90.0);
    float cost = cos(theta);
    float sint = fastSinf(theta);
    float px = 100.0 * cost;
    float py = 100.0 * sint;
    float sx = 75.0 * cost;
    float sy = 75.0 * sint;
    if (hitState == 0 || hitState == 1) {
      if (m_ReloadTimeout > 0.5) {
        LOGV("launch\n");
        m_LaunchTimeout = 0.0;
        m_SwipeTimeout = 0.0;
        m_ReloadTimeout = 0.0;

        m_LastShotBarrelIndex = m_CurrentBarrelIndex;
        m_LastFailedCollideIndex = -1;

        m_AtlasSprites[0]->m_Velocity[0] = px; 
        m_AtlasSprites[0]->m_Velocity[1] = py;

        m_AtlasSprites[m_CurrentBarrelIndex]->SetEmitVelocity(sx, sy);
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[0] += cost * 20.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[1] += sint * 20.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->Reset();
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[0] -= cost * 20.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->m_Position[1] -= sint * 20.0;
        m_AtlasSprites[m_CurrentBarrelIndex]->Fire();

      } else {
        if (m_SwipeTimeout < 0.125) {
          LOGV("boosting\n");
          m_AtlasSprites[0]->m_Velocity[0] *= 1.075; 
          m_AtlasSprites[0]->m_Velocity[1] *= 1.075;
        }
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
  m_SwipeTimeout += m_DeltaTime;
  m_ReloadTimeout += m_DeltaTime;

  for (unsigned int i=0; i<m_SpriteCount+1; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
  }

  if (m_AtlasSprites[0]->m_Position[1] < 0.0 || m_AtlasSprites[0]->m_Position[0] < 0.0) {
    m_AtlasSprites[0]->SetPosition(100.0, 600.0);
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = -100.0;
  }

  bool was_falling_before = (m_LastCollideIndex == -1);

	float collide_x = m_AtlasSprites[0]->m_Position[0];
	float collide_y = m_AtlasSprites[0]->m_Position[1];
  int collide_index = m_Space->at((collide_x / SUBDIVIDE), collide_y / SUBDIVIDE, 0);
  if (collide_index != m_LastCollideIndex) {
    LOGV("switch %d %d\n", m_LastCollideIndex, collide_index);
  } 

  if (collide_index != -1 && m_LaunchTimeout > 0.5 && (m_LastFailedCollideIndex != collide_index)) {
    float dx = m_AtlasSprites[0]->m_Position[0] - m_PlayerLastX;
    float dy = (m_AtlasSprites[0]->m_Position[1] - m_PlayerLastY);
    float theta = DEGREES_TO_RADIANS((int)(m_AtlasSprites[collide_index]->m_Rotation + 90.0) % 180);
    float theta_unclamped = DEGREES_TO_RADIANS((m_AtlasSprites[collide_index]->m_Rotation + 90.0));
    float tx = 1.0 * cos(theta);
    float ty = 1.0 * fastSinf(theta);
    float angle_of_incidence = 0.0;
    float angle_of_incidence_unclamped = 0.0;
    if (was_falling_before) {
      LOGV("raw:%f dx:%f dy:%f tx:%f ty:%f\n", m_AtlasSprites[collide_index]->m_Rotation, dx, dy, tx, ty);
      angle_of_incidence = (int)RadiansToDegrees(fastAbs(atan2f(tx, ty) - atan2f(dx, dy))) % 360;
      angle_of_incidence_unclamped = (int)RadiansToDegrees(fastAbs(atan2f(tx, ty) - atan2f(dx, dy)));
      LOGV("td:%f %f\n", angle_of_incidence, RadiansToDegrees(theta_unclamped));
    } else {
    }

    if ((angle_of_incidence > 130.0 && angle_of_incidence < 230.0) || (angle_of_incidence > -50 && angle_of_incidence < 50)) {
      bool mirror = false;
      if (mirror) {

        if (was_falling_before) {
          float rt = DEGREES_TO_RADIANS(angle_of_incidence) + theta;
          float rx = -1.0 * cos(rt);
          float ry = -1.0 * fastSinf(rt);
          float vx = m_AtlasSprites[0]->m_Velocity[0];
          float vy = m_AtlasSprites[0]->m_Velocity[1];

          m_AtlasSprites[0]->m_Velocity[0] = rx * 100.0;
          m_AtlasSprites[0]->m_Velocity[1] = ry * 100.0;

          LOGV("rt:%f vx:%f vy:%f rx:%f ry:%f ex:%f ey:%f\n", (int)RadiansToDegrees(rt) % 360, vx, vy, rx, ry, m_AtlasSprites[0]->m_Velocity[0], m_AtlasSprites[0]->m_Velocity[1]);

          m_LastFailedCollideIndex = collide_index;
        }
      } else {
        if (was_falling_before) {
          LOGV("collide %d\n", collide_index);
          m_CurrentBarrelIndex = collide_index;
          m_AtlasSprites[0]->m_Velocity[0] = 0.0;
          m_AtlasSprites[0]->m_Velocity[1] = 0.0;
          m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[collide_index]->m_Position[0];
          m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[collide_index]->m_Position[1];
          m_ReloadTimeout = 0.0;
        }
        if (m_RotateTimeout > 0.125) {
          //LOGV("spin: %d\n", collide_index);
          m_AtlasSprites[collide_index]->m_Rotation += 15.0;
          m_RotateTimeout = 0.0;
        }
      }
    } else {
      LOGV("failed %d\n", collide_index);
      m_LastFailedCollideIndex = collide_index;
    }
  } else {
    m_LastFailedCollideIndex = -1;
    m_ReloadTimeout = -1;
    //m_AtlasSprites[0]->m_Velocity[1] -= (m_Gravity * m_DeltaTime);
  }

  if (m_CurrentBarrelIndex != -1) {
    if (m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive) {
      if (m_LaunchTimeout > 0.5) {
        LOGV("reset\n");
        m_AtlasSprites[m_CurrentBarrelIndex]->Reset();
      }
    }
  }

  m_LastCollideIndex = collide_index;

	m_PlayerLastX = m_AtlasSprites[0]->m_Position[0];
	m_PlayerLastY = m_AtlasSprites[0]->m_Position[1];

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
	RenderSpriteRange(m_BarrelStopIndex + 1, m_BarrelStopIndex + 2);
	RenderSpriteRange(0, 1);
	RenderSpriteRange(m_BarrelStartIndex, m_BarrelStopIndex + 1);
	glDisable(GL_BLEND);
}
