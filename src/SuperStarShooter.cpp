// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SuperStarShooter.h"


enum colliders {
  BARREL = 1,
  MIRROR = 2,
  STAR = 4
};


#define SUBDIVIDE 75.0
#define BARREL_ROTATE_TIMEOUT 0.33
#define BARREL_ROTATE_PER_TICK 0 
#define SHOOT_VELOCITY 425.0
#define GRID_X 10
#define GRID_Y 10
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 


SuperStarShooter::SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {

  int xx = 0;
  int yy = 0;

  m_DebugTimeout = 0.0;

  m_PercentThere = 0.0;

  m_Zoom = 1.0;

  LoadSound(1);
  m_IsPushingAudio = true;

	m_CameraActualOffsetX = m_CameraStopOffsetX = m_CameraOffsetX = 0.0;
	m_CameraActualOffsetY = m_CameraStopOffsetY = m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(16 * 16, -64);

  //CreateCollider(SUBDIVIDE * 2, SUBDIVIDE * 2, 0.0, STAR);
  
  m_GridCount = GRID_X * GRID_Y;
  m_GridPositions = new int[m_GridCount * 2];
  m_GridStartIndex = m_SpriteCount;

  for (unsigned int i=0; i<m_GridCount; i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 64, 1.0, "", 0, 64, 0.0, SUBDIVIDE, SUBDIVIDE));
    m_AtlasSprites[m_SpriteCount]->Build(0);
    m_GridPositions[(i * 2)] = xx;
    m_GridPositions[(i * 2) + 1] = yy;
    m_AtlasSprites[m_SpriteCount]->SetPosition(xx * 10, yy * 10);
    xx++;
    if (xx >= GRID_X) {
      xx = 0;
      yy++;
    }
    m_SpriteCount++;
  }
  m_GridStopIndex = m_SpriteCount;

  m_WarpTimeout = 0.0;
}


SuperStarShooter::~SuperStarShooter() {
  AtlasSprite::ReleaseBuffers();
  delete m_Space;
  delete m_GridPositions;
}


void SuperStarShooter::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
  m_CameraOffsetX = -xx;
  m_CameraOffsetY = -yy;

/*
  float dx = (xx + m_CameraOffsetX) + (SUBDIVIDE * 0.5);
  float dy = (yy + m_CameraOffsetY) + (SUBDIVIDE * 0.5);
	float collide_x = (dx);
	float collide_y = (dy);
  int cx = (collide_x / SUBDIVIDE);
  int cy = (collide_y / SUBDIVIDE);
  int collide_index = -1;
  bool collide_index_set = false;
  if (cx > 0 && cy > 0) {
    collide_index_set = true;
    collide_index = m_Space->at(cx, cy, 0);
  }
  if (hitState == 0) {
    m_CameraStopOffsetX = (xx + m_CameraOffsetX);
    m_CameraStopOffsetY = (yy + m_CameraOffsetY);
  }
  m_CameraOffsetX = m_CameraStopOffsetX - xx;
  m_CameraOffsetY = m_CameraStopOffsetY -yy;
*/
}


void SuperStarShooter::RenderModelPhase() {
}


void SuperStarShooter::RenderSpritePhase() {
  glTranslatef(-m_CameraActualOffsetX, -m_CameraActualOffsetY, 0.0);
  RenderSpriteRange(m_GridStartIndex, m_GridStopIndex);
  AtlasSprite::Scrub();
  //RenderSpriteRange(0, m_GridStartIndex);
  AtlasSprite::Scrub();
  //AtlasSprite::ReleaseBuffers();
}


int SuperStarShooter::Simulate() {
  bool print_index = false;
  if (m_DebugTimeout > 2.0) {
    m_DebugTimeout = 0.0;
    print_index = true;
  }

  m_DebugTimeout += m_DeltaTime;

/*
  for (unsigned int i=0; i<m_SpriteCount; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
    if (i >= m_GridStartIndex && i <= m_GridStopIndex) {
      int annotate_index = -12;
      int ox = -1;
      int oy = -1;
      IndexToXY(i - m_GridStartIndex, &ox, &oy);
      float bumpx = -0.5;
      float bumpy = -0.5;
      //if (m_AtlasSprites[i]->m_Position[0] > 0.0) {
      //  bumpx = 0.5;
      //}
      //if (m_AtlasSprites[i]->m_Position[1] > 0.0) {
      //  bumpy = 0.5;
      //}
      float ax = (ox * SUBDIVIDE) + (m_CameraActualOffsetX + bumpx) - (((GRID_X - 1) / 2) * SUBDIVIDE);
      float ay = (oy * SUBDIVIDE) + (m_CameraActualOffsetY + bumpy) - (((GRID_Y - 1) / 2) * SUBDIVIDE);
      //float wtfx = ((int)((ax) / SUBDIVIDE) * SUBDIVIDE);
      //float wtfy = ((int)((ay) / SUBDIVIDE) * SUBDIVIDE);
      float wtfx = (((ax) / SUBDIVIDE) * SUBDIVIDE);
      float wtfy = (((ay) / SUBDIVIDE) * SUBDIVIDE);
      m_AtlasSprites[i]->SetPosition(wtfx, wtfy);
      if (ax > 0 & ay > 0) {
        annotate_index = m_Space->at((ax / SUBDIVIDE), (ay / SUBDIVIDE), 0);
      }
      if (m_AtlasSprites[i]->m_Frame != -annotate_index) {
        LOGV("changed: %d to %d\n", i, annotate_index);
      }
      m_AtlasSprites[i]->m_Frame = -annotate_index;
      if (print_index) {
        LOGV("%d %d | %d is %d @ %f %f  -- %f %f %f %f\n", ox, oy, i, m_AtlasSprites[i]->m_Frame,  m_AtlasSprites[i]->m_Position[0],  m_AtlasSprites[i]->m_Position[1], ax, ay, wtfx, wtfy);
      }
    }
  }
*/

  //m_CameraActualOffsetX += -(20.0 * m_DeltaTime * (m_CameraActualOffsetX - m_CameraOffsetX));
  //m_CameraActualOffsetY += -(20.0 * m_DeltaTime * (m_CameraActualOffsetY - m_CameraOffsetY));
  m_CameraActualOffsetX = m_CameraOffsetX;
  m_CameraActualOffsetY = m_CameraOffsetY;

  return 1;
}


void SuperStarShooter::CreateCollider(float x, float y, float r, int flag) {
  int sx = 0;
  int sy = 0;
  float l = 120 * (1.0 / 60.0);
  if (flag & BARREL) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 1, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  } else if (flag & STAR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 63, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  } else if (flag & MIRROR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 20, 21, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  }
  m_AtlasSprites[m_SpriteCount]->m_IsFlags = flag;
  m_AtlasSprites[m_SpriteCount]->SetPosition(x, y);
  m_AtlasSprites[m_SpriteCount]->m_Rotation = r;
  m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
  m_AtlasSprites[m_SpriteCount]->Build(1);
  sx = (x / SUBDIVIDE);
  sy = (y / SUBDIVIDE);
  int existing_index = m_Space->at(sx, sy, 0); 
  if (existing_index < 0) {
    m_Space->set(sx, sy, 0, m_SpriteCount);
  }
  m_SpriteCount++;
}


void SuperStarShooter::IndexToXY(int index, int* x, int* y) {
  *x = m_GridPositions[(index * 2)];
  *y = m_GridPositions[(index * 2) + 1];
}


int SuperStarShooter::XYToIndex(int x, int y) {
  return (y * GRID_X + x);
}
