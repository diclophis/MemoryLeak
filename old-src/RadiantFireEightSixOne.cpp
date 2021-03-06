// Jon Bardin GPL


#include "MemoryLeak.h"
#include "RadiantFireEightSixOne.h"


RadiantFireEightSixOne::RadiantFireEightSixOne(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {
  m_FrameCount = 0;
  LOGV("alloc RadiantFire\n");
  LoadSound(0);
  LoadTexture(0);
  m_Zoom = 3.0;

  m_RequestedFullscreen = false;
  m_Touched = false;
  b2BodyDef spriteBodyDef;
  spriteBodyDef.type = b2_dynamicBody;
  CreateBox2DWorld();
  CreateFoos();

  m_PlayerIndex = 0;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerFoo, NULL));
  m_AtlasSprites[m_PlayerIndex]->m_IsAlive = false;
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 10;
  m_AtlasSprites[m_PlayerIndex]->SetScale(30 / 2.0, 30 / 2.0);
  m_AtlasSprites[m_PlayerIndex]->Build(0);
  m_SpriteCount++;

  m_PlayerRadius = 14.0f;
  m_PlayerIsAwake = false;
  m_PlayerRotation = 0;
  PlayerCreateBox2DBody();
  PlayerUpdateNodePosition();
  PlayerSleep();

  m_Terrain = new Terrain(m_World, m_Textures.at(0));  
  m_Terrain->SetOffsetX(0, m_StateFoo);
}


RadiantFireEightSixOne::~RadiantFireEightSixOne() {
  LOGV("dealloc RadiantFire\n");
  delete m_Terrain;
  delete m_World;
  DestroyFoos();
}


void RadiantFireEightSixOne::CreateFoos() {
  LOGV("RadiantFire::CreateFoos\n");
  ResetStateFoo();
  m_PlayerFoo = AtlasSprite::GetFoo(m_Textures.at(0), 8, 8, 0, 64, 0.0);
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), 9));
  if (m_SimulationTime > 0.0) {
    for (unsigned int i=0; i<m_SpriteCount; i++) {
      if (i == m_PlayerIndex) {
        m_AtlasSprites[i]->ResetFoo(m_PlayerFoo, NULL);
      }
    }
  }
}


void RadiantFireEightSixOne::DestroyFoos() {
  LOGV("RadiantFire::DestroyFoos\n");
  delete m_PlayerFoo;
}


void RadiantFireEightSixOne::CreateBox2DWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0f, -9.8f);
  m_World = new b2World(gravity, false);
}


void RadiantFireEightSixOne::Hit(float x, float y, int hitState) {
  if (hitState == 0) {
    m_Touched = true;
  } else if (hitState == 2) {
    m_Touched = false;
  }
}


int RadiantFireEightSixOne::Simulate() {
  int32 velocityIterations = 2;
  int32 positionIterations = 8;
  
  m_World->Step(m_DeltaTime, velocityIterations, positionIterations);

  if (!m_PlayerIsAwake) {
    PlayerWake();
  }
  if (m_Touched) {
    PlayerDive();
  }
  PlayerLimitVelocity();
  PlayerUpdateNodePosition();
  m_Terrain->SetOffsetX(m_PlayerPosition.x, m_StateFoo);
  

  
  return 1;
}


void RadiantFireEightSixOne::RenderModelPhase() {
}


void RadiantFireEightSixOne::RenderSpritePhase() {
  glTranslatef(m_CameraPosition[0], 0.0, 0.0);
  m_Batches[0]->m_NumBatched = 0;
  m_Terrain->Render(m_StateFoo);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_Batches[0]);
  AtlasSprite::RenderFoo(m_StateFoo, m_Batches[0]);
}


void RadiantFireEightSixOne::PlayerSleep() {
  m_PlayerIsAwake = false;
  m_PlayerBody->SetActive(false);
}


void RadiantFireEightSixOne::PlayerWake() {
  m_PlayerIsAwake = true;
  m_PlayerBody->SetActive(true);
  m_PlayerBody->ApplyLinearImpulse(b2Vec2(10, 10), m_PlayerBody->GetPosition());
}


void RadiantFireEightSixOne::PlayerDive() {
  m_PlayerBody->ApplyForce(b2Vec2(0.5, -40), m_PlayerBody->GetPosition());
}


void RadiantFireEightSixOne::PlayerLimitVelocity() {
  const float minVelocityX = 10;
  const float minVelocityY = -20;
  b2Vec2 vel = m_PlayerBody->GetLinearVelocity();
  if (vel.x < minVelocityX) {
    vel.x = minVelocityX;
  }
  if (vel.y < minVelocityY) {
    vel.y = minVelocityY;
  }
  m_PlayerBody->SetLinearVelocity(vel);
}


void RadiantFireEightSixOne::PlayerUpdateNodePosition() {
  float x = m_PlayerBody->GetPosition().x * PTM_RATIO;
  float y = m_PlayerBody->GetPosition().y * PTM_RATIO;
  m_PlayerPosition = MLPointMake(x, y);
  b2Vec2 vel = m_PlayerBody->GetLinearVelocity();
  float angle = atan2f(vel.y, vel.x);
  m_PlayerRotation = angle;
  m_AtlasSprites[m_PlayerIndex]->m_Rotation = m_PlayerRotation;
  if (y < -m_PlayerRadius && m_PlayerIsAwake) {
    PlayerSleep();
  }
  m_AtlasSprites[m_PlayerIndex]->SetPosition(m_PlayerPosition.x, m_PlayerPosition.y);

  //float new_x = m_Terrain->position.x;
  //m_CameraPosition[0] += (0.95 * (new_x - old_x));
  m_CameraPosition[0] = -m_PlayerPosition.x;
}


void RadiantFireEightSixOne::PlayerReset() {
  m_World->DestroyBody(m_PlayerBody);
  PlayerCreateBox2DBody();
  PlayerSleep();
}


void RadiantFireEightSixOne::PlayerCreateBox2DBody() {
  MLPoint startPosition = MLPointMake(0, 128);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0f;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x / PTM_RATIO, startPosition.y / PTM_RATIO);
  m_PlayerBody = m_World->CreateBody(&bd);
  b2CircleShape shape;
  shape.m_radius = m_PlayerRadius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = 2.0f;
  fd.restitution = 0.0; //bounce
  fd.friction = 0.0;
  m_PlayerBody->CreateFixture(&fd);
}
