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
#define BARREL_ROTATE_TIMEOUT 0.25
#define BARREL_ROTATE_PER_TICK 45.0 
#define SHOOT_VELOCITY 200.0
#define GRID_SIZE 7

enum colliders {
  BARREL = 1,
  MIRROR = 2 
};

SuperBarrelBlast::SuperBarrelBlast(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_Space = new Octree<int>(256 * 256, -1);
  
  m_LastTouchedIndex = -1;
  m_DidDrag = false;

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
	m_AtlasSprites[m_SpriteCount]->SetPosition(390, 270.0);
  float st = DEGREES_TO_RADIANS(-90);
  float vx = SHOOT_VELOCITY * cos(st);
  float vy = SHOOT_VELOCITY * fastSinf(st);
	m_AtlasSprites[m_SpriteCount]->SetVelocity(vx, vy);
	m_AtlasSprites[m_SpriteCount]->Build(0);
	m_CameraOffsetX = m_AtlasSprites[0]->m_Position[0];
	m_CameraOffsetY = m_AtlasSprites[0]->m_Position[1];

  m_BarrelStartIndex = m_SpriteCount + 1;
  m_BarrelCount = 5;
  for (unsigned int i=0; i<m_BarrelCount; i++) {
    CreateCollider(x, y, r, BARREL);
    x += 120;
    r += 0.0;
  }
  CreateCollider(270.0, 240.0, 90.0 + 45.0, MIRROR);
  CreateCollider(390.0, 30.0, 90.0 + 45.0, BARREL);
  m_BarrelStopIndex = m_SpriteCount;

  m_SpriteCount++;
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, 2048.0, 2048.0));
	m_AtlasSprites[m_SpriteCount]->SetPosition(512.0, 0.0);
	m_AtlasSprites[m_SpriteCount]->Build(0);

  m_DebugBoxesStartIndex = m_SpriteCount + 1;
  m_DebugBoxesCount = 49;
  x = 0.0;
  y = 0.0;

  for (unsigned int i=0; i<m_DebugBoxesCount; i++) {
    m_SpriteCount++;
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(2), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, SUBDIVIDE, SUBDIVIDE));
    m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    /*
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
    */
  }
  m_DebugBoxesStopIndex = m_SpriteCount;
}


void SuperBarrelBlast::CreateCollider(float x, float y, float r, int flag) {
  int sx = 0;
  int sy = 0;
  m_SpriteCount++;
  float l = 30 * (1.0 / 60.0);

  if (flag & BARREL) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 0, 2, l * 0.5, "", 8, 11, l, 70.0, 70.0));
  } else if (flag & MIRROR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 20, 21, l * 0.5, "", 8, 11, l, 70.0, 70.0));
  }

  m_AtlasSprites[m_SpriteCount]->m_IsFlags = flag;

  m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
  m_AtlasSprites[m_SpriteCount]->m_Rotation = r;
  m_AtlasSprites[m_SpriteCount]->Build(10);
  sx = (x / SUBDIVIDE);
  sy = (y / SUBDIVIDE);
  int existing_index = m_Space->at(sx, sy, 0); 
  if (existing_index == -1) {
    m_Space->set(sx, sy, 0, m_SpriteCount);
  }
}


void SuperBarrelBlast::IndexToXY(int index, int* x, int* y) {
  *y = index / GRID_SIZE;
  *x = index - *y * GRID_SIZE;
}


int XYToIndex(int x, int y) {
  return (y * GRID_SIZE + x);
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
  int cx = (collide_x / SUBDIVIDE);
  int cy = (collide_y / SUBDIVIDE);
  int collide_index = -1;
  if (collide_x > 0 && collide_y > 0) {
    collide_index = m_Space->at(cx, cy, 0);
  }
  if (collide_index != -1) {
    if (collide_index >= m_DebugBoxesStartIndex && collide_index <= m_DebugBoxesStopIndex) {
      m_AtlasSprites[collide_index]->m_Rotation += 5.0;
      collide_index = -1;
    }
  }
  if (hitState == 0 && collide_index != -1 && collide_index != m_CurrentBarrelIndex) {
    //pickup
    m_DidDrag = false;
    m_Space->set(cx, cy, 0, -1);
    m_LastTouchedIndex = collide_index;
  } else if (hitState == 1 && m_LastTouchedIndex != -1) {
    //move
    if (collide_index == -1 && (cx > 0 && cy > 0)) {
      float tx = (int)(collide_x / SUBDIVIDE) * SUBDIVIDE;
      float ty = (int)(collide_y / SUBDIVIDE) * SUBDIVIDE;
      m_AtlasSprites[m_LastTouchedIndex]->SetPosition(tx, ty);
      m_Space->set(cx, cy, 0, m_LastTouchedIndex);
      m_DidDrag = true;
    }
  } else if (hitState == 2 && m_LastTouchedIndex != -1 && !m_DidDrag) {
    m_AtlasSprites[m_LastTouchedIndex]->m_Rotation += 45.0;
    m_DidDrag = false;
  } else if (hitState == 2 && m_LastTouchedIndex != -1 && m_DidDrag) {
    if (cx > 0 && cy > 0) {
      if (m_LastTouchedIndex) {
        m_Space->set(cx, cy, 0, m_LastTouchedIndex);
        m_LastTouchedIndex = -1;
      }
      m_DidDrag = false;
    }
  } else {
    if (collide_index == m_CurrentBarrelIndex) {
      float theta = DEGREES_TO_RADIANS(m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation + 90.0);
      float cost = cos(theta);
      float sint = fastSinf(theta);
      float px = SHOOT_VELOCITY * cost;
      float py = SHOOT_VELOCITY * sint;
      float sx = SHOOT_VELOCITY * 0.1 * cost;
      float sy = SHOOT_VELOCITY * 0.1 * sint;
      if (hitState == 2) {
        //if (m_ReloadTimeout > 0.5) {
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
        /*
        } else {
          if (m_SwipeTimeout < 0.125) {
            m_AtlasSprites[0]->m_Velocity[0] *= 1.075; 
            m_AtlasSprites[0]->m_Velocity[1] *= 1.075;
            LOGV("bump\n");
          } else {
            LOGV("no bump, cant shoot\n");
          }
        }
        */
      } else {
        LOGV("wtf 2\n");
      }
    } else {
      LOGV("wtf 1\n");
    }
  }
}


void SuperBarrelBlast::Build() {
  m_IsPushingAudio = true;
}


int SuperBarrelBlast::Simulate() {
  m_LaunchTimeout += m_DeltaTime;
  m_RotateTimeout += m_DeltaTime;
  m_MirrorRotateTimeout += m_DeltaTime;
  m_SwipeTimeout += m_DeltaTime;
  m_ReloadTimeout += m_DeltaTime;

  bool rotate_mirrors_this_tick = false;

  if (m_MirrorRotateTimeout > 0.5) {
    rotate_mirrors_this_tick = true;
    m_MirrorRotateTimeout = 0.0;
  }

  for (unsigned int i=0; i<m_SpriteCount+1; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
    if (rotate_mirrors_this_tick) {
      if (m_AtlasSprites[i]->m_IsFlags & MIRROR) {
        //m_AtlasSprites[i]->m_Rotation += 45.0;
      }
    }
    if (i >= m_DebugBoxesStartIndex && i <= m_DebugBoxesStopIndex) {
      int ox = -1;
      int oy = -1;
      IndexToXY(i - m_DebugBoxesStartIndex, &ox, &oy);
      float ax = (ox * SUBDIVIDE) + m_AtlasSprites[0]->m_Position[0] - (3 * SUBDIVIDE);
      float ay = (oy * SUBDIVIDE) + m_AtlasSprites[0]->m_Position[1] - (3 * SUBDIVIDE);
      m_AtlasSprites[i]->SetPosition(ax, ay);
      LOGV("i:%d x:%d y:%d ax:%f ay:%f\n", i, ox, oy, ax, ay);
    }
  }

  if (m_AtlasSprites[0]->m_Position[1] < 0.0 || m_AtlasSprites[0]->m_Position[0] < 0.0) {
    m_AtlasSprites[0]->SetPosition(270.0, 600.0);
    m_AtlasSprites[0]->m_Velocity[0] = 0.0;
    m_AtlasSprites[0]->m_Velocity[1] = -SHOOT_VELOCITY;
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

  if (collide_index != -1 && m_LaunchTimeout > 0.25 && (m_LastFailedCollideIndex != collide_index)) {
    
    float dx = (m_AtlasSprites[0]->m_Position[0] - m_PlayerLastX);
    float dy = (m_AtlasSprites[0]->m_Position[1] - m_PlayerLastY);
    float player_theta = atan2f(dy, dx);

    float collider_theta = DEGREES_TO_RADIANS((int)(m_AtlasSprites[collide_index]->m_Rotation));
    float collider_theta_normal = DEGREES_TO_RADIANS((int)((m_AtlasSprites[collide_index]->m_Rotation) + 90) % 180);
    float collider_xoft = 1.0 * cos(collider_theta);
    float collider_yoft = 1.0 * fastSinf(collider_theta);
    float readjusted_collider_theta = atan2f(collider_yoft, collider_xoft);
    float clamped_collider_theta = (int)RadiansToDegrees(readjusted_collider_theta) % 180;

    float delta_from_normal = collider_theta_normal + player_theta; 

    if (
      (RadiansToDegrees(delta_from_normal) >= -45.0 || RadiansToDegrees(delta_from_normal) < -44.0) ||
      (RadiansToDegrees(delta_from_normal) <= 45.0 && RadiansToDegrees(delta_from_normal) > 44.0) ||
      (RadiansToDegrees(delta_from_normal) <= 270.0 && RadiansToDegrees(delta_from_normal) > 269.0) ||
      RadiansToDegrees(delta_from_normal) == 0.0 ||
      RadiansToDegrees(delta_from_normal) == 90.0 ||
      (RadiansToDegrees(delta_from_normal) <= 180.0 && RadiansToDegrees(delta_from_normal) > 179.0)
    ) {
      bool mirror = false;
      if (m_AtlasSprites[collide_index]->m_IsFlags & MIRROR) {
        if (was_falling_before) {
          LOGV("reflect\n");
          LOGV("collider_theta:%f player_theta:%f ==> %f %f\n", roundf(RadiansToDegrees((collider_theta))), RadiansToDegrees(player_theta), RadiansToDegrees(readjusted_collider_theta), clamped_collider_theta);
          float rt = -DEGREES_TO_RADIANS(roundf(RadiansToDegrees((collider_theta))));
          float rx = SHOOT_VELOCITY * cos(rt);
          float ry = SHOOT_VELOCITY * fastSinf(rt);
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
        }
      }
    }
  } else {
    m_ReloadTimeout = -1;
    m_AtlasSprites[0]->m_Velocity[1] -= (m_Gravity * m_DeltaTime);
  }

  if (m_CurrentBarrelIndex != -1) {
    if (m_RotateTimeout > BARREL_ROTATE_TIMEOUT) {
      m_AtlasSprites[m_CurrentBarrelIndex]->m_Rotation += BARREL_ROTATE_PER_TICK;
      m_RotateTimeout = 0.0;
    }
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

  float ox = m_PlayerLastX - m_CameraOffsetX;
  float oy = m_PlayerLastY - m_CameraOffsetY;

  ox *= 0.9 * m_DeltaTime;
  oy *= 0.9 * m_DeltaTime;

	m_CameraOffsetX += ox;
	m_CameraOffsetY += oy;

	return 1;
}


void SuperBarrelBlast::RenderModelPhase() {
}


void SuperBarrelBlast::RenderSpritePhase() {
	glClearColor(0.8, 0.8, 0.9, 1.0);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	glTranslatef(-m_CameraOffsetX, -m_CameraOffsetY, 0.0);
	RenderSpriteRange(m_BarrelStopIndex + 1, m_BarrelStopIndex + 2);
	RenderSpriteRange(m_DebugBoxesStartIndex, m_DebugBoxesStopIndex + 1);
	RenderSpriteRange(0, 1);
	RenderSpriteRange(m_BarrelStartIndex, m_BarrelStopIndex + 1);
	glDisable(GL_BLEND);
}
