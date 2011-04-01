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

	m_Space = new Octree<int>(256 * 256, -1);

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

  for (unsigned int i=0; i<m_SpriteCount+1; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
  }

  if (m_AtlasSprites[0]->m_Position[1] < 0.0 || m_AtlasSprites[0]->m_Position[0] < 0.0) {
    m_AtlasSprites[0]->SetPosition(100.0, 300.0);
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = 0.0;
  }

  bool was_falling_before = (m_LastCollideIndex == -1);

	float collide_x = m_AtlasSprites[0]->m_Position[0];
	float collide_y = m_AtlasSprites[0]->m_Position[1];
  int collide_index = m_LastCollideIndex = m_Space->at((collide_x / SUBDIVIDE), collide_y / SUBDIVIDE, 0);

  m_AtlasSprites[0]->m_Velocity[1] -= (90.0 * m_DeltaTime);

  if (collide_index != -1 && m_LaunchTimeout > 0.5 && (m_LastFailedCollideIndex != collide_index)) {
    //LOGV("collide\n");
    float dx = m_AtlasSprites[0]->m_Position[0] - m_PlayerLastX;
    float dy = (m_AtlasSprites[0]->m_Position[1] - m_PlayerLastY);

    float theta = DEGREES_TO_RADIANS(m_AtlasSprites[collide_index]->m_Rotation + 90.0);
    float tx = 1.0 * cos(theta);
    float ty = 1.0 * fastSinf(theta);


//X= R*cos(Theta)
//Y= R*sin(Theta)
    

    float angle_of_incidence = 0.0;

    if (was_falling_before) {
      LOGV("dx:%f dy:%f tx:%f ty:%f\n", dx, dy, tx, ty);
      angle_of_incidence = RadiansToDegrees(fastAbs(atan2f(tx, ty) - atan2f(dx, dy)));
      //float theta_delta = atan2f(dx, dy) - atan2f(tx, ty);
      LOGV("td:%f\n", angle_of_incidence);


//angle of 2 relative to 1= atan2(v2.y,v2.x) - atan2(v1.y,v1.x)


/*
      float angle_of_colliding_thing = DEGREES_TO_RADIANS((int)(direct + 90.0) % 360);
      float raw_angle = (atan2f(dy, dx));
      if (raw_angle < 0) {
        //raw_angle = M_PI + raw_angle;
      }
      //if (dx != 0) {
           //r    := hypot(x,y);  ... := sqrt(x*x+y*y)
           //theta     := atan2(y,x).
  
        angle_of_incidence = RadiansToDegrees(raw_angle + angle_of_colliding_thing);
      //}
      LOGV("incidence: direct:%f rr:%f raw:%f thing:%f diff:%f %f %f\n", direct, raw_angle, RadiansToDegrees(raw_angle), RadiansToDegrees(angle_of_colliding_thing), angle_of_incidence, dx, dy);
*/
    }

    if ((angle_of_incidence > 170.0 && angle_of_incidence < 190.0) || (angle_of_incidence > -10 && angle_of_incidence < 10)) {
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
      LOGV("failed %d\n", collide_index);
      m_LastFailedCollideIndex = collide_index;
    }
  } else {
    //LOGV("reset\n");
    m_ReloadTimeout = -1.0;
  }

  if (m_CurrentBarrelIndex != -1) {
    if (m_LaunchTimeout < 1.0) {
      m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive = true;
    } else {
      m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive = false;
      m_AtlasSprites[m_CurrentBarrelIndex]->Reset();
      m_LaunchTimeout = 10.0;
    }
  }

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
	RenderSpriteRange(0, 1);
	RenderSpriteRange(m_BarrelStartIndex, m_BarrelStopIndex + 1);
	glDisable(GL_BLEND);
}
