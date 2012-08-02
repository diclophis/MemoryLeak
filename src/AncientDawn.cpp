// Jon Bardin GPL


#include "MemoryLeak.h"

#include "AncientDawn.h"

#define COUNT 18 * 128

AncientDawn::AncientDawn(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(0);
  LoadTexture(1);
  StartLevel(FirstLevel());
}


AncientDawn::~AncientDawn() {
  LOGV("dealloc AncientDawn\n");
  StopLevel();
}


void AncientDawn::CreateFoos() {
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), (COUNT * 2) + 2));
  m_PlayerDraw = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 0, 3, 5.0);
  m_SpaceShipDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 2, 1, 2, 0.0);
  m_BulletDraw = AtlasSprite::GetFoo(m_Textures.at(0), (16 * 4), (16 * 4), (8 * (16 * 4)) + 5, (8 * (16 * 4)) + 8, 5.0);
  m_SpaceShipBulletDraw = AtlasSprite::GetFoo(m_Textures.at(0), (16 * 4), (16 * 4), (9 * (16 * 4)) + 5, (9 * (16 * 4)) + 7, 5.0);

  m_LandscapeDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
  if (m_SimulationTime > 0.0) {
    for (int i=0; i<m_SpriteCount; i++) {
      m_AtlasSprites[i]->ResetFoo(m_PlayerDraw, m_BulletDraw);
    }
  }
}


void AncientDawn::DestroyFoos() {
  delete m_PlayerDraw;
  delete m_SpaceShipDraw;
  delete m_BulletDraw;
  delete m_LandscapeDraw;
  for (std::vector<foofoo *>::iterator i = m_Batches.begin(); i != m_Batches.end(); ++i) {
    delete (*i);
  }
  m_Batches.clear();
  ResetStateFoo();
}


void AncientDawn::StartLevel(int level_index) {
  m_CurrentLevel = level_index;

  ResetStateFoo();
  ResetGame();
  CreateFoos();
  CreateWorld();
  CreateDebugDraw();
  CreateContactListener();
  CreatePlayer();
  CreateSpaceShip();
  CreateLandscape();
}


void AncientDawn::ResetGame() {
  m_Zoom = 1.0;
  m_Batch = 0;
  m_BulletSpeed = 10.0;
  m_SimulationTime = 0.0;
  m_ShootTimeout = 0.0;
  m_BossShootTimeout = 0.0;
  m_PhysicsTimeout = 0.0;
  m_DebugDrawToggle = false;
  m_TouchedLeft = false;
  m_TouchedRight = false;
  m_CurrentSound = 0;
  m_PlayerIndex = 0;
  m_SpaceShipIndex = 0;
  m_Force = false;
  m_TouchOffsetX = 0;
  m_TouchOffsetY = 0;
  m_LastRecycledIndex = -1;
}


void AncientDawn::CreateWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0, 0.0);
  m_World = new BulletHellWorld(gravity, false);
}


void AncientDawn::DestroyWorld() {
  delete m_World;
}


void AncientDawn::CreateDebugDraw() {
  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  flags += b2Draw::e_jointBit;
  flags += b2Draw::e_aabbBit;
  flags += b2Draw::e_pairBit;
  flags += b2Draw::e_centerOfMassBit;
  m_DebugDraw = new GLESDebugDraw(PTM_RATIO);
  m_World->SetDebugDraw(m_DebugDraw);
  m_DebugDraw->SetFlags(flags);
}


void AncientDawn::DestroyDebugDraw() {
  delete m_DebugDraw;
}


void AncientDawn::CreateContactListener() {
  //TODO: do we want a contact listener?
  //m_ContactListener = new SpaceShipDownContactListener();
}


void AncientDawn::DestroyContactListener() {
  //delete m_ContactListener;
}


void AncientDawn::StopLevel() {
  DestroyFoos();
  ClearSprites();
  DestroyWorld();
  DestroyDebugDraw();
  DestroyContactListener();
  DestroyPlayer();
  DestroySpaceShip();
  DestroyLandscape();
}


void AncientDawn::CreatePlayer() {

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerDraw, m_BulletDraw));
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 60;
  m_AtlasSprites[m_PlayerIndex]->m_IsAlive = true;
  m_AtlasSprites[m_PlayerIndex]->SetPosition(0.0, 0.0);
  m_AtlasSprites[m_PlayerIndex]->SetScale(20.0, 20.0);
  m_AtlasSprites[m_PlayerIndex]->Build(18);
  m_SpriteCount++;

  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO);

  /*
  for (int i=0; i<m_AtlasSprites[m_PlayerIndex]->m_NumParticles; i++) {
    AtlasSprite *bullet = m_AtlasSprites[m_PlayerIndex]->m_AtlasSprites[i];
    bullet->m_Fps = 24; 
    bullet->SetScale(10.0, 50.0);
    b2BodyDef bd2;
    bd2.type = b2_dynamicBody;
    bd2.allowSleep = false;
    bd2.awake = false;
    bd2.linearDamping = 0.0;
    bd2.fixedRotation = true;
    bd2.position.Set(startPosition.x, startPosition.y);
    b2CircleShape shape2;
    shape2.m_radius = 5.0 / PTM_RATIO;
    b2FixtureDef fd2;
    fd2.shape = &shape2;
    fd2.isSensor = true;
    fd2.density = 0.0;
    fd2.friction = 0.0;
    fd2.filter.groupIndex = -1;
    b2Body *bullet_body = m_World->CreateBody(&bd2);
    bullet_body->SetUserData(bullet);
    bullet_body->CreateFixture(&fd2);
    //bullet_body->SetActive(true);
  }
  */

  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.awake = false;
  bd.linearDamping = 0.0;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);
  b2CircleShape shape;
  shape.m_radius = 10.0 / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.isSensor = true;
  fd.density = 0.0;
  fd.friction = 0.0;
  fd.filter.groupIndex = -1;
  m_PlayerBody = m_World->CreateBody(&bd);
  m_PlayerBody->SetUserData(m_AtlasSprites[m_PlayerIndex]);
  m_PlayerBody->CreateFixture(&fd);
  
  b2Body *center_body = m_World->CreateBody(&bd);
  b2MouseJointDef mouse_joint_def;
  mouse_joint_def.bodyA = center_body;
  mouse_joint_def.bodyB = m_PlayerBody;
  mouse_joint_def.target = b2Vec2(0.0, 0.0);
  mouse_joint_def.maxForce = 1000000.0f * m_PlayerBody->GetMass();
  mouse_joint_def.dampingRatio = 0.0;
  mouse_joint_def.frequencyHz = 100.0;
  m_PlayerMouseJoint = (b2MouseJoint *)m_World->CreateJoint(&mouse_joint_def);
}


void AncientDawn::DestroyPlayer() {
}


void AncientDawn::CreateSpaceShip() {
  m_SpaceShipIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_SpaceShipDraw, m_SpaceShipBulletDraw));
  m_AtlasSprites[m_SpaceShipIndex]->m_Fps = 0;
  m_AtlasSprites[m_SpaceShipIndex]->m_IsAlive = true;
  m_AtlasSprites[m_SpaceShipIndex]->SetPosition(50.0, 50.0);
  m_AtlasSprites[m_SpaceShipIndex]->SetScale(175.0, 100.0);
  m_AtlasSprites[m_SpaceShipIndex]->Build(COUNT);
  m_SpriteCount++;

  
  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_SpaceShipIndex]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_SpaceShipIndex]->m_Position[1] / PTM_RATIO);

  for (int i=0; i<m_AtlasSprites[m_SpaceShipIndex]->m_NumParticles; i++) {
    AtlasSprite *bullet = m_AtlasSprites[m_SpaceShipIndex]->m_AtlasSprites[i];
    bullet->m_Fps = 0; 
    bullet->SetScale(8.0, 8.0);
    b2BodyDef bd2;
    bd2.type = b2_dynamicBody;
    bd2.allowSleep = false;
    bd2.awake = false;
    bd2.linearDamping = 0.0;
    bd2.fixedRotation = true;
    bd2.position.Set(startPosition.x, startPosition.y);
    b2CircleShape shape2;
    shape2.m_radius = 1.0 / PTM_RATIO;
    b2FixtureDef fd2;
    fd2.shape = &shape2;
    fd2.isSensor = true;
    fd2.density = 0.0;
    fd2.friction = 0.0;
    fd2.filter.groupIndex = -1;
    b2Body *bullet_body = m_World->CreateBody(&bd2);
    bullet_body->SetUserData(bullet);
    bullet_body->CreateFixture(&fd2);
    bullet->m_UserData = bullet_body;
  }

 
  fseek(m_LevelFileHandles->at(0)->fp, m_LevelFileHandles->at(0)->off, 0);

  BulletMLParser* bp = new BulletMLParserTinyXML(m_LevelFileHandles->at(0)->fp, m_LevelFileHandles->at(0)->len);
  bp->build();
  bc = new BulletCommand(bp, m_AtlasSprites[m_SpaceShipIndex]);

}


void AncientDawn::DestroySpaceShip() {
}


void AncientDawn::CreateLandscape() {
}


void AncientDawn::DestroyLandscape() {
}


int AncientDawn::LevelProgress() {
  if (m_SimulationTime > 120.0) {
    return RESTART_LEVEL;
  }

  return CONTINUE_LEVEL;
}


void AncientDawn::RestartLevel() {
  StopLevel();
  //if (m_CurrentLevel > 0) {
  //  StopLevel();
  //} else {
  //  m_CurrentLevel = -1;
  //}
  StartLevel(m_CurrentLevel);
}


void AncientDawn::StartNextLevel() {
  StopLevel();
  StartLevel(NextLevel());
}


int AncientDawn::NextLevel() {
  return 0;
}


int AncientDawn::FirstLevel() {
  return 0;
}


void AncientDawn::StepPhysics() {
  int velocityIterations = 1;
  int positionIterations = 1;
  m_SolveTimeout += m_DeltaTime;
  m_World->m_Solve = true; //(m_SolveTimeout > 0.5);

  m_World->Step(m_DeltaTime, velocityIterations, positionIterations);
    
  if (m_World->m_Solve) {
    m_SolveTimeout = 0.0;
    m_ColliderSwitch = COLLIDE_PLAYER;
    aabb.lowerBound.Set((-10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO), (-10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO));
    aabb.upperBound.Set((10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO), (10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO));
    m_World->QueryAABB(this, aabb);
    
    m_ColliderSwitch = COLLIDE_CULLING;
    
    aabb.lowerBound.Set(((m_ScreenWidth * 0.5) / PTM_RATIO), (-(m_ScreenHeight * 0.5) / PTM_RATIO));
    aabb.upperBound.Set((((m_ScreenWidth * 0.5) + 150.0) / PTM_RATIO), ((m_ScreenHeight * 0.5) / PTM_RATIO));
    m_World->QueryAABB(this, aabb);
    
    aabb.lowerBound.Set((((-m_ScreenWidth * 0.5) - 150.0) / PTM_RATIO), ((-m_ScreenHeight * 0.5) / PTM_RATIO));
    aabb.upperBound.Set(((-m_ScreenWidth * 0.5) / PTM_RATIO), ((m_ScreenHeight * 0.5) / PTM_RATIO));
    m_World->QueryAABB(this, aabb);
    
    aabb.lowerBound.Set((-(m_ScreenWidth * 0.5) / PTM_RATIO), ((m_ScreenHeight * 0.5) / PTM_RATIO));
    aabb.upperBound.Set((((m_ScreenWidth * 0.5)) / PTM_RATIO), (((m_ScreenHeight * 0.5) + 150.0) / PTM_RATIO));
    m_World->QueryAABB(this, aabb);
    
    aabb.lowerBound.Set((-(m_ScreenWidth * 0.5) / PTM_RATIO), (((-m_ScreenHeight * 0.5) - 150.0) / PTM_RATIO));
    aabb.upperBound.Set((((m_ScreenWidth * 0.5)) / PTM_RATIO), (((-m_ScreenHeight * 0.5)) / PTM_RATIO));
    m_World->QueryAABB(this, aabb);
  }
}


void AncientDawn::UpdatePhysicialPositionOfSprite(AtlasSprite *sprite, float x, float y) {
  sprite->SetPosition(x, y);
}


b2Body *AncientDawn::BodyCollidingWithPlayer(b2Body *a, b2Body *b) {
  return NULL;
}


void AncientDawn::Hit(float x, float y, int hitState) {
  float xx = (((x) - (0.5 * (m_ScreenWidth)))) * m_Zoom;
	float yy = ((0.5 * (m_ScreenHeight) - (y))) * m_Zoom;
  if (hitState == 0) {
    m_TouchOffsetX = (xx - m_AtlasSprites[m_PlayerIndex]->m_Position[0]);
    m_TouchOffsetY = (yy - m_AtlasSprites[m_PlayerIndex]->m_Position[1]);
  }
  
  m_PlayerMouseJoint->SetTarget(b2Vec2((xx - m_TouchOffsetX) / PTM_RATIO, (yy - m_TouchOffsetY) / PTM_RATIO));

  if (hitState == 2) {
    m_Force = false;
  } else {
    m_Force = true;
  }
}


int AncientDawn::Simulate() {

  int chosen_state = 0;
  
  switch(LevelProgress()) {
    case CONTINUE_LEVEL:
      chosen_state = 1;
      break;
      
    case RESTART_LEVEL:
      RestartLevel();
      chosen_state = 1;
      return chosen_state;
      
    case START_NEXT_LEVEL:
      chosen_state = 1;
      return chosen_state;
      
    default:
      return chosen_state;
  }
  
  StepPhysics();

  if (bc != NULL) {
    bc->run((int)(m_SimulationTime * 100.0));
  }

  /*
  { // bullets
    int shot_this_tick = 0;
    int boss_shot_this_tick = 0;

    m_ShootTimeout += m_DeltaTime;
    m_BossShootTimeout += m_DeltaTime;
    
    float theta = 0.0;
    bool shoot_this_tick = (m_ShootTimeout > (1.0 / 10.0));
    if (shoot_this_tick) {
      m_ShootTimeout = 0.0;
    }
    
    bool boss_shoot_this_tick = (m_BossShootTimeout > (1.0 / 10.0));
    if (boss_shoot_this_tick) {
      m_BossShootTimeout = 0.0;
    }
    
    float spread = -(M_PI * 0.5);

    int last_used_index = 0;

    for (b2Body* body = m_World->GetBodyList(); body; body = body->GetNext()) {
      AtlasSprite *sprite = (AtlasSprite *)body->GetUserData();
      if (sprite != NULL) {
        if (sprite == m_AtlasSprites[m_PlayerIndex]) {
        } else if (sprite == m_AtlasSprites[m_SpaceShipIndex]) {
        } else {
          last_used_index++;
          if (sprite->m_Parent == m_AtlasSprites[m_PlayerIndex]) {
            if (shoot_this_tick && (last_used_index > m_LastRecycledIndex) && (!sprite->m_IsAlive) && shot_this_tick < 3) {
              body->SetAwake(false);
              body->SetTransform(b2Vec2(sprite->m_Parent->m_Position[0] / PTM_RATIO, sprite->m_Parent->m_Position[1] / PTM_RATIO), 0.0);
              float fx = (spread + (0.5 * M_PI * ((float)shot_this_tick))) * 2.0;
              float fy = 50.0;
              body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
              sprite->m_IsAlive = true;
              sprite->m_Life = 0.0;
              sprite->m_Frame = 0;
              shot_this_tick++;
              m_LastRecycledIndex = last_used_index;
              if (m_LastRecycledIndex >= (COUNT)) {
                m_LastRecycledIndex = -1;
              }
            } else if ((sprite->m_Life > (0.3))) { //bullet live time
              sprite->m_IsAlive = false;
              body->SetAwake(false);
            }
          } else {
            if (boss_shoot_this_tick && !sprite->m_IsAlive && boss_shot_this_tick < 10) {
              sprite->m_Scale[0] = 10.0;
              sprite->m_Scale[1] = 10.0;
              body->SetAwake(false);
              body->SetTransform(b2Vec2(sprite->m_Parent->m_Position[0] / PTM_RATIO, sprite->m_Parent->m_Position[1] / PTM_RATIO), 0.0);
              // x = r cos theta,
              // y = r sin theta, 
              float off = m_SimulationTime * 2.0;
              float fx = m_BulletSpeed * fastSinf((M_PI / 2.0) - (theta + off));
              float fy = m_BulletSpeed * fastSinf((theta + off));
              body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
              sprite->m_IsAlive = true;
              sprite->m_Life = 0.0;
              sprite->m_Frame = 0;
              boss_shot_this_tick++;
              theta += (M_PI * 2.0) / 10.0;
            } else if ((sprite->m_Life > (10.0))) {
              body->SetAwake(false);
              sprite->m_IsAlive = false;
            }
          }
        }
      }
    }
  }
  */

  { // graphics
    for (b2Body* body = m_World->GetBodyList(); body; body = body->GetNext()) {
      AtlasSprite *sprite = (AtlasSprite *)body->GetUserData();
      if (sprite != NULL) {
        if (sprite->m_IsAlive) {
          float x = body->GetPosition().x * PTM_RATIO;
          float y = body->GetPosition().y * PTM_RATIO;
          UpdatePhysicialPositionOfSprite(sprite, x, y);
          sprite->Simulate(m_DeltaTime);
        } else {
          //body->SetTransform(b2Vec2(sprite->m_Parent->m_Position[0] / PTM_RATIO, sprite->m_Parent->m_Position[1] / PTM_RATIO), 0.0);
          body->SetAwake(false);
        }
      }
    }
  }
  
  return chosen_state;
}


bool AncientDawn::ReportFixture(b2Fixture* fixture) {
  
  b2Body* body = fixture->GetBody();
  AtlasSprite *sprite = (AtlasSprite *)body->GetUserData();
 
  switch (m_ColliderSwitch) {
    case COLLIDE_PLAYER:
      if (sprite->m_IsAlive && sprite != m_AtlasSprites[m_PlayerIndex] && sprite->m_Parent != m_AtlasSprites[m_PlayerIndex]) {
        sprite->m_Scale[0] = 40.0;
        sprite->m_Scale[1] = 40.0;
      }
      break;

    case COLLIDE_CULLING:
      if (sprite->m_IsAlive && sprite != m_AtlasSprites[m_PlayerIndex] && sprite->m_Parent != m_AtlasSprites[m_PlayerIndex]) {
        //body->SetTransform(b2Vec2(sprite->m_Parent->m_Position[0] / PTM_RATIO, sprite->m_Parent->m_Position[1] / PTM_RATIO), 0.0);
        //body->SetAwake(false);
        sprite->m_IsAlive = false;
      }
      break;
  }

  return true;
  
}


void AncientDawn::RenderModelPhase() {
}


void AncientDawn::RenderSpritePhase() {
  m_Batches[0]->m_NumBatched = 0;
  RenderSpriteRange(0, 2, m_Batches[0]);
  AtlasSprite::RenderFoo(m_StateFoo, m_Batches[0]);
}

