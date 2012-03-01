// Jon Bardin GPL


enum colliders {
  BARREL = 1,
  MIRROR = 2,
  STAR = 4
};


#include "MemoryLeak.h"
#include "SuperStarShooter.h"


#define SUBDIVIDE 50.0
#define BARREL_ROTATE_TIMEOUT 0.33
#define BARREL_ROTATE_PER_TICK 0 
#define SHOOT_VELOCITY 425.0
#define GRID_X 24
#define GRID_Y 24
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 
#define BLANK 255
#define WALK 51
#define PLAYER_OFFSET 25.0


SuperStarShooter::SuperStarShooter(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {

  int xx = 0;
  int yy = 0;

  m_PercentThere = 0.0;

  m_Zoom = 1.0;

  LoadSound(0);

  m_IsPushingAudio = true;

	m_TouchStartX = m_LastCenterX = m_CameraActualOffsetX = m_CameraStopOffsetX = m_CameraOffsetX = 0.0;
	m_TouchStartY = m_LastCenterY = m_CameraActualOffsetY = m_CameraStopOffsetY = m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(32 * 32, BLANK);

  for (unsigned int i=0; i<100; i++) {
    for (unsigned int ii=0; ii<100; ii++) {
      m_Space->set(i, ii, 0, WALK);
    }
  }

  m_PlayerFoos = (foofoo **)malloc(sizeof(foofoo *) * 4);

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

  //m_Space->set(6, 7, 0, 11);

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

  for (int i=0; i<(m_GridCount * 2); i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_GridFoo, NULL));
  }

  for (int i=0; i<m_GridCount; i++) {

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

  m_PlayerStartIndex = m_SpriteCount;
  for (unsigned int i=0; i<4; i++) {
    int sub_index = m_SpriteCount;
    m_PlayerIndex = sub_index;
    m_AtlasSprites.push_back(new SpriteGun(m_PlayerFoos[i], NULL));
    m_AtlasSprites[sub_index]->SetPosition(0.0, PLAYER_OFFSET);
    m_AtlasSprites[sub_index]->m_IsAlive = true;
    m_AtlasSprites[sub_index]->m_Fps = 6;
    m_AtlasSprites[sub_index]->m_Frame = 0;
    m_AtlasSprites[sub_index]->SetScale(25.0, 35.0);
    m_AtlasSprites[sub_index]->m_TargetPosition[0] = 0.0;
    m_AtlasSprites[sub_index]->m_TargetPosition[1] = PLAYER_OFFSET;
    m_AtlasSprites[sub_index]->Build(0);
    m_SpriteCount++;
  }
  m_PlayerStopIndex = m_SpriteCount;

  m_HoleIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_HoleFoo, NULL));
  m_AtlasSprites[m_HoleIndex]->SetPosition(150.0, 500.0);
  m_AtlasSprites[m_HoleIndex]->m_Frame = 0;
  m_AtlasSprites[m_HoleIndex]->SetScale(50.0, 50.0);
  m_AtlasSprites[m_HoleIndex]->Build(0);
  m_SpriteCount++;

  m_WarpTimeout = 0.0;

	//m_ModelOctree = new micropather::ModelOctree(*m_Space);

	m_Pather = new micropather::MicroPather(this);
	m_Steps = new std::vector<void *>;

  //startState = (nodexyz *)malloc(1 * sizeof(nodexyz));
  //endState = (nodexyz *)malloc(1 * sizeof(nodexyz));
  m_StatePointer = 0;
  for (unsigned int i=0; i<256; i++) {
    m_States.push_back(new nodexyz());
  }

  m_TargetX = -1;
  m_TargetY = -1;
}


SuperStarShooter::~SuperStarShooter() {
  delete m_Space;
  delete m_GridPositions;
  DestroyFoos();
}


void SuperStarShooter::BlitIntoSpace(int layer, int bottom_right_start, int width, int height, int offset_x, int offset_y) {
  for (int fy = 0; fy < height; fy++) {
    for (int fx = (width - 1); fx >= 0; fx--) {
      //LOGV("%d %d %d\n", fx + offset_x, fy + offset_y, bottom_right_start);
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

  for (unsigned int i=0; i<4; i++) {
    m_PlayerFoos[i] = AtlasSprite::GetFoo(m_Textures->at(7), 16, 14, 13 + (16 * i), 13 + (16 * i) + 3, 0.0);
  }

  m_HoleFoo = AtlasSprite::GetFoo(m_Textures->at(7), 8, 8, 24, 25, 0.0);
  m_BatchFoo = AtlasSprite::GetBatchFoo(m_Textures->at(7), (m_GridCount * 2) + 2);
  if (m_SimulationTime > 0.0) {
    for (int i=m_GridStartIndex; i<m_SecondGridStopIndex; i++) {
      m_AtlasSprites[i]->ResetFoo(m_GridFoo, NULL);
    }
    //m_AtlasSprites[m_PlayerIndex]->ResetFoo(m_PlayerFoo, NULL);
  }
}


void SuperStarShooter::DestroyFoos() {
  LOGV("SuperStarShooter::DestroyFoos\n");
  delete m_GridFoo;
  delete m_BatchFoo;
  //delete m_PlayerFoo;
}


void SuperStarShooter::Hit(float x, float y, int hitState) {
  //LOGV("hit: %d\n", hitState);
	
  
  float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
  float dx = (xx + m_CameraOffsetX) + (SUBDIVIDE / 2.0);
  float dy = (yy + m_CameraOffsetY) + (SUBDIVIDE / 2.0);
	float collide_x = (dx);
	float collide_y = (dy);
  int cx = (collide_x / SUBDIVIDE);
  int cy = (collide_y / SUBDIVIDE);
  int collide_index = -1;
  bool collide_index_set = false;
  if (cx >= 0 && cy >= 0) {
    collide_index_set = true;
    collide_index = m_Space->at(cx, cy, 0);
  }

  if (collide_index_set && m_PlayerCanMove) {
    m_TargetX = cx;
    m_TargetY = cy;
  }


  /*
  if (hitState == 0) {
    m_CameraStopOffsetX = (xx + m_CameraOffsetX);
    m_CameraStopOffsetY = (yy + m_CameraOffsetY);
  }
  if (hitState == 1) {
    m_CameraOffsetX = (m_CameraStopOffsetX - xx);
    m_CameraOffsetY = (m_CameraStopOffsetY - yy);
  }
  */

  /*
  if (m_PlayerCanMove) {
    if (hitState == 0) {
      m_TouchStartX = (x);
      m_TouchStartY = (y);
    }

    if (hitState == 2) {
      float dx = x - m_TouchStartX;
      float dy = y - m_TouchStartY;
      if (fastAbs(dx) > fastAbs(dy)) {
        //LOGV("left right\n");
        if (dx > 0) {
          m_PlayerIndex = m_PlayerStartIndex + 1;
          m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] += SUBDIVIDE;
        } else {
          m_PlayerIndex = m_PlayerStartIndex + 3;
          m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] -= SUBDIVIDE;
        }
      } else {
        //LOGV("up down\n");
        if (dy < 0) {
          m_PlayerIndex = m_PlayerStartIndex + 0;
          m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1] += SUBDIVIDE;
        } else {
          m_PlayerIndex = m_PlayerStartIndex + 2;
          m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1] -= SUBDIVIDE;
        }
      }
    }
  }
  */
}


void SuperStarShooter::RenderModelPhase() {
}


void SuperStarShooter::RenderSpritePhase() {
  glTranslatef(-m_CameraActualOffsetX, -m_CameraActualOffsetY, 0.0);
  RenderSpriteRange(m_GridStartIndex, m_GridStopIndex, m_BatchFoo);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_BatchFoo);
  RenderSpriteRange(m_SecondGridStartIndex, m_SecondGridStopIndex, m_BatchFoo);
  RenderSpriteRange(m_HoleIndex, m_HoleIndex + 1, m_BatchFoo);
  AtlasSprite::RenderFoo(m_StateFoo, m_BatchFoo);
}


int SuperStarShooter::Simulate() {

  m_CameraOffsetX = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
  m_CameraOffsetY = m_AtlasSprites[m_PlayerIndex]->m_Position[1];

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
    for (int i=m_GridStartIndex; i<m_GridStopIndex; i++) {
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
    for (int i=0; i<4; i++) {
      if ((m_PlayerStartIndex + i) != m_PlayerIndex) {
        m_AtlasSprites[m_PlayerStartIndex + i]->m_Position[0] = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
        m_AtlasSprites[m_PlayerStartIndex + i]->m_Position[1] = m_AtlasSprites[m_PlayerIndex]->m_Position[1];
        m_AtlasSprites[m_PlayerStartIndex + i]->m_TargetPosition[0] = m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0];
        m_AtlasSprites[m_PlayerStartIndex + i]->m_TargetPosition[1] = m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1];
      }
    }

    m_PlayerCanMove = true;

    if (m_TargetX >= 0 && m_TargetY >= 0) {
      m_StatePointer = 0;

      int startState = StatePointerFor((m_AtlasSprites[m_PlayerIndex]->m_Position[0] / SUBDIVIDE), (m_AtlasSprites[m_PlayerIndex]->m_Position[1] / SUBDIVIDE), 0);
      int endState = StatePointerFor(m_TargetX, m_TargetY, 0);

      LOGV("tx: %d ty: %d\n", m_TargetX, m_TargetY);
     
      float totalCost;
      m_Pather->Reset();
      int solved = m_Pather->Solve((void *)startState, (void *)endState, m_Steps, &totalCost);
      switch (solved) {
        case micropather::MicroPather::SOLVED:
          //LOGV("solved\n");
          break;
        case micropather::MicroPather::NO_SOLUTION:
          //LOGV("none\n");
          break;
        case micropather::MicroPather::START_END_SAME:
          //LOGV("same\n");
          m_TargetX = -1;
          m_TargetY = -1;
          break;	
        default:
          break;
      }

      if (m_Steps->size() > 1) {
        nodexyz *step = m_States[(intptr_t)m_Steps->at(1)];
        float tx = ((float)step->x * SUBDIVIDE);
        float ty = ((float)step->y * SUBDIVIDE) + PLAYER_OFFSET;
        float dx = tx - m_AtlasSprites[m_PlayerIndex]->m_Position[0];
        float dy = ty - m_AtlasSprites[m_PlayerIndex]->m_Position[1];

        m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] = tx;
        m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1] = ty;

        m_Steps->erase(m_Steps->begin());

        if (fastAbs(dx) > fastAbs(dy)) {
          //LOGV("left right\n");
          if (dx > 0) {
            m_PlayerIndex = m_PlayerStartIndex + 1;
          } else {
            m_PlayerIndex = m_PlayerStartIndex + 3;
          }
        } else {
          //LOGV("up down\n");
          if (dy > 0) {
            m_PlayerIndex = m_PlayerStartIndex + 0;
          } else {
            m_PlayerIndex = m_PlayerStartIndex + 2;
          }
        }
      }
    }
  } else {
    m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);
    m_PlayerCanMove = false;
  }

  return 1;
}


void SuperStarShooter::CreateCollider(float x, float y, float r, int flag) {
  int sx = 0;
  int sy = 0;
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


float SuperStarShooter::LeastCostEstimate(void *nodeStart, void *nodeEnd) {	

  int xStart = m_States[((intptr_t)nodeStart)]->x;
  int yStart = m_States[((intptr_t)nodeStart)]->y;

  int xEnd = m_States[((intptr_t)nodeEnd)]->x;
  int yEnd = m_States[((intptr_t)nodeEnd)]->y;

  int dx = xStart - xEnd;
  int dy = yStart - yEnd;
  int dz = 0;

  float least_cost = (float) sqrt( (double)(dx * dx) + (double)(dy * dy) + (double)(dz * dz));

  return least_cost;
}


void SuperStarShooter::AdjacentCost(void *node, std::vector<micropather::StateCost> *neighbors) {

  int ax = m_States[((intptr_t)node)]->x;
  int ay = m_States[((intptr_t)node)]->y;

  const int dx[8] = { 1, 0, -1, 0};
  const int dy[8] = { 0, 1, 0, -1};
  
  int lx = m_States[0]->x - ax;
  int ly = m_States[0]->y - ay;
  int lz = 0;

  float look_distance = (float)sqrt((double)(lx * lx) + (double)(ly * ly) + (double)(lz * lz));
  
  if (look_distance > 20.0) {
    LOGV("look: %f\n", look_distance);
    return;
  }
  
  float pass_cost = 0;
  
  for( int i=0; i<4; ++i ) {
    int nx = ax + dx[i];
    int ny = ay + dy[i];	
    bool passable = false;
    if (nx >= 0 && ny >= 0) {
      int colliding_index = m_Space->at(nx, ny, 0);
      //LOGV("%d\n", colliding_index);
      if (colliding_index != (68 + 16 + 16 + 16 + 16)) {
        if (nx == m_States[1]->x && ny == m_States[1]->y) {
          passable = true;
          pass_cost = 0.0;
        } else {
          passable = true;
          pass_cost = 100.0;
        }
      }
    }
    
    if (passable) {
      micropather::StateCost nodeCost = {
        (void *)StatePointerFor(nx, ny, 0), pass_cost
      };
      neighbors->push_back(nodeCost);
    }
  }
}


int SuperStarShooter::StatePointerFor(int x, int y, int z) {
  bool exists = false;
  int found = -1;
  for (int i=0; i<m_StatePointer; i++) {
    if (m_States[i]->x == x && m_States[i]->y == y) {
      found = i;
      exists = true;
      break;
    }
  }
  
  if (!exists) {
    found = m_StatePointer;
    m_States[m_StatePointer]->x = x;
    m_States[m_StatePointer]->y = y;
    m_StatePointer++;
  }

  return found;
}
