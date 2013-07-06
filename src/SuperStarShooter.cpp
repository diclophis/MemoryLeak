// Jon Bardin GPL


#include "MemoryLeak.h"


#define ZOOM (1.0)
#define ZOOM2 (1.0 / 3.0)
#define SUBDIVIDE (32.0)
#define BLANK ((16 * 3) + 2)
#define WATER ((16 * 5) + 6)
#define TREASURE 10
#define PURE 97
#define SAND 98
#define FILL BLANK
#define OVER WATER
#define PLAYER_SCALE_X ((SUBDIVIDE * 0.5) * 1.0)
#define PLAYER_SCALE_Y ((SUBDIVIDE * 0.5) + (SUBDIVIDE * 0.2) * 1.0)

#define PLAYER_OFFSET (SUBDIVIDE * 0.5) 
#define PLAYER_OFFSET_X (SUBDIVIDE * 8.0) 
#define VELOCITY (SUBDIVIDE * 65.5)
#define MAX_WAIT_BEFORE_WARP (0.03345)
#define MAX_SEARCH 64
#define MAX_STATE_POINTERS 1024
#define MAX_CAMERA_VELOCITY (SUBDIVIDE * 8)
#define MANUAL_SCROLL_TIMEOUT 0.25
#define BYTES_AT_A_TIME (1024)
#define NETWORK_TIMEOUT (1.0 / 2.0)
#define LEVEL_LOAD_TIMEOUT 0.34
#define LEVEL_LOAD_STRIDE (8)
#define MAX_OTHER_PLAYERS 128

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


struct my_struct {
  int id;            // we'll use this field as the key
  int index;
  int render;
  int update;
  UT_hash_handle hh; // makes this structure hashable
};

struct my_struct *users = NULL;

SuperStarShooter::SuperStarShooter(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {

  m_Network = new MazeNetwork(this, BYTES_AT_A_TIME);
  m_NetworkTickTimeout = 0.0;
  m_PerformNetworkWrite = false;

  m_NeedsTerrainRebatched = true;

  m_CenterOfWorldX = 4;
  m_CenterOfWorldY = 0;

  int xx = 0;
  int yy = 0;

  m_Zoom = ZOOM;
  m_Zoom2 = ZOOM2;

	m_TouchStartX = m_LastCenterX = m_CameraActualOffsetX = m_CameraStopOffsetX = m_CameraOffsetX = 0.0;
	m_TouchStartY = m_LastCenterY = m_CameraActualOffsetY = m_CameraStopOffsetY = m_CameraOffsetY = 0.0;

  m_Space = new Octree<int>(2048, WATER);

  if (false) {
    for (unsigned int i=0; i<124; i++) {
      for (unsigned int ii=0; ii<124; ii++) {
        m_Space->set(i, ii, 0, FILL);
      }
    }
  }

  float overX = 0.33;
  GRID_X = 12; //((((m_ScreenWidth * overX) / SUBDIVIDE))); // + 3;
  GRID_Y = 15; //((((m_ScreenHeight * overX) / SUBDIVIDE))); // + 3;

  m_GridCount = (GRID_X * GRID_Y);
  float sizeOfCell = (SUBDIVIDE / 2.0);

  LOGV("%f %d %d %d\n", sizeOfCell, m_GridCount, GRID_X, GRID_Y);

  m_GridPositions = (int *)malloc((m_GridCount * 2) * sizeof(int));
  m_GridStartIndex = m_SpriteCount;

  m_TrailCount = 0; //MAX_SEARCH * 2;

  m_LoadedLevel = false;
  m_MazeCursor = 0;
  m_LevelIndex = 0;
  m_LevelLoadTimeout = 0;
  m_MazeLoadStride = LEVEL_LOAD_STRIDE;
  m_MazeWalkX = m_MazeWalkY = m_MazeWalkXX = m_MazeWalkYY = 0;

  LoadMaze();

  // this will draw a temple
  if (false) {
    //int layer, int bottom_right_start, int width, int height, int offset_x, int offset_y
    int bt_x = 10;
    int bt_y = 10;
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

  LoadSound(0);
  CreateFoos();

  for (int i=0; i<(m_GridCount * 2); i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_GridFoo, NULL));
  }

  for (int i=0; i<m_GridCount; i++) {

    float px = ((xx) * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE);
    float py = ((yy) * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE);
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



    m_AtlasSprites[m_SpriteCount]->SetScale(sizeOfCell, sizeOfCell);
    m_AtlasSprites[m_SpriteCount + m_GridCount]->SetScale(sizeOfCell, sizeOfCell);
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

  m_WarpTimeout = 0.0;

	m_Pather = new micropather::MicroPather(this);
	m_Steps = new std::vector<void *>;
  m_CurrentStep = 0;

  m_MaxStatePointers = MAX_STATE_POINTERS;
  m_StatePointer = 0;
  for (int i=0; i<m_MaxStatePointers; i++) {
    m_States.push_back(new nodexyz());
  }

  m_TargetIsDirty = false;
  m_SelectTimeout = MANUAL_SCROLL_TIMEOUT;

  m_GotLastSwipeAt = -10.0;
  m_SwipedBeforeUp = false;

  m_DesiredTargetX = 0;
  m_DesiredTargetY = 0;

  m_TrailStartIndex = m_SpriteCount;
  for (unsigned int i=0; i<m_TrailCount; i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_TrailFoo, NULL));
    //m_AtlasSprites[m_SpriteCount]->SetPosition((m_CenterOfWorldX * (SUBDIVIDE)), ((m_CenterOfWorldY * (SUBDIVIDE))));
    m_AtlasSprites[m_SpriteCount]->SetPosition(0, 0);
    //m_AtlasSprites[m_SpriteCount]->m_IsAlive = false;
    m_AtlasSprites[m_SpriteCount]->m_Fps = 25;
    m_AtlasSprites[m_SpriteCount]->SetScale(100, 100);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    //m_AtlasSprites[m_SpriteCount]->m_Rotation = i * 20;
    m_SpriteCount++;
  }
  m_TrailStopIndex = m_SpriteCount;

  m_PlayerStartIndex = m_SpriteCount;
  AddPlayer((m_CenterOfWorldX * (SUBDIVIDE)), ((m_CenterOfWorldY * (SUBDIVIDE))), VELOCITY);

  struct my_struct *ss = (struct my_struct *)malloc(sizeof(struct my_struct));
  m_PlayerId = 0;
  ss->id = m_PlayerId;
  ss->index = m_PlayerStartIndex;
  ss->render = m_PlayerStartIndex;
  m_PlayerIndex = m_PlayerStartIndex;
  HASH_ADD_INT(users, id, ss);    

	m_CameraActualOffsetX = (m_CenterOfWorldX * (SUBDIVIDE));
  m_CameraActualOffsetY = (((m_CenterOfWorldY) * (SUBDIVIDE))) + PLAYER_OFFSET;
  m_ForceRebuffer = true;
}


void SuperStarShooter::AddPlayer(float x, float y, float v) {
  for (unsigned int i=0; i<4; i++) {
    int sub_index = m_SpriteCount;
    m_AtlasSprites.push_back(new SpriteGun(m_PlayerFoos.at(i), NULL));
    m_AtlasSprites[sub_index]->SetVelocity(v, v);
    m_AtlasSprites[sub_index]->SetPosition(x, y + PLAYER_OFFSET);
    m_AtlasSprites[sub_index]->m_IsAlive = true;
    m_AtlasSprites[sub_index]->m_Fps = 15;
    m_AtlasSprites[sub_index]->m_Frame = 0;
    m_AtlasSprites[sub_index]->SetScale(PLAYER_SCALE_X, PLAYER_SCALE_Y);
    m_AtlasSprites[sub_index]->m_TargetPosition[0] = m_AtlasSprites[sub_index]->m_Position[0];
    m_AtlasSprites[sub_index]->m_TargetPosition[1] = m_AtlasSprites[sub_index]->m_Position[1];
    m_AtlasSprites[sub_index]->Build(0);
    m_SpriteCount++;
  }
  m_PlayerStopIndex = m_SpriteCount;
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
      m_Space->set(fx + offset_x, fy + offset_y, layer, bottom_right_start);
      m_Space->set(fx + offset_x, fy + offset_y, layer + 1, BLANK);
      bottom_right_start -= 1;
    }
    bottom_right_start -= (16);
    bottom_right_start += width;
  }
}


void SuperStarShooter::CreateFoos() {
  ResetStateFoo();

  LoadTexture(0);

  m_GridFoo = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 0, 256, 0.0);

  for (unsigned int i=0; i<4; i++) {
    m_PlayerFoos.push_back(AtlasSprite::GetFoo(m_Textures.at(0), 16, 14, 13 + (16 * i), 13 + (16 * i) + 3, 0.0));
  }

  m_TrailFoo = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 251, 256, 0.0);

  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), (m_GridCount)));
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), (m_GridCount)));
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), MAX_OTHER_PLAYERS + 1)); //1 + 1 + m_TrailCount));
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), m_TrailCount)); //1 + 1 + m_TrailCount));
  
  int p_foo = 0;
  if (m_SimulationTime > 0.0) {
    for (int i=0; i<m_SpriteCount; i++) {
      if (i >= m_PlayerStartIndex && i < m_PlayerStopIndex) {
        m_AtlasSprites[i]->ResetFoo(m_PlayerFoos.at(p_foo), NULL);
        p_foo++;
      } else if (i >= m_TrailStartIndex && i < m_TrailStopIndex) {
        m_AtlasSprites[i]->ResetFoo(m_TrailFoo, NULL);
      } else if (i >= m_GridStartIndex && i < m_GridStopIndex) {
        m_AtlasSprites[i]->ResetFoo(m_GridFoo, NULL);
      } else if (i >= m_SecondGridStartIndex && i < m_SecondGridStopIndex) {
        m_AtlasSprites[i]->ResetFoo(m_GridFoo, NULL);
      } else {
        assert(false);
      }
    }
    m_NeedsTerrainRebatched = true;
  }
}


void SuperStarShooter::DestroyFoos() {
  LOGV("SuperStarShooter::DestroyFoos\n");

  for (std::vector<GLuint>::iterator i = m_Textures.begin(); i != m_Textures.end(); ++i) {
    glDeleteTextures(1, &*i); // yea that happened
  }
  m_Textures.clear();

  if (m_GridFoo) {
    delete m_GridFoo;
    m_GridFoo = NULL;
  }

  for (std::vector<foofoo *>::iterator i = m_PlayerFoos.begin(); i != m_PlayerFoos.end(); ++i) {
    delete *i;
  }
  m_PlayerFoos.clear();

  if (m_TrailFoo) {
    delete m_TrailFoo;
    m_TrailFoo = NULL;
  }

  for (std::vector<foofoo *>::iterator i = m_Batches.begin(); i != m_Batches.end(); ++i) {
    delete *i;
  }
  m_Batches.clear();
}


// handle touch events
void SuperStarShooter::Hit(float x, float y, int hitState) {
  float xx = (((x) - (0.5 * (m_ScreenWidth)))) * m_Zoom2;
	float yy = ((0.5 * (m_ScreenHeight) - (y))) * m_Zoom2;
  float dx = (xx + m_CameraActualOffsetX) - (SUBDIVIDE / 2.0);
  float dy = (yy + m_CameraActualOffsetY) - (SUBDIVIDE / 2.0);

  float collide_x = (dx);
  float collide_y = (dy);
  int cx = ((collide_x + PLAYER_OFFSET) / SUBDIVIDE);
  int cy = ((collide_y + PLAYER_OFFSET) / SUBDIVIDE);
  bool collide_index_set = false;

  if (hitState == 0) {
    m_CameraStopOffsetX = (xx + m_CameraActualOffsetX);
    m_CameraStopOffsetY = (yy + m_CameraActualOffsetY);
    m_StartedSwipe = false;
    m_SwipedBeforeUp = false;
    m_ForceRebuffer = false;
  }

  if (hitState != 0) {
    if (false && m_SwipedBeforeUp) {
      //end swipe
      if (hitState == 1) {
        m_SelectTimeout = 0;
        m_StartedSwipe = true;
      } else {
        m_StartedSwipe = false;
      }
    } else {
      if (cx >= 0 && cy >= 0) {
        collide_index_set = true;
      }

      if (collide_index_set) {
        if (hitState == 2 && !m_SwipedBeforeUp) {
          m_TargetX = cx;
          m_TargetY = cy;
          m_TargetIsDirty = true;
        }
      }
    }

    m_DesiredTargetX = (xx - m_CameraStopOffsetX);
    m_DesiredTargetY = (yy - m_CameraStopOffsetY);

    float movedX = fastAbs(m_CameraStopOffsetX - (xx + m_CameraActualOffsetX));
    float movedY = fastAbs(m_CameraStopOffsetY - (yy + m_CameraActualOffsetY));

    if (fastAbs(movedX) > (SUBDIVIDE / 4.0) || fastAbs(movedY) > (SUBDIVIDE / 4.0)) {
      m_SwipedBeforeUp = true;
    }
  }
}


void SuperStarShooter::RenderModelPhase() {
}

//float roundp(float num, int precision)
//{
//  return floorf(num * pow(10.0f,precision) + .5f)/pow(10.0f,precision);
//}

// render the scene
void SuperStarShooter::RenderSpritePhase() {
  float a = (((int)cdx) + (SUBDIVIDE / 2.0));
  float b = (((int)cdy) + (SUBDIVIDE / 2.0));
  float offX = (-m_LastCenterX / (SUBDIVIDE / 2.0));
  float offY = (-m_LastCenterY / (((SUBDIVIDE / 2.0) + ((1.0 / 5.0) * SUBDIVIDE))));

  //float offX2 = (-0 / (SUBDIVIDE / 2.0));
  //float offY2 = -0 / (((SUBDIVIDE / 2.0) + ((1.0 / 5.0) * SUBDIVIDE)));

  if (m_Batches.size() == 4) {
    //if (m_NeedsTerrainRebatched) {
      m_Batches[0]->m_NumBatched = 0;
      m_Batches[1]->m_NumBatched = 0;
      RenderSpriteRange(m_GridStartIndex, m_GridStopIndex, m_Batches[0], 0.0, 0.0);
      //RenderSpriteRange(m_SecondGridStartIndex, m_SecondGridStopIndex, m_Batches[1], 0, 0);
      m_NeedsTerrainRebatched = false;
    //}
    
    m_Batches[2]->m_NumBatched = 0;
    m_Batches[3]->m_NumBatched = 0;

    struct my_struct *s;

    for(s = users; s != NULL; s = (struct my_struct *)s->hh.next) {
      if ((m_SimulationTime - s->update) < 30.0) { 
        RenderSpriteRange(s->render, s->render + 1, m_Batches[2], offX, offY);
        //RenderSpriteRange(s->render, s->render + 1, m_Batches[2], 0, 0);
      }
    }

    glTranslatef(a, b, 0.0);
    //glTranslatef(0.0, 0.0, 0.0);
    AtlasSprite::RenderFoo(m_StateFoo, m_Batches[0]);
    AtlasSprite::RenderFoo(m_StateFoo, m_Batches[2]);

    //glTranslatef(-m_LastCenterX, -m_LastCenterY, 0.0);
    //RenderSpriteRange(m_TrailStartIndex, m_TrailStopIndex, m_Batches[3], 0.0, 0.0);
  
    //AtlasSprite::RenderFoo(m_StateFoo, m_Batches[3]);
    //AtlasSprite::RenderFoo(m_StateFoo, m_Batches[1]);
  }
}


int SuperStarShooter::Simulate() {

  m_LevelLoadTimeout += m_DeltaTime;
  if (m_LevelLoadTimeout > LEVEL_LOAD_TIMEOUT) {
    LoadMaze();
    m_LevelLoadTimeout = 0;
  }

  struct my_struct *s;
  HASH_FIND_INT(users, &m_PlayerId, s);

  //m_AtlasSprites[m_TrailStartIndex]->m_Position[0] = m_AtlasSprites[s->render]->m_Position[0];
  //m_AtlasSprites[m_TrailStartIndex]->m_Position[1] = m_AtlasSprites[s->render]->m_Position[1];

  struct my_struct *ss;

  bool player_at_target = false;

  for(ss = users; ss != NULL; ss = (struct my_struct *)ss->hh.next) {
    if (m_AtlasSprites[ss->render]->MoveToTargetPosition(m_DeltaTime)) {
      if (ss->index == s->index) {
        player_at_target = true;
      }
    } else {
      m_AtlasSprites[ss->render]->Simulate(m_DeltaTime);
    }

    int rStartIndex = ss->index;

    for (int i=0; i<4; i++) {
      if ((rStartIndex + i) != ss->render) {
        m_AtlasSprites[rStartIndex + i]->m_Position[0] = m_AtlasSprites[ss->render]->m_Position[0];
        m_AtlasSprites[rStartIndex + i]->m_Position[1] = m_AtlasSprites[ss->render]->m_Position[1];
        m_AtlasSprites[rStartIndex + i]->m_TargetPosition[0] = m_AtlasSprites[ss->render]->m_TargetPosition[0];
        m_AtlasSprites[rStartIndex + i]->m_TargetPosition[1] = m_AtlasSprites[ss->render]->m_TargetPosition[1];
      }
    }

    float tx = m_AtlasSprites[rStartIndex]->m_TargetPosition[0];
    float ty = m_AtlasSprites[rStartIndex]->m_TargetPosition[1];
    float pdx = tx - m_AtlasSprites[rStartIndex]->m_Position[0];
    float pdy = ty - m_AtlasSprites[rStartIndex]->m_Position[1];

    if (pdy > 0.0) {
      //UP
      ss->render = rStartIndex + 0;
    }

    if (pdx > 0.0) {
      //RIGHT
      ss->render = rStartIndex + 1;
    }

    if (pdy < 0.0) {
      //DOWN
      ss->render = rStartIndex + 2;
    }

    if (pdx < 0.0) {
      //LEFT
      ss->render = rStartIndex + 3;
    }
  }

  bool needs_next_step = false;
  if (player_at_target) {
    m_WarpTimeout += m_DeltaTime;
  }

  if (m_WarpTimeout > MAX_WAIT_BEFORE_WARP) {
    needs_next_step = true;  
    m_WarpTimeout = 0.0;
  }

  if (s != NULL) {
    m_SelectTimeout += m_DeltaTime;
    s->update = m_SimulationTime;
    if (m_SelectTimeout > MANUAL_SCROLL_TIMEOUT) {
      m_DesiredTargetX = m_DeltaTime * 4.0 * (m_CameraActualOffsetX - m_AtlasSprites[s->render]->m_Position[0]);
      m_DesiredTargetY = m_DeltaTime * 4.0 * (m_CameraActualOffsetY - m_AtlasSprites[s->render]->m_Position[1]);
      //m_DesiredTargetX = (m_CameraActualOffsetX - m_AtlasSprites[s->render]->m_Position[0]);
      //m_DesiredTargetY = (m_CameraActualOffsetY - m_AtlasSprites[s->render]->m_Position[1]);
      m_CameraActualOffsetX += -m_DesiredTargetX;
      m_CameraActualOffsetY += -m_DesiredTargetY;
    } else if (m_StartedSwipe) {
      m_CameraActualOffsetX = -m_DesiredTargetX;
      m_CameraActualOffsetY = -m_DesiredTargetY;
    }
  }

  // manage player target selection and pathfinding
  if (s != NULL) {
    if (m_TargetIsDirty) {
      m_TargetIsDirty = false;
      int startState = -1;
      int colliding_index = m_Space->at(m_TargetX, m_TargetY, 0);
      bool foundEndState = false;
      if (Passable(colliding_index)) {
        foundEndState = true;
      } else {
        int dirs[16] = {
          +1, +0,
          +0, +1,
          +0, -1,
          -1, +0,
          -1, +1,
          +1, +1,
          +1, -1,
          -1, -1
        };
        for (int i=0; i<16; i+=2) {
          int altTargetX = m_TargetX + dirs[i];
          int altTargetY = m_TargetY + dirs[i+1];
          if (altTargetX >= 0 && altTargetY >= 0) {
            colliding_index = m_Space->at(altTargetX, altTargetY, 0);
            if (Passable(colliding_index)) {
              m_TargetX = altTargetX;
              m_TargetY = altTargetY;
              foundEndState = true;
              break;
            }
          }
        }
      }

      if (foundEndState) {
        m_StatePointer = 0;
        int endState = StatePointerFor(m_TargetX, m_TargetY, 0);

        int selected_x = (m_AtlasSprites[s->render]->m_Position[0] / SUBDIVIDE);
        int selected_y = (m_AtlasSprites[s->render]->m_Position[1] / SUBDIVIDE);

        int startStateTarget = StatePointerFor(selected_x, selected_y, 0);
        startState = startStateTarget;
        float totalCost;
        m_Pather->Reset();
        m_Steps->clear();
        m_CurrentStep = 0;
        int solved = m_Pather->Solve((void *)startState, (void *)endState, m_Steps, &totalCost);
        switch (solved) {
          case micropather::MicroPather::SOLVED:
            m_CurrentStep = 1;
            break;
          case micropather::MicroPather::NO_SOLUTION:
            break;
          case micropather::MicroPather::START_END_SAME:
            break;
          default:
            break;
        }
      }
    }

    if (needs_next_step && m_Steps->size() > 0 && m_CurrentStep < m_Steps->size()) {
      m_NetworkTickTimeout += NETWORK_TIMEOUT;
      nodexyz *step = m_States[(intptr_t)m_Steps->at(m_CurrentStep)];
      m_CurrentStep++;

      float tx = ((float)step->x * SUBDIVIDE);
      float ty = ((float)step->y * SUBDIVIDE);

      m_AtlasSprites[s->render]->m_TargetPosition[0] = tx;
      m_AtlasSprites[s->render]->m_TargetPosition[1] = ty + PLAYER_OFFSET;

      for (int i=0; i<4; i++) {
        if ((m_PlayerStartIndex + i) != s->render) {
          m_AtlasSprites[m_PlayerStartIndex + i]->m_TargetPosition[0] = m_AtlasSprites[s->render]->m_TargetPosition[0];
          m_AtlasSprites[m_PlayerStartIndex + i]->m_TargetPosition[1] = m_AtlasSprites[s->render]->m_TargetPosition[1];
        }
      }
    }
  }


  // manage tilemap
  bool recenter_x = false;
  bool recenter_y = false;
  int dsx = 0;
  int dsy = 0;

  cdx = (m_LastCenterX - m_CameraActualOffsetX);
  cdy = (m_LastCenterY - m_CameraActualOffsetY);

  dsx = ((cdx / SUBDIVIDE));
  dsy = ((cdy / SUBDIVIDE));

  if (abs(dsx) > 0) {
    recenter_x = true;
    cdx = 0;
  }
  
  if (abs(dsy) > 0) {
    recenter_y = true;
    cdy = 0;
  }

  if (recenter_x) {
    m_LastCenterX -= ((float)dsx * SUBDIVIDE);
  }

  if (recenter_y) {
    m_LastCenterY -= ((float)dsy * SUBDIVIDE);
  }

  int xx = 0;
  int yy = 0;

  if ((recenter_x || recenter_y) || m_NeedsTerrainRebatched) {
    m_NeedsTerrainRebatched = true;
    for (int i=m_GridStartIndex; i<(m_GridStopIndex); i++) {
      int nsx = 0;
      int nsy = 0;
      int offset_index = i - m_GridStartIndex;
      nsx = m_GridPositions[(offset_index * 2)];
      nsy = m_GridPositions[(offset_index * 2) + 1];

      float px = (((xx) * SUBDIVIDE) - ((GRID_X / 2) * SUBDIVIDE));
      float py = (((yy) * SUBDIVIDE) - ((GRID_Y / 2) * SUBDIVIDE));

      float foo_x = (px);
      float foo_y = (py);

      if (recenter_x) {
        nsx -= dsx;
        m_AtlasSprites[i]->m_Position[0] = foo_x;
        m_AtlasSprites[i + m_GridCount]->m_Position[0] = foo_x;
      }
      if (recenter_y) {
        nsy -= dsy;
        m_AtlasSprites[i]->m_Position[1] = foo_y;
        m_AtlasSprites[i + m_GridCount]->m_Position[1] = foo_y;
      }
      m_GridPositions[(i * 2)] = nsx;
      m_GridPositions[(i * 2) + 1] = nsy;
      if (nsx >= 0 && nsy >= 0) {
        bool shorted = false;
        bool fast_short = false;

        fast_short = true;

        if ((recenter_x && recenter_y) || nsx == 0 || nsy == 0) {
          shorted = false;
          fast_short = true;
        }

        if (!fast_short && recenter_y) {
          if (dsy < 0) {
            if (i < (m_GridStopIndex + (GRID_X * dsy))) {
              shorted = true;
              m_AtlasSprites[i]->m_OldFrame = m_AtlasSprites[i]->m_Frame;
              m_AtlasSprites[i]->m_Frame = m_AtlasSprites[i + (GRID_X * -dsy)]->m_Frame;
            }
          } else {
            if (offset_index > (GRID_X * dsy)) {
              shorted = true;
              m_AtlasSprites[i]->m_OldFrame = m_AtlasSprites[i]->m_Frame;
              m_AtlasSprites[i]->m_Frame = m_AtlasSprites[i + (GRID_X * -dsy)]->m_OldFrame;
            }
          }
        } 
        
        if (!fast_short && recenter_x) {
          if (dsx < 0) {
            if (((offset_index % GRID_X) < (GRID_X + dsx))) {
              shorted = true;
              m_AtlasSprites[i]->m_OldFrame = m_AtlasSprites[i]->m_Frame;
              m_AtlasSprites[i]->m_Frame = m_AtlasSprites[i + -dsx]->m_Frame;
            }
          } else {
            if (((offset_index % GRID_X) > dsx)) {
              shorted = true;
              m_AtlasSprites[i]->m_OldFrame = m_AtlasSprites[i]->m_Frame;
              m_AtlasSprites[i]->m_Frame = m_AtlasSprites[i + -dsx]->m_OldFrame;
            }
          }
        }

        if (m_ForceRebuffer || !shorted) {
          m_AtlasSprites[i]->m_OldFrame = m_AtlasSprites[i]->m_Frame;
          m_AtlasSprites[i]->m_Frame = m_Space->at(nsx, nsy, 0);
          m_AtlasSprites[i + m_GridCount]->m_Frame = m_Space->at(nsx, nsy, 1);
        }
      } else {
        m_AtlasSprites[i]->m_Frame = OVER;
        m_AtlasSprites[i + m_GridCount]->m_Frame = BLANK;
      }

      xx++;
      if (xx >= GRID_X) {
        xx = 0;
        yy++;
      }
    }
  }

  if (m_ForceRebuffer) {
    m_ForceRebuffer = false;
  }

  // manage trail that indicates player target
  float inverter = -1.0;
  for (unsigned int i=0; i<m_TrailCount; i++) {
    m_AtlasSprites[m_TrailStartIndex + i]->Simulate(m_DeltaTime);
    m_AtlasSprites[m_TrailStartIndex + i]->m_Position[0] = m_TargetX * SUBDIVIDE;
    m_AtlasSprites[m_TrailStartIndex + i]->m_Position[1] = m_TargetY * SUBDIVIDE;
    //m_AtlasSprites[m_TrailStartIndex + i]->m_Rotation += (m_DeltaTime * 4.0 * inverter);
    inverter *= -1.0;
  }

  // process network events
  m_NetworkTickTimeout += m_DeltaTime;
  if (m_NetworkTickTimeout > NETWORK_TIMEOUT) {
    m_NetworkTickTimeout = 0.0;
    m_PerformNetworkWrite = true;
  }

  int network_status = m_Network->Tick(m_PerformNetworkWrite,
    (int) m_AtlasSprites[m_PlayerIndex]->m_Position[0], (int) m_AtlasSprites[m_PlayerIndex]->m_Position[1],
    (int) m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[0], (int) m_AtlasSprites[m_PlayerIndex]->m_TargetPosition[1]
  );
  if (network_status > 0) {
    //LOGV("incorrect network status %d\n", network_status);
  }

  m_PerformNetworkWrite = false;

  return 1;
}


// calculate the possible least cost between two states
float SuperStarShooter::LeastCostEstimate(void *nodeStart, void *nodeEnd) {	
  int xStart = m_States[((intptr_t)nodeStart)]->x;
  int yStart = m_States[((intptr_t)nodeStart)]->y;

  int xEnd = m_States[((intptr_t)nodeEnd)]->x;
  int yEnd = m_States[((intptr_t)nodeEnd)]->y;

  int dx = xStart - xEnd;
  int dy = yStart - yEnd;
  int dz = 0;

  //float least_cost = dx + dy;
  float least_cost = (float) sqrt((double)(dx * dx) + (double)(dy * dy) + (double)(dz * dz));

  return least_cost;
}


// only sand tiles are walkable
bool SuperStarShooter::Passable(int i) {
  if (i == 91 || i == 92 || i == 74 || i == 75 || i == 76 || i == 90 || i == 106 || i == 107 || i == 108) {
    return true;
  }

  return false;
}


// calculates the path finding for player movement
// each node of the graph is an index into the state pool
// the state pools contain collections of x,y values
void SuperStarShooter::AdjacentCost(void *node, std::vector<micropather::StateCost> *neighbors) {
  int ax = m_States[((intptr_t)node)]->x;
  int ay = m_States[((intptr_t)node)]->y;

  const int dx[8] = { 1, 0, -1,  0,   1, -1,  1, -1};
  const int dy[8] = { 0, 1,  0, -1,   1, -1, -1,  1};
  
  int lx = m_States[0]->x - ax;
  int ly = m_States[0]->y - ay;
  int lz = 0;

  float look_distance = (float)sqrt((double)(lx * lx) + (double)(ly * ly) + (double)(lz * lz));
  
  if (look_distance > MAX_SEARCH) {
    return;
  }
  
  float pass_cost = 0;
  
  for( int i=0; i<8; i++) {
    int nx = ax + dx[i];
    int ny = ay + dy[i];	
    bool passable = false;
    if (nx >= 0 && ny >= 0) {
      int colliding_index = m_Space->at(nx, ny, 0);
      if (Passable(colliding_index)) {
        passable = true;
        pass_cost = 1.0;
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


// fetch the index of an available State from the shared state pool, based on x/y/z
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


// Loads maze from given file handle index
// mazes are binary files, see top of file for details
// reads the entire maze into memory
// pops the first two chars for width and height
// iterates over remaining chars to fill in octree
// the octree stores whether or not a cell is passable
// and what sprite the cell should be draw with
void SuperStarShooter::LoadMaze() {
  if (!m_LoadedLevel) {
    m_Level = (uint16_t *)malloc(sizeof(uint16_t) * m_LevelFileHandles->at(m_LevelIndex)->len);
    fseek(m_LevelFileHandles->at(m_LevelIndex)->fp, m_LevelFileHandles->at(m_LevelIndex)->off, SEEK_SET);
    fread(m_Level, sizeof(char), m_LevelFileHandles->at(m_LevelIndex)->len, m_LevelFileHandles->at(m_LevelIndex)->fp);
    m_LoadedLevel = true;
  }

  int width = m_Level[0];
  int height = m_Level[1];

  for (int i=0; i<m_MazeLoadStride; i++) {
    int d = width;
    if (m_MazeCursor < (width * height)) {
      if (m_MazeCursor == 0) {
        BlitMazeCell(0, 0, d);
        m_NeedsTerrainRebatched = true;
        m_MazeWalkXX = 1;
        m_MazeCursor++;
      } else {
        if (m_MazeWalkYY == 0) {
          if (m_MazeWalkY < (m_MazeWalkXX)) {
            m_MazeWalkX = m_MazeWalkXX;
            BlitMazeCell(m_MazeWalkX, m_MazeWalkY, d);
            m_NeedsTerrainRebatched = true;
            m_MazeCursor++;
            m_MazeWalkY++;
          } else {
            BlitMazeCell(m_MazeWalkX, m_MazeWalkY, d);
            m_NeedsTerrainRebatched = true;
            m_MazeCursor++;
            m_MazeWalkYY = 1;
            m_MazeWalkX = (m_MazeWalkY - 1);
            continue;
          }
        }

        if (m_MazeWalkYY == 1) {
          BlitMazeCell(m_MazeWalkX, m_MazeWalkY, d);
          m_NeedsTerrainRebatched = true;
          m_MazeCursor++;
          if (m_MazeWalkX == 0) {
            m_MazeWalkXX++;
            m_MazeWalkX = m_MazeWalkXX;
            m_MazeWalkYY = 0;
            m_MazeWalkY = 0;
          } else {
            m_MazeWalkX--;
          }
        }
      }
    }
  }
}


void SuperStarShooter::BlitMazeCell(int row, int col, int w) {
  //    col = m_MazeCursor % (width);
  //    row = m_MazeCursor / (width);
  int mask = m_Level[2 + (row * w + col)];
  int x = row * 3;
  int y = col * 3;


  int b8 = (12 * 16) + 12;
  int bc = (12 * 16) + 12;
  int ba = (12 * 16) + 13;
  int b7 = (12 * 16) + 15;

  int rOff = 2;
  int cOff = -1;

  int b9 = (rOff * 16) + (4 * 16) + 7 + cOff;
  int be = (rOff * 16) + (4 * 16) + 9 + cOff;

  int bd = (rOff * 16) + (6 * 16) + 7 + cOff;
  int b5 = (rOff * 16) + (6 * 16) + 9 + cOff;

  int bl = (0 * 16) + (6 * 16) + 12;

  int b4 = (0 * 16) + (8 * 16) + 13;

  int b3 = (0 * 16) + (9 * 16) + 12;
  int bb = (0 * 16) + (9 * 16) + 15;

  int b6 = (0 * 16) + (10 * 16) + 11;
  int b2 = (10 * 16) + 15;

  switch(mask) {
    case 5:
      //7#6
      //##6
      //445
      BlitIntoSpace(0, b7, 3, 3, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 2) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 1) * 3)); 
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 1) * 3) + 2); 
      BlitIntoSpace(0, b4, 2, 2, ((x + 0) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 0) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 1) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 1) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b5, 2, 2, ((x + 2) * 3), ((y + 0) * 3) + 1);
      break;
    case 6:
      //2#c
      //2##
      //d44
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 2) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bc, 3, 3, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bd, 2, 2, ((x + 0) * 3) + 1, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 1) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 1) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 2) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 2) * 3) + 2, ((y + 0) * 3) + 1);
      break;
    case 7:
      //7#8
      //###
      //444
      BlitIntoSpace(0, b7, 3, 3, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b8, 3, 3, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b4, 2, 2, ((x + 0) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 0) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 1) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 1) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 2) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 2) * 3) + 2, ((y + 0) * 3) + 1);
      break;
    case 3:
      //aaa
      //###
      //444
      BlitIntoSpace(0, ba, 2, 2, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 0) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 1) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 2) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b4, 2, 2, ((x + 0) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 0) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 1) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 1) * 3) + 2, ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 2, 2, ((x + 2) * 3), ((y + 0) * 3) + 1);
      BlitIntoSpace(0, b4, 1, 2, ((x + 2) * 3) + 2, ((y + 0) * 3) + 1);
      break;
    case 9:
      //aae
      //##6
      //b#6
      BlitIntoSpace(0, ba, 2, 2, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 0) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 1) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, be, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, bb, 3, 3, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 0) * 3) + 2);
      break;
    case 10:
      //9aa
      //2##
      //2#3
      BlitIntoSpace(0, b9, 2, 2, ((x + 0) * 3) + 1, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 2) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 1) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 0) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b3, 3, 3, ((x + 2) * 3), ((y + 0) * 3));
      break;
    case 11:
      //aaa
      //###
      //b#3
      BlitIntoSpace(0, ba, 2, 2, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 0) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 1) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, ba, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, ba, 1, 2, ((x + 2) * 3) + 2, ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bb, 3, 3, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b3, 3, 3, ((x + 2) * 3), ((y + 0) * 3));
      break;
    case 12:
      //2#6
      //2#6
      //2#6
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 2) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 2) * 3) + 2);
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 0) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 0) * 3) + 2);
      break;
    case 13:
      //7#6
      //##6
      //b#6
      BlitIntoSpace(0, b7, 3, 3, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 2) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, bb, 3, 3, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b6, 2, 2, ((x + 2) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b6, 2, 1, ((x + 2) * 3), ((y + 0) * 3) + 2);
      break;
    case 14:
      //2#8
      //2##
      //2#3
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 2) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b8, 3, 3, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 1) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, b2, 3, 2, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b2, 3, 1, ((x + 0) * 3), ((y + 0) * 3) + 2);
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b3, 3, 3, ((x + 2) * 3), ((y + 0) * 3));
      break;
    case 15:
      //7#8
      //###
      //b#3
      BlitIntoSpace(0, b7, 3, 3, ((x + 0) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, b8, 3, 3, ((x + 2) * 3), ((y + 2) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
      BlitIntoSpace(0, bb, 3, 3, ((x + 0) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 0) * 3));
      BlitIntoSpace(0, b3, 3, 3, ((x + 2) * 3), ((y + 0) * 3));
      break;
    //case 3075: //above ground
    //  BlitIntoSpace(0, bl, 3, 3, ((x + 0) * 3), ((y + 1) * 3));
    //  BlitIntoSpace(0, bl, 3, 3, ((x + 1) * 3), ((y + 1) * 3));
    //  BlitIntoSpace(0, bl, 3, 3, ((x + 2) * 3), ((y + 1) * 3));
    //  break;
    default:
      if (mask > 0) {
        LOGV("missing blit pattern %d %d %d\n", row, col, mask);
        //BlitIntoSpace(0, bl, 3, 3, x, y);
      }
      break;
  };
}


bool SuperStarShooter::UpdatePlayerAtIndex(int i, float x, float y, float a, float b) {

  struct my_struct *s = NULL;

  HASH_FIND_INT(users, &i, s);

  if (NULL == s) {
    if ((((m_PlayerStopIndex - m_PlayerStartIndex) / 4) - 1) < MAX_OTHER_PLAYERS) {
      s = (struct my_struct*)malloc(sizeof(struct my_struct));
      s->id = i;
      s->index = m_SpriteCount;
      s->render = s->index;
      AddPlayer(x * SUBDIVIDE, y * SUBDIVIDE, VELOCITY * 0.75);
      HASH_ADD_INT(users, id, s);
      m_AtlasSprites[s->render]->SetPosition(x, y);
    } else {
      return true;
    }
  }

  if (m_AtlasSprites[s->render]->m_TargetPosition[0] != a || m_AtlasSprites[s->render]->m_TargetPosition[1] != b) {
    //m_AtlasSprites[s->render]->m_Position[0] = (x);
    //m_AtlasSprites[s->render]->m_Position[1] = (y);
    m_AtlasSprites[s->render]->m_TargetPosition[0] = (a);
    m_AtlasSprites[s->render]->m_TargetPosition[1] = (b);
    s->update = m_SimulationTime;
  }

  return true;
}


bool SuperStarShooter::RequestRegistration(int i) {
  //m_PlayerId = i;
  return true;
}
