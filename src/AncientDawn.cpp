// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SpaceShipDownContactListener.h"
#include "AncientDawn.h"

#define COUNT 12 * 7

AncientDawn::AncientDawn(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {
  LOGV("alloc AncientDawn %d %d %d\n", CONTINUE_LEVEL, RESTART_LEVEL, START_NEXT_LEVEL);
  LoadSound(0);
  LoadTexture(1);
  StartLevel(FirstLevel());
}


AncientDawn::~AncientDawn() {
  LOGV("dealloc AncientDawn\n");
  StopLevel();
}


void AncientDawn::CreateFoos() {

  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), COUNT + 1));

  m_PlayerDraw = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 0, 3, 5.0);
  m_SpaceShipDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
  m_BulletDraw = AtlasSprite::GetFoo(m_Textures.at(0), (16 * 4), (16 * 4), (8 * (16 * 4)) + 7, (8 * (16 * 4)) + 8, 5.0);
  m_LandscapeDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
}


void AncientDawn::DestroyFoos() {
  delete m_PlayerDraw;
  delete m_SpaceShipDraw;
  delete m_BulletDraw;
  delete m_LandscapeDraw;
}


void AncientDawn::StartLevel(int level_index) {
  LOGV("Starting Level: %d\n", level_index);

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
  m_Batch = 0;
  m_BulletSpeed = 10.0;
  m_SimulationTime = 0.0;
  m_ShootTimeout = 0.0;
  m_PhysicsTimeout = 0.0;
  m_DebugDrawToggle = false;
  m_TouchedLeft = false;
  m_TouchedRight = false;
  m_CurrentSound = 0;
  m_PlayerIndex = 0;
  m_Force = false;
}


void AncientDawn::CreateWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0, 0.0);
  m_World = new b2World(gravity, true);
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
  m_ContactListener = new SpaceShipDownContactListener();
  //m_World->SetContactListener(m_ContactListener);
}


void AncientDawn::DestroyContactListener() {
  delete m_ContactListener;
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
  m_AtlasSprites[m_PlayerIndex]->Build(COUNT);
  m_SpriteCount++;

  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO);

  for (int i=0; i<m_AtlasSprites[m_PlayerIndex]->m_NumParticles; i++) {
    AtlasSprite *bullet = m_AtlasSprites[m_PlayerIndex]->m_AtlasSprites[i];
    bullet->m_Fps = 0; 
    //bullet->m_IsAlive = true;
    bullet->SetScale(8.0, 8.0);
    //LOGV("%p %p\n", m_AtlasSprites[m_PlayerIndex], bullet);
    b2BodyDef bd2;
    bd2.type = b2_dynamicBody;
    bd2.allowSleep = true;
    bd2.awake = false;
    bd2.linearDamping = 0.0;
    bd2.fixedRotation = true;
    bd2.position.Set(startPosition.x, startPosition.y);
    b2CircleShape shape2;
    shape2.m_radius = 1.0 / PTM_RATIO;
    //b2EdgeShape shape2;
    //shape2.Set(b2Vec2(-0.5, 0.0), b2Vec2(0.5, 0.0));
    b2FixtureDef fd2;
    fd2.shape = &shape2;
    fd2.isSensor = true;
    fd2.density = 0.0;
    //fd.restitution = 0.0;
    fd2.friction = 0.0;
    //fd2.filter.categoryBits = 0x0002;
    fd2.filter.groupIndex = -1;
    b2Body *bullet_body = m_World->CreateBody(&bd2);
    bullet_body->SetUserData(bullet);
    bullet_body->CreateFixture(&fd2);
    bullet_body->SetActive(true);
  }

  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.awake = false;
  bd.linearDamping = 0.0;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);
  b2CircleShape shape;
  shape.m_radius = 5.0 / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.isSensor = true;
  fd.density = 0.0;
  //fd.restitution = 0.0;
  fd.friction = 0.0;
  //fd.filter.categoryBits = 0x0002;
  fd.filter.groupIndex = -1;
  m_PlayerBody = m_World->CreateBody(&bd);
  m_PlayerBody->SetUserData(m_AtlasSprites[m_PlayerIndex]);
  m_PlayerBody->CreateFixture(&fd);
  m_PlayerBody->SetActive(true);

  
  b2Body *center_body = m_World->CreateBody(&bd);
  b2MouseJointDef mouse_joint_def;
  mouse_joint_def.bodyA = center_body;
  mouse_joint_def.bodyB = m_PlayerBody;
  mouse_joint_def.target = b2Vec2(0.0, 0.0);
  mouse_joint_def.maxForce = 100.0f * m_PlayerBody->GetMass();
  mouse_joint_def.dampingRatio = 100.0;
  mouse_joint_def.frequencyHz = 100.0;
  m_PlayerMouseJoint = (b2MouseJoint *)m_World->CreateJoint(&mouse_joint_def);
}


void AncientDawn::DestroyPlayer() {
}


void AncientDawn::CreateSpaceShip() {
}


void AncientDawn::DestroySpaceShip() {
}


void AncientDawn::CreateLandscape() {
}


void AncientDawn::DestroyLandscape() {
}


int AncientDawn::LevelProgress() {
  if (m_SimulationTime > 60.0) {
    return RESTART_LEVEL;
  }

  return CONTINUE_LEVEL;
}


void AncientDawn::RestartLevel() {
  StopLevel();
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
  m_World->Step(m_DeltaTime, velocityIterations, positionIterations);
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
    //m_BulletSpeed += 1.0;
  }
  //if (hitState == 1) {
    m_PlayerMouseJoint->SetTarget(b2Vec2(xx / PTM_RATIO, yy / PTM_RATIO));
  //}

  if (hitState == 2) {
    m_Force = false;
  } else {
    m_Force = true;
  }
}


int AncientDawn::Simulate() {

  StepPhysics();

  int shot_this_tick = 0;
  m_ShootTimeout += m_DeltaTime;
  float theta = 0.0;
  bool shoot_this_tick = (m_ShootTimeout > (1.0 / 6.0));
  if (shoot_this_tick) {
    m_ShootTimeout = 0.0;
  }

  for (b2Body* body = m_World->GetBodyList(); body; body = body->GetNext()) {
    AtlasSprite *sprite = (AtlasSprite *)body->GetUserData();
    if (sprite != NULL) {
      float x = body->GetPosition().x * PTM_RATIO;
      float y = body->GetPosition().y * PTM_RATIO;
      if (sprite == m_AtlasSprites[m_PlayerIndex]) {
      } else {
        if (sprite->m_IsAlive) {
          if ((sprite->m_Life > (1.0))) {
            body->SetTransform(b2Vec2(sprite->m_Parent->m_Position[0] / PTM_RATIO, sprite->m_Parent->m_Position[1] / PTM_RATIO), 0.0);
            body->SetAwake(false);
            sprite->m_IsAlive = false;
          }
        } else {
          if (shoot_this_tick && shot_this_tick < (12)) {
            body->SetTransform(b2Vec2(sprite->m_Parent->m_Position[0] / PTM_RATIO, sprite->m_Parent->m_Position[1] / PTM_RATIO), 0.0);
            // x = r cos theta,
            // y = r sin theta, 
            float off = fastSinf(m_SimulationTime * 0.1) * 0.33;
            //float off = 0.0;
            float fx = m_BulletSpeed * fastSinf((M_PI / 2.0) - (theta + off));
            float fy = m_BulletSpeed * fastSinf((theta + off));
            body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
            //body->ApplyForce(b2Vec2(10.0, 0), body->GetPosition());
            sprite->m_IsAlive = true;
            sprite->m_Life = 0.0;
            sprite->m_Frame = 0;
            shot_this_tick++;
            theta += ((M_PI * 2.0) / 12.0);
          }
        }
      }
      UpdatePhysicialPositionOfSprite(sprite, x, y);
      sprite->Simulate(m_DeltaTime);
    }
  }

  switch(LevelProgress()) {
    case CONTINUE_LEVEL:
      return 1;

    case RESTART_LEVEL:
      RestartLevel();
      return 1;

    case START_NEXT_LEVEL:
      StartNextLevel();
      return 1;
      
    default:
      return 0;
  }
}


void AncientDawn::RenderModelPhase() {
}


void AncientDawn::RenderSpritePhase() {
  if (true) {
    m_Batches[0]->m_NumBatched = 0;
    RenderSpriteRange(0, 1, m_Batches[0]);
    AtlasSprite::RenderFoo(m_StateFoo, m_Batches[0]);
  } else {
    AtlasSprite::ReleaseBuffers();
    ResetStateFoo();
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    //glColor4f(1.0, 0.0, 0.0, 1.0);
    m_World->DrawDebugData();
    //glColor4f(1.0, 1.0, 1.0, 1.0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
  }
}
