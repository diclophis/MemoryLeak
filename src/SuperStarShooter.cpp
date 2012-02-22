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
#define GRID_X 17
#define GRID_Y 17
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 
#define BLANK 255


SuperStarShooter::SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {

  int xx = 0;
  int yy = 0;

  m_PercentThere = 0.0;

  m_Zoom = 1.0;

  LoadSound(0);

  m_IsPushingAudio = true;

	m_LastCenterX = m_CameraActualOffsetX = m_CameraStopOffsetX = m_CameraOffsetX = 0.0;
	m_LastCenterY = m_CameraActualOffsetY = m_CameraStopOffsetY = m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(32 * 32, BLANK);

  for (unsigned int i=0; i<100; i++) {
    for (unsigned int ii=0; ii<100; ii++) {
      m_Space->set(i, ii, 0, 51);
    }
  }

  //for (unsigned int i=0; i<100; i++) {
  //  m_Space->set(i, 0, 1, 56);
  //}

  /*
  for (unsigned int i=0; i<100; i++) {
    int col_top = 68;
    m_Space->set(i * 4, 7, 1, col_top);
    m_Space->set(i * 4, 6, 1, col_top + 16);
    m_Space->set(i * 4, 5, 1, col_top + 16 + 16);
    m_Space->set(i * 4, 4, 1, col_top + 16 + 16 + 16);
    m_Space->set(i * 4, 3, 0, col_top + 16 + 16 + 16 + 16);

    m_Space->set((i * 4) + 2, 5, 1, col_top);
    m_Space->set((i * 4) + 2, 4, 1, col_top + 16);
    m_Space->set((i * 4) + 2, 3, 1, col_top + 16 + 16);
    m_Space->set((i * 4) + 2, 2, 1, col_top + 16 + 16 + 16);
    m_Space->set((i * 4) + 2, 1, 0, col_top + 16 + 16 + 16 + 16);
  }
  */

  m_Space->set(6, 7, 0, 11);

  BlitIntoSpace(0, 41, 10, 3, 0, 0);

  int col_top = 68;
  m_Space->set(0 + 1, 7, 1, col_top);
  m_Space->set(0 + 1, 6, 1, col_top + 16);
  m_Space->set(0 + 1, 5, 1, col_top + 16 + 16);
  m_Space->set(0 + 1, 4, 1, col_top + 16 + 16 + 16);
  m_Space->set(0 + 1, 3, 0, col_top + 16 + 16 + 16 + 16);

  m_Space->set(0 + 3, 7, 1, col_top);
  m_Space->set(0 + 3, 6, 1, col_top + 16);
  m_Space->set(0 + 3, 5, 1, col_top + 16 + 16);
  m_Space->set(0 + 3, 4, 1, col_top + 16 + 16 + 16);
  m_Space->set(0 + 3, 3, 0, col_top + 16 + 16 + 16 + 16);

  m_Space->set(0 + 6, 7, 1, col_top);
  m_Space->set(0 + 6, 6, 1, col_top + 16);
  m_Space->set(0 + 6, 5, 1, col_top + 16 + 16);
  m_Space->set(0 + 6, 4, 1, col_top + 16 + 16 + 16);
  m_Space->set(0 + 6, 3, 0, col_top + 16 + 16 + 16 + 16);

  m_Space->set(0 + 8, 7, 1, col_top);
  m_Space->set(0 + 8, 6, 1, col_top + 16);
  m_Space->set(0 + 8, 5, 1, col_top + 16 + 16);
  m_Space->set(0 + 8, 4, 1, col_top + 16 + 16 + 16);
  m_Space->set(0 + 8, 3, 0, col_top + 16 + 16 + 16 + 16);

  BlitIntoSpace(1, 249, 10, 7, 0, 7);

  m_GridCount = (GRID_X * GRID_Y);
  m_GridPositions = (int *)malloc((m_GridCount * 2) * sizeof(int));
  m_GridStartIndex = m_SpriteCount;

  CreateFoos();

  for (unsigned int i=0; i<(m_GridCount * 2); i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_GridFoo, NULL));
  }

  for (unsigned int i=0; i<m_GridCount; i++) {

    float px = (xx * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
    float py = (yy * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);
    int sx = (int)floor(px / SUBDIVIDE);
    int sy = (int)floor(py / SUBDIVIDE);

    m_GridPositions[(i * 2)] = sx;
    m_GridPositions[(i * 2) + 1] = sy;

    m_AtlasSprites[m_SpriteCount]->SetPosition(px, py);
    m_AtlasSprites[m_SpriteCount + m_GridCount]->SetPosition(px, py);
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
    m_AtlasSprites[m_SpriteCount + m_GridCount]->m_IsAlive = false;
    m_AtlasSprites[m_SpriteCount]->m_Fps = 0;
    m_AtlasSprites[m_SpriteCount + m_GridCount]->m_Fps = 0;

    if (sx >= 0 && sy >= 0) {
      m_AtlasSprites[m_SpriteCount]->m_Frame = m_Space->at(sx, sy, 0);
      m_AtlasSprites[m_SpriteCount + m_GridCount]->m_Frame = m_Space->at(sx, sy, 1);
    } else {
      m_AtlasSprites[m_SpriteCount]->m_Frame = BLANK;
      m_AtlasSprites[m_SpriteCount + m_GridCount]->m_Frame = BLANK;
    }

    m_AtlasSprites[m_SpriteCount]->SetScale(SUBDIVIDE / 2.0, SUBDIVIDE / 2.0);
    m_AtlasSprites[m_SpriteCount + m_GridCount]->SetScale(SUBDIVIDE / 2.0, SUBDIVIDE / 2.0);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    m_AtlasSprites[m_SpriteCount + m_GridCount]->Build(0);

    xx++;
    if (xx >= GRID_X) {
      xx = 0;
      yy++;
    }
    m_SpriteCount++;
  }

  m_GridStopIndex = m_SpriteCount;
  m_SecondGridStartIndex = m_GridStopIndex;
  m_SecondGridStopIndex = m_GridStopIndex + m_GridCount;
  m_SpriteCount += m_GridCount;

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerFoo, NULL));
  m_AtlasSprites[m_PlayerIndex]->SetPosition(0.0, 130.0);
  m_AtlasSprites[m_PlayerIndex]->m_IsAlive = true;
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 6;
  m_AtlasSprites[m_PlayerIndex]->m_Frame = 0;
  m_AtlasSprites[m_PlayerIndex]->SetScale(25.0, 35.0);
  m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] = SUBDIVIDE;
  m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1] = 130.0;
  m_AtlasSprites[m_PlayerIndex]->Build(0);
  m_SpriteCount++;

  m_WarpTimeout = 0.0;
}


SuperStarShooter::~SuperStarShooter() {
  delete m_Space;
  delete m_GridPositions;
  DestroyFoos();
}


void SuperStarShooter::BlitIntoSpace(int layer, int bottom_right_start, int width, int height, int offset_x, int offset_y) {
  for (int fy = 0; fy < height; fy++) {
    for (int fx = (width - 1); fx >= 0; fx--) {
      LOGV("%d %d %d\n", fx + offset_x, fy + offset_y, bottom_right_start);
      m_Space->set(fx + offset_x, fy + offset_y, layer, bottom_right_start);
      bottom_right_start -= 1;
    }
    bottom_right_start -= (16);
    bottom_right_start += width;
  }
}


void SuperStarShooter::CreateFoos() {
  LOGV("SuperStarShooter::CreateFoos\n");
  ResetStateFoo();
  m_GridFoo = AtlasSprite::GetFoo(m_Textures->at(7), 16, 16, 0, 256, 0.0);
  m_PlayerFoo = AtlasSprite::GetFoo(m_Textures->at(7), 16, 14, 29, 32, 0.0);
  m_BatchFoo = AtlasSprite::GetBatchFoo(m_Textures->at(7), (m_GridCount * 2) + 1);
  if (m_SimulationTime > 0.0) {
    for (unsigned int i=m_GridStartIndex; i<m_SecondGridStopIndex; i++) {
      m_AtlasSprites[i]->ResetFoo(m_GridFoo, NULL);
    }
    m_AtlasSprites[m_PlayerIndex]->ResetFoo(m_PlayerFoo, NULL);
  }
}


void SuperStarShooter::DestroyFoos() {
  LOGV("SuperStarShooter::DestroyFoos\n");
  delete m_GridFoo;
  delete m_BatchFoo;
  delete m_PlayerFoo;
}


void SuperStarShooter::Hit(float x, float y, int hitState) {
  //LOGV("hit: %d\n", hitState);
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
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
  */
  if (hitState == 0) {
    m_CameraStopOffsetX = (xx + m_CameraOffsetX);
    m_CameraStopOffsetY = (yy + m_CameraOffsetY);
  }
  if (hitState == 1) {
    m_CameraOffsetX = (m_CameraStopOffsetX - xx);
    m_CameraOffsetY = (m_CameraStopOffsetY - yy);
  }
}


void SuperStarShooter::RenderModelPhase() {
}


void SuperStarShooter::RenderSpritePhase() {
  glTranslatef(-m_CameraActualOffsetX, -m_CameraActualOffsetY, 0.0);
  RenderSpriteRange(m_GridStartIndex, m_GridStopIndex, m_BatchFoo);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_BatchFoo);
  RenderSpriteRange(m_SecondGridStartIndex, m_SecondGridStopIndex, m_BatchFoo);
  AtlasSprite::RenderFoo(m_StateFoo, m_BatchFoo);
}


int SuperStarShooter::Simulate() {

  //m_CameraOffsetX += m_DeltaTime * 1000.0;
  //m_CameraOffsetY += m_DeltaTime * 20.0;
  //m_CameraOffsetX = fastSinf(m_SimulationTime * 1.5) * 400.0;
  //m_CameraOffsetY = fastSinf(m_SimulationTime * 3.0) * 400.0;

  float tx = (m_CameraActualOffsetX - m_CameraOffsetX);
  float ty = (m_CameraActualOffsetY - m_CameraOffsetY);
  float mx = (tx * m_DeltaTime * 5.0);
  float my = (ty * m_DeltaTime * 5.0);

  /*
  float length = mx * mx + my * my;
  float max = 1.0;
 
  if ((length > max * max) && (length > 0 )) {
    float ratio = max / sqrtf(length);
    mx *= ratio;
    my *= ratio;
  }
  LOGV("mx: %f\n", mx);
  */

  m_CameraActualOffsetX -= (mx);
  m_CameraActualOffsetY -= (my);

  bool recenter_x = false;
  bool recenter_y = false;
  int dsx = 0;
  int dsy = 0;
  float dx = (m_LastCenterX - m_CameraActualOffsetX);
  float dy = (m_LastCenterY - m_CameraActualOffsetY);

  if (fastAbs(dx) > (SUBDIVIDE)) {
    recenter_x = true;
  }
  
  if (fastAbs(dy) > (SUBDIVIDE)) {
    recenter_y = true;
  }

  dsx = (dx / SUBDIVIDE);
  dsy = (dy / SUBDIVIDE);

  if (recenter_x) {
    m_LastCenterX -= ((float)dsx * SUBDIVIDE);
  }

  if (recenter_y) {
    m_LastCenterY -= ((float)dsy * SUBDIVIDE);
  }

  int xx = 0;
  int yy = 0;

  if (true && (recenter_x || recenter_y)) {
    for (unsigned int i=m_GridStartIndex; i<m_GridStopIndex; i++) {
      int sx = -1;
      int sy = -1;
      int nsx = 0;
      int nsy = 0;
      IndexToXY(i - m_GridStartIndex, &sx, &sy);
      nsx = sx;
      nsy = sy;

      float px = (xx * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
      float py = (yy * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);

      if (recenter_x) {
        nsx -= dsx;
        m_AtlasSprites[i]->m_Position[0] = m_LastCenterX + px;
        m_AtlasSprites[i + m_GridCount]->m_Position[0] = m_LastCenterX + px;
      }
      if (recenter_y) {
        nsy -= dsy;
        m_AtlasSprites[i]->m_Position[1] = m_LastCenterY + py;
        m_AtlasSprites[i + m_GridCount]->m_Position[1] = m_LastCenterY + py;
      }
      m_GridPositions[(i * 2)] = nsx;
      m_GridPositions[(i * 2) + 1] = nsy;
      if (nsx >= 0 && nsy >= 0) {
        m_AtlasSprites[i]->m_Frame = m_Space->at(nsx, nsy, 0);
        m_AtlasSprites[i + m_GridCount]->m_Frame = m_Space->at(nsx, nsy, 1);
      } else {
        m_AtlasSprites[i]->m_Frame = BLANK;
        m_AtlasSprites[i + m_GridCount]->m_Frame = BLANK;
      }
      xx++;
      if (xx >= GRID_X) {
        xx = 0;
        yy++;
      }
    }
  }

  if (m_AtlasSprites[m_PlayerIndex]->MoveToTargetPosition(m_DeltaTime)) {
    m_WarpTimeout += m_DeltaTime;
    if (m_WarpTimeout > 0.5) {
      m_WarpTimeout = 0.0;
      m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] += SUBDIVIDE;
    }
  } else {
    m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);
  }

  return 1;
}


void SuperStarShooter::CreateCollider(float x, float y, float r, int flag) {
  int sx = 0;
  int sy = 0;
  /*
  float l = 120 * (1.0 / 60.0);
  if (flag & BARREL) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 1, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  } else if (flag & STAR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 0, 63, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  } else if (flag & MIRROR) {
    m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 20, 21, l * 0.5, "", 8, 11, l, 100.0, 100.0));
  }
  */
  if (flag & BARREL) {
    m_AtlasSprites.push_back(new SpriteGun(NULL, NULL));
  } else if (flag & STAR) {
    m_AtlasSprites.push_back(new SpriteGun(NULL, NULL));
  } else if (flag & MIRROR) {
    m_AtlasSprites.push_back(new SpriteGun(NULL, NULL));
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
