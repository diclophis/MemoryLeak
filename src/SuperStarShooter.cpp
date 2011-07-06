// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SuperStarShooter.h"


enum colliders {
  BARREL = 1,
  MIRROR = 2,
  STAR = 4
};


#define SUBDIVIDE 50.0
#define BARREL_ROTATE_TIMEOUT 0.33
#define BARREL_ROTATE_PER_TICK 0 
#define SHOOT_VELOCITY 425.0
#define GRID_X 13 
#define GRID_Y 13 
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 


SuperStarShooter::SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("super star shooter alloc\n");


  int xx = 0;
  int yy = 0;

  m_Zoom = 1.0;

  LoadSound(1);
  m_IsPushingAudio = true;

	m_CameraOffsetX = 0.0;
	m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(16 * 16, -63);

  CreateCollider(SUBDIVIDE * 2, SUBDIVIDE * 2, 0.0, STAR);
  
  m_GridCount = GRID_X * GRID_Y;
  m_GridPositions = new int[m_GridCount * 2];
  m_GridStartIndex = m_SpriteCount;

  LOGV("super star shooter alloc 123\n");

  for (unsigned int i=0; i<m_GridCount; i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 64, 1.0, "", 0, 64, 0.0, SUBDIVIDE, SUBDIVIDE));
    m_AtlasSprites[m_SpriteCount]->Build(0);
    m_GridPositions[(i * 2)] = xx;
    m_GridPositions[(i * 2) + 1] = yy;
    xx++;
    if (xx >= GRID_X) {
      xx = 0;
      yy++;
    }
    m_SpriteCount++;
  }
  m_GridStopIndex = m_SpriteCount;

  LOGV("super star shooter alloc 456\n");

}


SuperStarShooter::~SuperStarShooter() {
  LOGV("super star shooter dealloc\n");
}


void SuperStarShooter::Hit(float x, float y, int hitState) {
  LOGV("super star shooter hit\n");

	float xx = (x - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - y) * m_Zoom;
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
  //if (hitState == 0 && collide_index >= 0 && collide_index != m_CurrentBarrelIndex) {
  m_CameraOffsetX = -xx;
  m_CameraOffsetY = -yy;

}


void SuperStarShooter::RenderModelPhase() {
  LOGV("super star shooter render model\n");
}


void SuperStarShooter::RenderSpritePhase() {
  LOGV("super star shooter render sprite\n");

  glTranslatef(-m_CameraOffsetX, -m_CameraOffsetY, 0.0);
  
  AtlasSprite::Scrub();
  RenderSpriteRange(m_GridStartIndex, m_GridStopIndex);
  
  AtlasSprite::Scrub();
  RenderSpriteRange(0, m_GridStartIndex);

}


int SuperStarShooter::Simulate() {
  LOGV("super star shooter sim 123\n");

  //PushMessageToWebView(CreateWebViewFunction("hide()"));

  for (unsigned int i=0; i<m_SpriteCount; i++) {
    LOGV("super star shooter sim 456\n");

    m_AtlasSprites[i]->Simulate(m_DeltaTime);
    if (i >= m_GridStartIndex && i <= m_GridStopIndex) {
      int annotate_index = 0;
      int ox = -1;
      int oy = -1;
      IndexToXY(i - m_GridStartIndex, &ox, &oy);
      float ax = (ox * SUBDIVIDE) + (m_CameraOffsetX) - (((GRID_X - 1) / 2) * SUBDIVIDE);
      float ay = (oy * SUBDIVIDE) + (m_CameraOffsetY) - (((GRID_Y - 1) / 2) * SUBDIVIDE);
      float wtfx = (int)((ax) / SUBDIVIDE) * SUBDIVIDE;
      float wtfy = (int)((ay) / SUBDIVIDE) * SUBDIVIDE;
      m_AtlasSprites[i]->SetPosition(wtfx, wtfy);
      if (ax > 0 & ay > 0) {
        annotate_index = m_Space->at((ax / SUBDIVIDE), (ay / SUBDIVIDE), 0);
      }
      m_AtlasSprites[i]->m_Frame = -annotate_index;
    }
  }

  LOGV("super star shooter sim 789\n");

  return 1;
}


void SuperStarShooter::CreateCollider(float x, float y, float r, int flag) {
  int sx = 0;
  int sy = 0;
  float l = 120 * (1.0 / 60.0);

  if (flag & BARREL) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 1, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  } else if (flag & STAR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 43, 44, l * 0.5, "", 8, 11, l, 100.0, 100.0));
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
