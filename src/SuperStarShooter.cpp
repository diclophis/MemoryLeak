// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SuperStarShooter.h"


#define ZOOM (1.0)
#define SUBDIVIDE (32.0) 
#define BARREL_ROTATE_TIMEOUT 0.33
#define BARREL_ROTATE_PER_TICK 0 
#define SHOOT_VELOCITY 425.0
//#define GRID_X 24
//#define GRID_Y 34
#define GRID_X (58/2)
#define GRID_Y (74/2)
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 
#define BLANK 255
#define TREASURE 10
#define PURE 97
#define SAND 98
#define FILL BLANK
#define OVER BLANK
#define PLAYER_OFFSET (SUBDIVIDE * 0.5) 
#define VELOCITY 1000.0
#define MAX_WAIT_BEFORE_WARP 0.016
#define MAX_SEARCH 15
#define MAX_STATE_POINTERS 1024


// Each cell in the maze is a bitfield. The bits that are set indicate which
// passages exist leading AWAY from this cell. Bits in the low byte (corresponding
// to the PRIMARY bitmask) represent passages on the normal plane. Bits
// in the high byte (corresponding to the UNDER bitmask) represent passages
// that are passing under this cell.
#define N  = 0x01 // North
#define S  = 0x02 // South
#define E  = 0x04 // East
#define W  = 0x08 // West
#define NW = 0x10 // Northwest
#define NE = 0x20 // Northeast
#define SW = 0x40 // Southwest
#define SE = 0x80 // Southeast

// bitmask identifying directional bits on the primary plane
#define PRIMARY 0x000000FF

// bitmask identifying directional bits under the primary plane
#define UNDER 0x0000FF00

// bits reserved for use by individual algorithm implementations
#define RESERVED 0xFFFF0000

// The size of the PRIMARY bitmask (e.g. how far to the left the UNDER bitmask is shifted).
#define UNDER_SHIFT 8


SuperStarShooter::SuperStarShooter(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {

  m_NeedsTerrainRebatched = true;

  LoadTexture(0);

  m_CenterOfWorldX = 4;
  m_CenterOfWorldY = 4;

  int xx = 0;
  int yy = 0;

  m_Zoom = ZOOM; //0.4321;

	m_TouchStartX = m_LastCenterX = m_CameraActualOffsetX = m_CameraStopOffsetX = m_CameraOffsetX = 0.0;
	m_TouchStartY = m_LastCenterY = m_CameraActualOffsetY = m_CameraStopOffsetY = m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(2048, BLANK);

  if (false) {
    for (unsigned int i=0; i<124; i++) {
      for (unsigned int ii=0; ii<124; ii++) {
        m_Space->set(i, ii, 0, FILL);
      }
    }
  }

  m_PlayerFoos = (foofoo **)malloc(sizeof(foofoo *) * 4);

  if (false) {
    //int layer, int bottom_right_start, int width, int height, int offset_x, int offset_y
    int bt_x = m_CenterOfWorldX;
    int bt_y = m_CenterOfWorldY;
    int col_top = 68;
    int col_bottom_start = col_top + (16 * 4);
    int col_top_start = col_top + (16 * 3);

    //stairs
    BlitIntoSpace(0, 41, 10, 3, bt_x, bt_y);

    //cols
    BlitIntoSpace(0, col_bottom_start, 1, 1, bt_x + 1, bt_y + 3);
    BlitIntoSpace(1, col_top_start, 1, 4, bt_x + 1, bt_y + 4);
    BlitIntoSpace(0, col_bottom_start, 1, 1, bt_x + 3, bt_y + 3);
    BlitIntoSpace(1, col_top_start, 1, 4, bt_x + 3, bt_y + 4);
    BlitIntoSpace(0, col_bottom_start, 1, 1, bt_x + 6, bt_y + 3);
    BlitIntoSpace(1, col_top_start, 1, 4, bt_x + 6, bt_y + 4);
    BlitIntoSpace(0, col_bottom_start, 1, 1, bt_x + 8, bt_y + 3);
    BlitIntoSpace(1, col_top_start, 1, 4, bt_x + 8, bt_y + 4);

    //roof
    BlitIntoSpace(1, 249, 10, 7, bt_x, bt_y + 7);
  }

  m_GridCount = (GRID_X * GRID_Y);
  m_GridPositions = (int *)malloc((m_GridCount * 2) * sizeof(int));
  m_GridStartIndex = m_SpriteCount;

  m_TrailCount = MAX_SEARCH * 2;

  LoadMaze(1);
  LoadSound(2);
  //LoadSound(1);
  CreateFoos();

  for (int i=0; i<(m_GridCount * 2); i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_GridFoo, NULL));
  }

  for (int i=0; i<m_GridCount; i++) {

    float px = ((xx + m_CenterOfWorldX) * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
    float py = ((yy + m_CenterOfWorldY) * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);
    int sx = (int)(px / SUBDIVIDE);
    int sy = (int)(py / SUBDIVIDE);

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
    m_AtlasSprites[sub_index]->SetVelocity(VELOCITY, VELOCITY);
    m_AtlasSprites[sub_index]->SetPosition((m_CenterOfWorldX * (SUBDIVIDE)), (m_CenterOfWorldY * (SUBDIVIDE)) + PLAYER_OFFSET);
    m_AtlasSprites[sub_index]->m_IsAlive = true;
    m_AtlasSprites[sub_index]->m_Fps = 20;
    m_AtlasSprites[sub_index]->m_Frame = 0;
    m_AtlasSprites[sub_index]->SetScale((SUBDIVIDE / 2.0) * 1.0, ((SUBDIVIDE / 2.0) + ((1.0 / 5.0) * SUBDIVIDE)) * 1.0);
    m_AtlasSprites[sub_index]->m_TargetPosition[0] = m_AtlasSprites[sub_index]->m_Position[0];
    m_AtlasSprites[sub_index]->m_TargetPosition[1] = m_AtlasSprites[sub_index]->m_Position[1];
    m_AtlasSprites[sub_index]->Build(0);
    m_SpriteCount++;
  }
  m_PlayerStopIndex = m_SpriteCount;

  /*
  m_HoleIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_HoleFoo, NULL));
  m_AtlasSprites[m_HoleIndex]->SetPosition(0.0, 0.0);
  m_AtlasSprites[m_HoleIndex]->m_Frame = 0;
  m_AtlasSprites[m_HoleIndex]->SetScale(50.0, 50.0);
  m_AtlasSprites[m_HoleIndex]->Build(0);
  m_SpriteCount++;
  */

  m_WarpTimeout = 0.0;

	m_Pather = new micropather::MicroPather(this);
	m_Steps = new std::vector<void *>;

  m_MaxStatePointers = MAX_STATE_POINTERS;
  m_StatePointer = 0;
  for (int i=0; i<m_MaxStatePointers; i++) {
    m_States.push_back(new nodexyz());
  }

  m_TargetX = 4;
  m_TargetY = 4;

  m_TargetIsDirty = true;

  m_GotLastSwipeAt = -10.0;
  m_SwipedBeforeUp = false;

  m_TrailStartIndex = m_SpriteCount;
  for (unsigned int i=0; i<m_TrailCount; i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_TrailFoo, NULL));
    m_AtlasSprites[m_SpriteCount]->SetPosition(0.0, 0.0);
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
    m_AtlasSprites[m_SpriteCount]->m_Fps = 0; 
    m_AtlasSprites[m_SpriteCount]->SetScale(SUBDIVIDE / 2.5, SUBDIVIDE / 2.5);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    m_AtlasSprites[m_SpriteCount]->m_Rotation = i * 20;
    m_SpriteCount++;
  }
  m_TrailStopIndex = m_SpriteCount;

  m_CameraOffsetX = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
  m_CameraOffsetY = m_AtlasSprites[m_PlayerIndex]->m_Position[1];

}


SuperStarShooter::~SuperStarShooter() {
  delete m_Space;
  delete m_GridPositions;
  for (std::vector<nodexyz *>::iterator i = m_States.begin(); i != m_States.end(); ++i) {
    delete *i;
  }
  m_States.clear();
  m_Steps->clear();
  delete m_Steps;
  delete m_Pather;
  DestroyFoos();
}


void SuperStarShooter::BlitIntoSpace(int layer, int bottom_right_start, int width, int height, int offset_x, int offset_y) {
  for (int fy = 0; fy < height; fy++) {
    for (int fx = (width - 1); fx >= 0; fx--) {
      //LOGV("wtf: %d %d\n", fx + offset_x, fy + offset_y);
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

  m_GridFoo = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 0, 256, 0.0);

  for (unsigned int i=0; i<4; i++) {
    m_PlayerFoos[i] = AtlasSprite::GetFoo(m_Textures.at(0), 16, 14, 13 + (16 * i), 13 + (16 * i) + 3, 0.0);
  }

  m_HoleFoo = AtlasSprite::GetFoo(m_Textures.at(0), 8, 8, 24, 25, 0.0);
  m_TrailFoo = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 251, 256, 0.0);

  //m_BatchFoo = AtlasSprite::GetBatchFoo(m_Textures.at(0), (m_GridCount * 2) + 1 + 1 + m_TrailCount);
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), (m_GridCount * 2)));
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), 1 + 1 + m_TrailCount));

  if (m_SimulationTime > 0.0) {
    for (int i=m_GridStartIndex; i<m_SecondGridStopIndex; i++) {
      m_AtlasSprites[i]->ResetFoo(m_GridFoo, NULL);
    }
  }
}


void SuperStarShooter::DestroyFoos() {
  LOGV("SuperStarShooter::DestroyFoos\n");
  delete m_GridFoo;
  for (unsigned int i=0; i<4; i++) {
    delete m_PlayerFoos[i];
  }
  free(m_PlayerFoos);
  delete m_HoleFoo;
  delete m_TrailFoo;
  //delete m_BatchFoo;

}


void SuperStarShooter::Hit(float x, float y, int hitState) {
  float xx = (((x) - (0.5 * (m_ScreenWidth)))) * m_Zoom;
	float yy = ((0.5 * (m_ScreenHeight) - (y))) * m_Zoom;
  float dx = (xx + m_CameraActualOffsetX) + (SUBDIVIDE / 2.0);
  float dy = (yy + m_CameraActualOffsetY) + (SUBDIVIDE / 2.0);
	float collide_x = (dx);
	float collide_y = (dy);
  int cx = (collide_x / SUBDIVIDE);
  int cy = (collide_y / SUBDIVIDE);
  bool collide_index_set = false;

  if (hitState == 0) {
    m_CameraStopOffsetX = (xx + m_CameraOffsetX);
    m_CameraStopOffsetY = (yy + m_CameraOffsetY);
    m_StartedSwipe = false;
  }

  if (hitState == 1) {
    /*
    if (m_SwipedBeforeUp) {
      m_CameraOffsetX = (m_CameraStopOffsetX - xx);
      m_CameraOffsetY = (m_CameraStopOffsetY - yy);
    }

    if (!m_StartedSwipe) {
      m_GotLastSwipeAt = m_SimulationTime;
      m_StartedSwipe = true;
    }

    if ((m_GotLastSwipeAt > 0.0) && ((m_SimulationTime - m_GotLastSwipeAt) > 0.0655)) {
      m_SwipedBeforeUp = true;
    }
    */
  }


  if (hitState != 0) {
    if (m_SwipedBeforeUp) {
      //end swipe
    } else {
      if (cx >= 0 && cy >= 0) {
        collide_index_set = true;
        //collide_index = m_Space->at(cx, cy, 0);
      }

      if (collide_index_set) {
        m_TargetX = cx;
        m_TargetY = cy;
        m_TargetIsDirty = true;
        //m_WarpTimeout += MAX_WAIT_BEFORE_WARP;
      }
    }
    m_SwipedBeforeUp = false;
  }
}


void SuperStarShooter::RenderModelPhase() {
}


void SuperStarShooter::RenderSpritePhase() {
  glTranslatef(-(m_CameraActualOffsetX), -(m_CameraActualOffsetY), 0.0);

  if (m_NeedsTerrainRebatched) {
    m_Batches[0]->m_NumBatched = 0;
    RenderSpriteRange(m_GridStartIndex, m_GridStopIndex, m_Batches[0]);
    //RenderSpriteRange(m_SecondGridStartIndex, m_SecondGridStopIndex, m_BatchFoo);
    m_NeedsTerrainRebatched = false;
  }

  m_Batches[1]->m_NumBatched = 0;
  RenderSpriteRange(m_TrailStartIndex, m_TrailStopIndex, m_Batches[1]);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_Batches[1]);

  AtlasSprite::RenderFoo(m_StateFoo, m_Batches[0]);
  AtlasSprite::RenderFoo(m_StateFoo, m_Batches[1]);

  //m_BatchFoo->m_NumBatched = 0;
}


int SuperStarShooter::Simulate() {

  bool needs_next_step = false;

  if (m_AtlasSprites[m_PlayerIndex]->MoveToTargetPosition(m_DeltaTime)) {
    m_WarpTimeout += m_DeltaTime;
    if (m_WarpTimeout > MAX_WAIT_BEFORE_WARP) {
      needs_next_step = true;  
      m_WarpTimeout = 0.0;
    }
  } else {
    m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);
  }

  m_CameraOffsetX = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
  m_CameraOffsetY = m_AtlasSprites[m_PlayerIndex]->m_Position[1];

  float tx = (m_CameraActualOffsetX - m_CameraOffsetX);
  float ty = (m_CameraActualOffsetY - m_CameraOffsetY);

  float speed_x = fastAbs(tx) * 1.5;
  float speed_y = fastAbs(ty) * 1.5;
  float speed_max = 1100.0;

  if (speed_x > speed_max) {
    speed_x = speed_max;
  }
  if (speed_y > speed_max) {
    speed_y = speed_max;
  }

  if (tx > 0.0) {
    tx = 1.0;
  } else {
    tx = -1.0;
  }

  if (ty > 0.0) {
    ty = 1.0;
  } else {
    ty = -1.0;
  }

  float mx = (tx * speed_x * m_DeltaTime);
  float my = (ty * speed_y * m_DeltaTime);

  //LOGV("%f %f %f\n", mx, tx, speed);

  /*
  float s = 0.001;

  float mx = (tx * s);// * m_DeltaTime);
  float my = (ty * s);// * m_DeltaTime);
  */

  /*
  if (mx > 0.7) {
    mx = 0.7;
  }
  if (mx < -0.7) {
    mx = -0.7;
  }
  if (my > 0.7) {
    my = 0.7;
  }
  if (my < -0.7) {
    my = -0.7;
  }
  */


  if (false) {
    m_CameraActualOffsetX = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
    m_CameraActualOffsetY = m_AtlasSprites[m_PlayerIndex]->m_Position[1];
  } else {
    // this causes seaming problems
    m_CameraActualOffsetX -= (mx);
    m_CameraActualOffsetY -= (my);
  }

  bool recenter_x = false;
  bool recenter_y = false;
  int dsx = 0;
  int dsy = 0;

  float dx = (m_LastCenterX - m_CameraActualOffsetX);
  float dy = (m_LastCenterY - m_CameraActualOffsetY);

  dsx = (dx / SUBDIVIDE) + m_CenterOfWorldX;
  dsy = (dy / SUBDIVIDE) + m_CenterOfWorldY;

  //if (fastAbs(dx) > (SUBDIVIDE)) {
  if (abs(dsx) > 1) {
    recenter_x = true;
  }
  
  //if (fastAbs(dy) > (SUBDIVIDE)) {
  if (abs(dsy) > 1) {
    recenter_y = true;
  }

  if (recenter_x) {
    m_LastCenterX -= ((float)dsx * SUBDIVIDE);
  }

  if (recenter_y) {
    m_LastCenterY -= ((float)dsy * SUBDIVIDE);
  }

  int xx = 0;
  int yy = 0;

  if ((recenter_x || recenter_y)) {
    m_NeedsTerrainRebatched = true;
    for (int i=m_GridStartIndex; i<m_GridStopIndex; i++) {
      int sx = -1;
      int sy = -1;
      int nsx = 0;
      int nsy = 0;
      IndexToXY(i - m_GridStartIndex, &sx, &sy);
      nsx = sx;
      nsy = sy;

      float px = ((xx + m_CenterOfWorldX) * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
      float py = ((yy + m_CenterOfWorldY) * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);

      if (recenter_x) {
        nsx -= dsx;
        m_AtlasSprites[i]->m_Position[0] = (m_LastCenterX + px);
        m_AtlasSprites[i + m_GridCount]->m_Position[0] = (m_LastCenterX + px);
      }
      if (recenter_y) {
        nsy -= dsy;
        m_AtlasSprites[i]->m_Position[1] = (m_LastCenterY + py);
        m_AtlasSprites[i + m_GridCount]->m_Position[1] = (m_LastCenterY + py);
      }
      m_GridPositions[(i * 2)] = nsx;
      m_GridPositions[(i * 2) + 1] = nsy;
      if (nsx >= 0 && nsy >= 0) {
      //LOGV("do this less %f %f %f %d %f\n", fastAbs(dx), fastAbs(dy), m_SimulationTime, m_CenterOfWorldX, m_LastCenterX);
      //LOGV("%d %d\n", dsx, dsy);
      //192.000244 224.000244
        m_AtlasSprites[i]->m_Frame = m_Space->at(nsx, nsy, 0);
        m_AtlasSprites[i + m_GridCount]->m_Frame = m_Space->at(nsx, nsy, 1);
      } else {
        m_AtlasSprites[i]->m_Frame = OVER;
        m_AtlasSprites[i + m_GridCount]->m_Frame = OVER;
      }
      xx++;
      if (xx >= GRID_X) {
        xx = 0;
        yy++;
      }
    }
  } else {
  }

  float inverter = -1.0;
  for (unsigned int i=0; i<m_TrailCount; i++) {
    m_AtlasSprites[m_TrailStartIndex + i]->Simulate(m_DeltaTime);
    m_AtlasSprites[m_TrailStartIndex + i]->m_Rotation += (m_DeltaTime * 4.0 * inverter);
    inverter *= -1.0;
  }

  bool stuck = false;
  
  if (m_TargetIsDirty) {
    m_TargetIsDirty = false;



    int startState = -1;
    int selected_x = (m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] / SUBDIVIDE);
    int selected_y = (m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1] / SUBDIVIDE);

    //LOGV("wtf: %d\n", m_TargetX);
    int colliding_index = m_Space->at(m_TargetX, m_TargetY, 0);
    if (colliding_index != BLANK) {
      m_StatePointer = 0;
      
      int endState = StatePointerFor(m_TargetX, m_TargetY, 0);
    //LOGV("%d == %d\n", BLANK, colliding_index);
      int startStateTarget = StatePointerFor(selected_x, selected_y, 0);
      startState = startStateTarget;
      float totalCost;
      m_Pather->Reset();
      int solved = m_Pather->Solve((void *)startState, (void *)endState, m_Steps, &totalCost);
      switch (solved) {
        case micropather::MicroPather::SOLVED:
          m_Steps->erase(m_Steps->begin());
          m_TargetIsDirty = false;
          //m_Zoom = 0.33;
          break;
        case micropather::MicroPather::NO_SOLUTION:
          //LOGV("none\n");
          //m_TargetX = selected_x;
          //m_TargetY = selected_y;
          stuck = true;
          m_Steps->clear();
          //m_Zoom = 1.33;
          break;
        case micropather::MicroPather::START_END_SAME:
          //LOGV("same\n");
          //m_TargetX = selected_x;
          //m_TargetY = selected_y;
          m_TargetIsDirty = false;
          break;	
        default:
          break;
      }
    }
  }


    //int stacked = m_TrailCount;
    bool top_of_stack = false;
    for (unsigned int i=0; i<m_TrailCount; i++) {
      if (i < m_Steps->size()) { // && i < stacked) {
        nodexyz *step = m_States[(intptr_t)m_Steps->at(i)];
        float tx = ((float)step->x * SUBDIVIDE);
        float ty = ((float)step->y * SUBDIVIDE);
        //m_AtlasSprites[m_TrailStartIndex + i]->m_Frame = (i % 5);
        m_AtlasSprites[m_TrailStartIndex + i]->m_Frame = 2;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Fps = 0;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Life = 0;
        m_AtlasSprites[m_TrailStartIndex + i]->m_IsAlive = true;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Position[0] = tx;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Position[1] = ty;
      } else {
        if (top_of_stack) {
          //m_AtlasSprites[m_TrailStartIndex + i]->m_Frame = (int)fastAbs((stacked) % 5);
        } else {
          //m_AtlasSprites[m_TrailStartIndex + i]->m_Frame = 3;
          top_of_stack = true;
        }
        //stacked--;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Fps = 0;
        m_AtlasSprites[m_TrailStartIndex + i]->m_IsAlive = false;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Position[0] = 0; //m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0]; //m_TargetX * SUBDIVIDE;
        m_AtlasSprites[m_TrailStartIndex + i]->m_Position[1] = 0; //m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0]m_TargetY * SUBDIVIDE;
      }
    }


  if (needs_next_step && m_Steps->size() > 0) {

    for (int i=0; i<4; i++) {
      if ((m_PlayerStartIndex + i) != m_PlayerIndex) {
        m_AtlasSprites[m_PlayerStartIndex + i]->m_Position[0] = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
        m_AtlasSprites[m_PlayerStartIndex + i]->m_Position[1] = m_AtlasSprites[m_PlayerIndex]->m_Position[1];
      }
    }

    nodexyz *step = m_States[(intptr_t)m_Steps->at(0)];
    m_Steps->erase(m_Steps->begin());

    float tx = ((float)step->x * SUBDIVIDE);
    float ty = ((float)step->y * SUBDIVIDE) + PLAYER_OFFSET;
    dx = tx - m_AtlasSprites[m_PlayerIndex]->m_Position[0];
    dy = ty - m_AtlasSprites[m_PlayerIndex]->m_Position[1];

    if (dy > 0.0) {
      //UP
      m_PlayerIndex = m_PlayerStartIndex + 0;
    }

    if (dx > 0.0) {
      //RIGHT
      m_PlayerIndex = m_PlayerStartIndex + 1;
    }

    if (dy < 0.0) {
      //DOWN
      m_PlayerIndex = m_PlayerStartIndex + 2;
    }

    if (dx < 0.0) {
      //LEFT
      m_PlayerIndex = m_PlayerStartIndex + 3;
    }

    m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0] = tx;
    m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1] = ty;

    for (int i=0; i<4; i++) {
      if ((m_PlayerStartIndex + i) != m_PlayerIndex) {
        m_AtlasSprites[m_PlayerStartIndex + i]->m_TargetPosition[0] = m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0];
        m_AtlasSprites[m_PlayerStartIndex + i]->m_TargetPosition[1] = m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1];
      }
    }
  }

  if (m_TargetY > 0 && m_TargetX > 0 && m_TargetY < 1024 && m_TargetX < 1024 && m_Steps->size() == 0 && !stuck) {
    switch (m_PlayerIndex - m_PlayerStartIndex) {
      case (0):
        m_TargetY++;
        break;
      case (1):
        m_TargetX++;
        break;
      case (2):
        m_TargetY--;
        break;
      case (3):
        m_TargetX--;
        break;
    }
    //if (m_TargetX < 0) {
      
    m_TargetIsDirty = true;
  }
  
  return 1;
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
  
  if (look_distance > MAX_SEARCH) {
    return;
  }
  
  float pass_cost = 0;
  
  for( int i=0; i<4; ++i ) {
    int nx = ax + dx[i];
    int ny = ay + dy[i];	
    bool passable = false;
    if (nx >= 0 && ny >= 0) {
      int colliding_index = m_Space->at(nx, ny, 0);
      //if (colliding_index != (68 + 16 + 16 + 16 + 16)) {
      if (colliding_index != BLANK) {
        //if (nx == m_States[1]->x && ny == m_States[1]->y) {
          passable = true;
        //  pass_cost = 0.0;
        //} else {
        //  passable = true;
        //  pass_cost = 100.0;
        //}
        if (abs(ly) == 1 || abs(lx) == 1) {
          if (i==0 || i==2) {
            if (fastAbs(ly) > fastAbs(lx)) {
              pass_cost = 0.0;
            } else {
              pass_cost = 100.0;
            }
          } else {
            if (fastAbs(lx) > fastAbs(ly)) {
              pass_cost = 0.0;
            } else {
              pass_cost = 100.0;
            }
          }
        } else {
          pass_cost = 100.0;
        }
      }
    }
    
    if (m_StatePointer >= m_MaxStatePointers) {
      passable = false;
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


void SuperStarShooter::LoadMaze(int level_index) {
	uint16_t *level = (uint16_t *)malloc(sizeof(char) * m_LevelFileHandles->at(level_index)->len);
	fseek(m_LevelFileHandles->at(level_index)->fp, m_LevelFileHandles->at(level_index)->off, SEEK_SET);
	fread(level, sizeof(char), m_LevelFileHandles->at(level_index)->len, m_LevelFileHandles->at(level_index)->fp);

  int char_to_int_ratio = sizeof(uint16_t) / sizeof(char);
  int int_count = m_LevelFileHandles->at(level_index)->len / char_to_int_ratio;
  int width = level[0];
  int height = level[1];
  int cursor = 0;
  int r = 0;
  unsigned int i=0;

  LOGV("%d %d %d\n", int_count, width, height);

  for (i=0; i<width*height; i++) {
    //LOGV("%d %d\n", cursor, level[i]);
    BlitMazeCell(r, cursor, level[i+2]);
    cursor++;
    if (cursor > (width - 1)) {
      r++;
      cursor = 0;
    }
  }
  
m_CameraActualOffsetX = (float)width * SUBDIVIDE * 2.0;
m_CameraActualOffsetY = (float)height * SUBDIVIDE * 2.0;

  free(level);
}

void SuperStarShooter::BlitMazeCell(int row, int col, int mask) {
  //BlitIntoSpace(int layer, int bottom_right_start, int width, int height, int offset_x, int offset_y)
  int x = row * 3;
  int y = col * 3;
  int bl = 60 + 16 + 16 + 16;
  switch(mask) {
    case 5:
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      break;
    case 6:
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      break;
    case 7:
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      break;
    case 3075: //above ground
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      break;
    case 3:
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      break;
    case 9:
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      break;
    case 10:
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      break;
    case 11:
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      break;
    case 12:
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      break;
    case 13:
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      break;
    case 14:
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3)); // #
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3)); // ##
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3)); //
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3)); // #
      break;
    case 15:
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3)); // #
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3)); //###
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3)); //
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3)); //
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3)); // #
      break;
    default:
      if (mask > 0) {
        LOGV("%d %d %d\n", row, col, mask);
        //BlitIntoSpace(0, bl, 3, 3, x, y);
      }
      break;
  };
}

  /*
	unsigned int i = 0;
	unsigned int l = m_LevelFoos->at(level_index)->len;

	const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int idx = -1;
	int *data = (int *)malloc(sizeof(int) * l);
	const char *code;
	for (unsigned int j=0; j<l; j++) {
  */
