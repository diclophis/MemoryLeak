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

enum colliders {
  BARREL = 1,
  MIRROR = 2 
};

SuperBarrelBlast::SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_Space = new Octree<int>(256 * 256, -1);

	m_Gravity = 0.0;

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
  m_MirrorRotateTimeout = 0.0;
  m_ReloadTimeout = 0.0;
  m_SwipeTimeout = 0.0;

  float x = 30.0;
  float y = 120.0;
  float r = 0.0;

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 56, 60, 1.0, "", 0, 0, 0.0, 60.0, 60.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(270.0, 500.0);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(0.0, -100.0);
	m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
	m_AtlasSprites[m_SpriteCount]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->SetScale(1.0, 1.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_BarrelStartIndex = m_SpriteCount + 1;
  m_BarrelCount = 5;
  for (unsigned int i=0; i<m_BarrelCount; i++) {
    CreateBarrel(x, y, r);
    x += 120;
    r += 0.0;
  }
  CreateBarrel(270.0, 270.0, 180.0);
  m_BarrelStopIndex = m_SpriteCount;

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, 512.0, 512.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(256.0, 256.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_DebugBoxesStartIndex = m_SpriteCount + 1;
  m_DebugBoxesCount = 500;
  x = 0.0;
  y = 0.0;

  for (unsigned int i=0; i<m_DebugBoxesCount; i++) {
    m_SpriteCount++;
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(2), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, SUBDIVIDE, SUBDIVIDE));
    m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    float sx = (x / SUBDIVIDE);
    float sy = (y / SUBDIVIDE);
    int existing_index = m_Space->at(sx, sy, 0); 
    if (existing_index == -1) {
      m_Space->set(sx, sy, 0, m_SpriteCount);
    }
    if (x > 1000.0) {
      y += SUBDIVIDE;
      x = 0;
    } else {
      x += SUBDIVIDE;
    }
  }
  m_DebugBoxesStopIndex = m_SpriteCount;
}


void SuperBarrelBlast::CreateBarrel(float x, float y, float r) {
  int sx = 0;
  int sy = 0;
  int flag = 0;
  m_SpriteCount++;
  float l = 30 * (1.0 / 60.0);
  if ((m_SpriteCount % 2) == 0) {
    flag = BARREL;
  } else {
    flag = MIRROR;
  }

  if (flag & BARREL) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 0, 2, l * 0.5, "", 8, 11, l, 70.0, 70.0));
  } else if (flag & MIRROR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 20, 21, l * 0.5, "", 8, 11, l, 70.0, 70.0));
  }

  m_AtlasSprites[m_SpriteCount]->m_IsFlags = flag;

  m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
  m_AtlasSprites[m_SpriteCount]->m_Rotation = r;
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

	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;

  float dx = (xx + m_CameraOffsetX) + (SUBDIVIDE * 0.5);
  float dy = (yy + m_CameraOffsetY) + (SUBDIVIDE * 0.5);
	float collide_x = dx;
	float collide_y = dy;
  LOGV("target:100,250 xx:%f,%f hit:%f,%f cam:%f,%f d:%f,%f %f %f\n", xx, yy, x, y, m_CameraOffsetX, m_CameraOffsetY, dx, dy, collide_x, collide_y);
  int collide_index = m_Space->at((collide_x / SUBDIVIDE), (collide_y / SUBDIVIDE), 0);
  LOGV("found: %d\n", collide_index);
  if (collide_index != -1) {
    if (collide_index >= m_DebugBoxesStartIndex && collide_index <= m_DebugBoxesStopIndex) {
      //m_AtlasSprites[collide_index]->SetScale(0.5, 0.5);
      m_AtlasSprites[collide_index]->m_Rotation += 15.0;
      collide_index = -1;
    }
  }
  if (hitState == 2 && collide_index != -1) {
    m_AtlasSprites[collide_index]->m_Rotation += 15.0;
  } else {
    LOGV("maybe\b");

    if (collide_index == -1 && m_CurrentBarrelIndex != -1) {
    LOGV("shoot\b");
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
        LOGV("what\n");
          if (m_SwipeTimeout < 0.125) {
            LOGV("boosting\n");
            m_AtlasSprites[0]->m_Velocity[0] *= 1.075; 
            m_AtlasSprites[0]->m_Velocity[1] *= 1.075;
          }
        }
      }
    }
  }
}


void SuperBarrelBlast::Build() {
  m_IsPushingAudio = true;
}


int SuperBarrelBlast::Simulate() {
  //LOGV("get: %d=%d\n", 1, m_AtlasSprites[1]->m_IsFlags);
  //exit(1);
  //LOGV("y:%f\n", m_AtlasSprites[0]->m_Position[1]);

  m_LaunchTimeout += m_DeltaTime;
  m_RotateTimeout += m_DeltaTime;
  m_MirrorRotateTimeout += m_DeltaTime;
  m_SwipeTimeout += m_DeltaTime;
  m_ReloadTimeout += m_DeltaTime;

  bool rotate_mirrors_this_tick = false;

  if (m_MirrorRotateTimeout > 0.05) {
    rotate_mirrors_this_tick = true;
    m_MirrorRotateTimeout = 0.0;
  }

  for (unsigned int i=0; i<m_SpriteCount+1; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
    if (rotate_mirrors_this_tick) {
      if (m_AtlasSprites[i]->m_IsFlags & MIRROR) {
        //m_AtlasSprites[i]->m_Rotation += 1.0;
      }
    }
  }

  if (m_AtlasSprites[0]->m_Position[1] < 0.0 || m_AtlasSprites[0]->m_Position[0] < 0.0) {
    m_AtlasSprites[0]->SetPosition(270.0, 600.0);
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = -100.0;
  }

  bool was_falling_before = (m_LastCollideIndex == -1);

	float collide_x = m_AtlasSprites[0]->m_Position[0];
	float collide_y = m_AtlasSprites[0]->m_Position[1];
  int collide_index = m_Space->at((collide_x / SUBDIVIDE), collide_y / SUBDIVIDE, 0);

  if (collide_index != -1) {
    if (collide_index >= m_DebugBoxesStartIndex && collide_index <= m_DebugBoxesStopIndex) {
      m_AtlasSprites[collide_index]->SetScale(0.5, 0.5);
      collide_index = -1;
    }
  }

  if (collide_index != m_LastCollideIndex) {
    LOGV("switch %d %d\n", m_LastCollideIndex, collide_index);
  } 

  //0 -> up
  //180 -> down
  if (collide_index != -1 && m_LaunchTimeout > 0.5 && (m_LastFailedCollideIndex != collide_index)) {
    
    float dx = m_AtlasSprites[0]->m_Position[0] - m_PlayerLastX;
    float dy = (m_AtlasSprites[0]->m_Position[1] - m_PlayerLastY);
    float player_theta = atan2f(dx, dy);

    float collider_theta = DEGREES_TO_RADIANS((int)(m_AtlasSprites[collide_index]->m_Rotation));
    float collider_xoft = 1.0 * cos(collider_theta);
    float collider_yoft = 1.0 * fastSinf(collider_theta);
    float readjusted_collider_theta = atan2f(collider_xoft, collider_yoft);
    float clamped_collider_theta = (int)RadiansToDegrees(readjusted_collider_theta) % 180;


    LOGV("collider_theta:%f player_theta:%f ==> %f %f\n", RadiansToDegrees(collider_theta), RadiansToDegrees(player_theta), RadiansToDegrees(readjusted_collider_theta), clamped_collider_theta);

    //-44 => -135
    //44 => 135

    if ((clamped_collider_theta > -136 && clamped_collider_theta < -43) || (clamped_collider_theta < 136 && clamped_collider_theta > 43)) {
      bool mirror = false;
      //LOGV("mir%d bar:%d\n", m_AtlasSprites[collide_index]->m_IsFlags & MIRROR, m_AtlasSprites[collide_index]->m_IsFlags & BARREL);

      if (m_AtlasSprites[collide_index]->m_IsFlags & MIRROR) {

        if (was_falling_before) {

          //out = incidentVec - 2.f * Dot(incidentVec, normal) * normal;

          LOGV("reflect\n");
          LOGV("collider_theta:%f player_theta:%f ==> %f %f\n", RadiansToDegrees(collider_theta), RadiansToDegrees(player_theta), RadiansToDegrees(readjusted_collider_theta), clamped_collider_theta);

          float rt = DEGREES_TO_RADIANS(90) + 2.0 * collider_theta; //(2.0 * DEGREES_TO_RADIANS(clamped_collider_theta)) - player_theta;
          float rx = 100.0 * cos(rt);
          float ry = 100.0 * fastSinf(rt);

          m_AtlasSprites[0]->m_Velocity[0] = rx;
          m_AtlasSprites[0]->m_Velocity[1] = ry;
          m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[collide_index]->m_Position[0];
          m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[collide_index]->m_Position[1];

          LOGV("!!!mirror:%d rt:%f p:%f,%f v:%f,%f r:%f,%f\n",
            collide_index, RadiansToDegrees(rt), m_AtlasSprites[collide_index]->m_Position[0], m_AtlasSprites[collide_index]->m_Position[1], m_AtlasSprites[0]->m_Velocity[0], m_AtlasSprites[0]->m_Velocity[1], rx, ry);


          m_LastFailedCollideIndex = collide_index;
        }
      } else if (m_AtlasSprites[collide_index]->m_IsFlags & BARREL) {
        if (was_falling_before) {
          LOGV("collide %d flag:%d\n", collide_index, m_AtlasSprites[collide_index]->m_IsFlags);
          m_CurrentBarrelIndex = collide_index;
          m_LastFailedCollideIndex = -1;
          m_AtlasSprites[0]->m_Velocity[0] = 0.0;
          m_AtlasSprites[0]->m_Velocity[1] = 0.0;
          m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[collide_index]->m_Position[0];
          m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[collide_index]->m_Position[1];
          m_ReloadTimeout = 0.0;
        } else {
          //LOGV("failed %d\n", collide_index);
        }
      }
    }
    //else {
    //  LOGV("failed %d\n", collide_index);
    //  m_LastFailedCollideIndex = collide_index;
    //}

/*


    float angle_of_incidence = 0.0;
    if (was_falling_before) {
      LOGV("rotation of collider:%f\n", m_AtlasSprites[collide_index]->m_Rotation);
      angle_of_incidence = (int)RadiansToDegrees(fastAbs(atan2f(tx, ty) - atan2f(dx, dy)));
      LOGV("collide normal:%f player angle:%f\n", RadiansToDegrees(atan2f(tx, ty)), RadiansToDegrees(atan2f(dx, dy)));
      LOGV("angle of incidence:%f\n", angle_of_incidence);
    }

    float max_theta_delta = 61;

    if (
      (angle_of_incidence >= (360 - max_theta_delta) && angle_of_incidence <= (360 + max_theta_delta)) ||
      (angle_of_incidence >= (270 - max_theta_delta) && angle_of_incidence <= (270 + max_theta_delta)) ||
      (angle_of_incidence >= (180 - max_theta_delta) && angle_of_incidence <= (180 + max_theta_delta)) ||
      (angle_of_incidence >= (90 - max_theta_delta) && angle_of_incidence <= (90 + max_theta_delta)) ||
      (angle_of_incidence >= -max_theta_delta && angle_of_incidence <= max_theta_delta)
    ) {
      bool mirror = false;
      //LOGV("mir%d bar:%d\n", m_AtlasSprites[collide_index]->m_IsFlags & MIRROR, m_AtlasSprites[collide_index]->m_IsFlags & BARREL);

      if (m_AtlasSprites[collide_index]->m_IsFlags & MIRROR) {

        if (was_falling_before) {
          float rt = atan2f(dx, dy) - DEGREES_TO_RADIANS(90) - DEGREES_TO_RADIANS(angle_of_incidence); //atan2f(dx, dy);
          //}
          float rx = 1.0 * cos(rt);
          float ry = 1.0 * fastSinf(rt);

          //float vx = m_AtlasSprites[0]->m_Velocity[0];
          //float vy = m_AtlasSprites[0]->m_Velocity[1];


          m_AtlasSprites[0]->m_Velocity[0] = rx * 100.0;
          m_AtlasSprites[0]->m_Velocity[1] = ry * 100.0;
          m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[collide_index]->m_Position[0];
          m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[collide_index]->m_Position[1];

          LOGV("!!!mirror:%d rt:%f p:%f,%f v:%f,%f r:%f,%f\n",
            collide_index, RadiansToDegrees(rt), m_AtlasSprites[collide_index]->m_Position[0], m_AtlasSprites[collide_index]->m_Position[1], m_AtlasSprites[0]->m_Velocity[0], m_AtlasSprites[0]->m_Velocity[1], rx, ry);


          //m_AtlasSprites[m_CurrentBarrelIndex]->Reset();
          //m_AtlasSprites[m_CurrentBarrelIndex]->Fire();

          //LOGV("rt:%f vx:%f vy:%f rx:%f ry:%f ex:%f ey:%f\n", (int)RadiansToDegrees(rt) % 360, vx, vy, rx, ry, m_AtlasSprites[0]->m_Velocity[0], m_AtlasSprites[0]->m_Velocity[1]);

          m_LastFailedCollideIndex = collide_index;
        }
      } else if (m_AtlasSprites[collide_index]->m_IsFlags & BARREL) {
        if (was_falling_before) {
          LOGV("collide %d flag:%d\n", collide_index, m_AtlasSprites[collide_index]->m_IsFlags);
          m_CurrentBarrelIndex = collide_index;
          m_LastFailedCollideIndex = -1;
          m_AtlasSprites[0]->m_Velocity[0] = 0.0;
          m_AtlasSprites[0]->m_Velocity[1] = 0.0;
          m_AtlasSprites[0]->m_Position[0] = m_AtlasSprites[collide_index]->m_Position[0];
          m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[collide_index]->m_Position[1];
          m_ReloadTimeout = 0.0;
        }
      }
    } else {
      LOGV("failed %d\n", collide_index);
      m_LastFailedCollideIndex = collide_index;
    }
*/
  } else {
    m_ReloadTimeout = -1;
    m_AtlasSprites[0]->m_Velocity[1] -= (m_Gravity * m_DeltaTime);
  }

  if (m_CurrentBarrelIndex != -1) {
    if (m_RotateTimeout > 0.33) {
      //LOGV("spin: %d\n", collide_index);
      //m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation += 15.0;
      m_RotateTimeout = 0.0;
    }
    if (m_AtlasSprites[m_CurrentBarrelIndex]->m_IsAlive) {
      if (m_LaunchTimeout > 0.5) {
        LOGV("reset\n");
        m_AtlasSprites[m_CurrentBarrelIndex]->Reset();
        //m_CurrentBarrelIndex = -1;
      }
    }
  }

  m_LastCollideIndex = collide_index;

	m_PlayerLastX = m_AtlasSprites[0]->m_Position[0];
	m_PlayerLastY = m_AtlasSprites[0]->m_Position[1];

  float ox = m_PlayerLastX - m_CameraOffsetX;
  float oy = m_PlayerLastY - m_CameraOffsetY;

  ox *= 0.001;
  oy *= 0.001;

	m_CameraOffsetX += ox;
	m_CameraOffsetY += oy;

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
	RenderSpriteRange(m_DebugBoxesStartIndex, m_DebugBoxesStopIndex + 1);
	//RenderSpriteRange(m_BarrelStopIndex + 1, m_BarrelStopIndex + 2);
	RenderSpriteRange(0, 1);
	RenderSpriteRange(m_BarrelStartIndex, m_BarrelStopIndex + 1);
	glDisable(GL_BLEND);
}
