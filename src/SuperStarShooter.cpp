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
#define GRID_X 7
#define GRID_Y 7
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 


SuperStarShooter::SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {

  int xx = 0;
  int yy = 0;

  m_DebugTimeout = 0.0;

  m_PercentThere = 0.0;

  m_Zoom = 1.0;

  LoadSound(3);

  m_IsPushingAudio = true;

	m_LastCenterX = m_CameraActualOffsetX = m_CameraStopOffsetX = m_CameraOffsetX = 0.0;
	m_LastCenterY = m_CameraActualOffsetY = m_CameraStopOffsetY = m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(32 * 32, -1);
  m_Space->set(0, 0, 0, -3);
  m_Space->set(0, 1, 0, -3);
  m_Space->set(1, 0, 0, -3);
  m_Space->set(1, 1, 0, -32);
  m_Space->set(2, 2, 0, -40);
  for (unsigned int i=3; i<30; i++) {
    m_Space->set(i, i, 0, -40);
  }
  
  m_GridCount = (GRID_X * GRID_Y);
  m_GridPositions = (int *)malloc((m_GridCount * 2) * sizeof(int));
  m_GridStartIndex = m_SpriteCount;

  CreateFoos();

  for (unsigned int i=0; i<m_GridCount; i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_GridFoo, NULL));
    m_GridPositions[(i * 2)] = xx;
    m_GridPositions[(i * 2) + 1] = yy;
    m_AtlasSprites[m_SpriteCount]->SetPosition((xx * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE), (yy * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE));
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
    m_AtlasSprites[m_SpriteCount]->m_Fps = 0;
    m_AtlasSprites[m_SpriteCount]->m_Frame = 1;
    m_AtlasSprites[m_SpriteCount]->SetScale(SUBDIVIDE / 2.0, SUBDIVIDE / 2.0);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    xx++;
    if (xx >= GRID_X) {
      xx = 0;
      yy++;
    }
    m_SpriteCount++;
  }
  m_GridStopIndex = m_SpriteCount;

  m_WarpTimeout = 0.0;
  m_Bumped = false;
  m_Bump = 0.0;
}


SuperStarShooter::~SuperStarShooter() {
  delete m_Space;
  delete m_GridPositions;
  DestroyFoos();
}


void SuperStarShooter::CreateFoos() {
  LOGV("SuperStarShooter::CreateFoos\n");
  ResetStateFoo();
  m_GridFoo = AtlasSprite::GetFoo(m_Textures->at(0), 8, 8, 0, 64, 0.0);
  m_BatchFoo = AtlasSprite::GetBatchFoo(m_Textures->at(0), m_GridCount);
  if (m_SimulationTime > 0.0) {
    for (unsigned int i=0; i<m_SpriteCount; i++) {
      m_AtlasSprites[i]->ResetFoo(m_GridFoo, NULL);
    }
  }
}


void SuperStarShooter::DestroyFoos() {
  LOGV("SuperStarShooter::DestroyFoos\n");
  delete m_GridFoo;
  delete m_BatchFoo;
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
    m_CameraOffsetX = m_CameraStopOffsetX - xx;
    m_CameraOffsetY = m_CameraStopOffsetY -yy;
  }
}


void SuperStarShooter::RenderModelPhase() {
}


void SuperStarShooter::RenderSpritePhase() {
  glTranslatef(-m_CameraActualOffsetX, -m_CameraActualOffsetY, 0.0);
  RenderSpriteRange(m_GridStartIndex, m_GridStopIndex, m_BatchFoo);
  AtlasSprite::RenderFoo(m_StateFoo, m_BatchFoo);
}


int SuperStarShooter::Simulate() {

  //if (m_Bumped) {
  //  m_Bumped = false;
  //  m_Bump += 1.0;
  //}

  m_CameraActualOffsetX = (m_CameraOffsetX);
  m_CameraActualOffsetY = (m_CameraOffsetY);

  //m_CameraOffsetY -= m_DeltaTime * 5.0;
  //m_CameraOffsetX += m_DeltaTime * 600.0;

  
  //bool print_index = false;
  //if (m_DebugTimeout > 2.0) {
  //  m_DebugTimeout = 0.0;
  //  print_index = true;
  //}

  //m_DebugTimeout += m_DeltaTime;

  bool recenter_x = false;
  bool recenter_y = false;
  //float dir_x = 0.0;
  //float dir_y = 0.0;
  //float recenter_dx = 0.0;
  //float recenter_dy = 0.0;
  float dx = (m_LastCenterX - m_CameraActualOffsetX);
  float dy = (m_LastCenterY - m_CameraActualOffsetY);
  
  if ((dx) >= (SUBDIVIDE)) {
    //dir_x = floorf(dx / SUBDIVIDE);
    //recenter_dx = dx - SUBDIVIDE;
    recenter_x = true;
  }
  
  if ((dy) >= (SUBDIVIDE)) {
    //dir_y = floorf(dy / SUBDIVIDE);
    //recenter_dy = dy - SUBDIVIDE;
    recenter_y = true;
  }
  
  if ((dx) <= (-SUBDIVIDE)) {
    //dir_x = -floorf(-dx / SUBDIVIDE);
    //recenter_dx = -(-dx - SUBDIVIDE);
    recenter_x = true;
  }
  
  if ((dy) <= (-SUBDIVIDE)) {
    //dir_y = -floorf(-dy / SUBDIVIDE);
    //recenter_dy = -(-dy - SUBDIVIDE);
    recenter_y = true;
  }

  if (recenter_x) {
    m_LastCenterX = m_CameraActualOffsetX;     
  }

  if (recenter_y) {
    m_LastCenterY = m_CameraActualOffsetY;
  }

  for (unsigned int i=0; i<m_SpriteCount; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
    if (i >= m_GridStartIndex && i <= m_GridStopIndex) {


      int annotate_index = -12;
      int gx = -1;
      int gy = -1;
      IndexToXY(i - m_GridStartIndex, &gx, &gy);

      float nx = m_LastCenterX + (gx * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
      float ny = m_LastCenterY + (gy * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);

      if (recenter_x) {
        //m_AtlasSprites[i]->SetPosition(m_AtlasSprites[i]->m_Position[0] - dx, m_AtlasSprites[i]->m_Position[1]);
        m_AtlasSprites[i]->SetPosition(nx, m_AtlasSprites[i]->m_Position[1]);
      }
      
      if (recenter_y) {
        //m_AtlasSprites[i]->SetPosition(m_AtlasSprites[i]->m_Position[0], m_AtlasSprites[i]->m_Position[1] - dy);
        m_AtlasSprites[i]->SetPosition(m_AtlasSprites[i]->m_Position[0], ny);
      }

      //m_AtlasSprites[i]->SetPosition(nx, ny);

      float px = m_AtlasSprites[i]->m_Position[0];
      //m_CameraActualOffsetX + (gx * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
      float py = m_AtlasSprites[i]->m_Position[1];
      //m_CameraActualOffsetY + (gy * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);
      int sx = floor(px / SUBDIVIDE);
      int sy = floor(py / SUBDIVIDE);
      if (sx >= 0 && sy >= 0) {
        annotate_index = m_Space->at(sx, sy, 0);
      }
      m_AtlasSprites[i]->m_Frame = -annotate_index;

      /*
      //if (print_index) {
      if (i == 4 || i == 7) {
        if (recenter_y || recenter_x) {
          LOGV("%d %f %f %d %d %f\n", recenter_y, dy, dir_y, i, m_AtlasSprites[i]->m_Frame, recenter_dy);
        }
        if (m_AtlasSprites[i]->m_Frame == 12) {
          //sleep(2);
        }
      }
      //}
      */
    }
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
