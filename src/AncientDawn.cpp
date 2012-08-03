// Jon Bardin GPL


#include "MemoryLeak.h"
#include "AncientDawn.h"
#include "MetalWolfParams.h"
#include "Bridge.h"

std::string string_format(const std::string &fmt, ...) {
  int size=100;
  std::string str;
  va_list ap;
  while (1) {
    str.resize(size);
    va_start(ap, fmt);
    int n = vsnprintf((char *)str.c_str(), size, fmt.c_str(), ap);
    va_end(ap);
    if (n > -1 && n < size) {
      str.resize(n);
      return str;
    }
    if (n > -1)
      size=n+1;
    else
      size*=2;
  }
}


static AncientDawn *game;

void start_game(const char *s) {
  LOGV("starting game");
  game->StartLevel(game->FirstLevel());
}


#define COUNT 18 * 10

AncientDawn::AncientDawn(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) 
: Engine(w, h, t, m, l, s)
, m_PlayerHealth (MWParams::kPlayerStartHeatlh)
, m_EnemyHealth(MWParams::kEnemyStartingHealth)
, mpBulletCommandPlayer(NULL)
, bc(NULL)
, mbGameStarted(false)
, m_EnemyBody(NULL)
, m_PlayerBulletIsLaser(false)
{
  //LoadSound(0);
  LoadTexture(0);
  LoadTexture(1);
  game = this;
}


AncientDawn::~AncientDawn() {
  LOGV("dealloc AncientDawn\n");
  StopLevel();
}


void AncientDawn::CreateFoos() {
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(0), 32));
  m_Batches.push_back(AtlasSprite::GetBatchFoo(m_Textures.at(1), (COUNT * 2) + 2));
  
  m_PlayerDraw = AtlasSprite::GetFoo(m_Textures.at(1), 16, 16, 0, 3, 5.0);
  m_SpaceShipDraw = AtlasSprite::GetFoo(m_Textures.at(1), 1, 2, 1, 2, 0.0);
  m_BulletDraw = AtlasSprite::GetFoo(m_Textures.at(1), (16 * 4), (16 * 4), (8 * (16 * 4)) + 5, (8 * (16 * 4)) + 8, 5.0);
  m_SpaceShipBulletDraw = AtlasSprite::GetFoo(m_Textures.at(1), (16 * 4), (16 * 4), (9 * (16 * 4)) + 5, (9 * (16 * 4)) + 7, 5.0);

  m_LandscapeDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
  
  //TODO: reset foos on restart for android
  /*
  if (m_SimulationTime > 0.0) {
    for (int i=0; i<m_SpriteCount; i++) {
      m_AtlasSprites[i]->ResetFoo(m_PlayerDraw, m_BulletDraw);
    }
  }
  */
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
  CreateSpaceShip();
  CreatePlayer();
  CreateLandscape();
}


void AncientDawn::ResetGame() {
  m_WebViewTimeout = 0;
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
  m_TouchOffsetX = 0;
  m_TouchOffsetY = 0;
  m_LastRecycledIndex = -1;
  
  //Initialize Player
  m_PlayerHealth = MWParams::kPlayerStartHeatlh;
  
  //Initilize Game State
  mbGameStarted = true;
  
  m_JavascriptTick = "";
  
  m_LastBulletCommandTurn = -1;
  
  m_PlayerBulletIsLaser = (MWParams::kPlayerGunMLFileIndex >= EPlayerLaserMLFileName_LVL1 && MWParams::kPlayerGunMLFileIndex < EPlayerLaserMLFileName_UPTO_COUNT);
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


void AncientDawn::StopLevel() {
  DestroyFoos();
  ClearSprites();
  DestroyWorld();
  DestroyDebugDraw();
  DestroyPlayer();
  DestroySpaceShip();
  DestroyLandscape();
  
  mbGameStarted = false;
}


void AncientDawn::CreatePlayer() {

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerDraw, m_BulletDraw));
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 60;
  m_AtlasSprites[m_PlayerIndex]->m_IsAlive = true;
  m_AtlasSprites[m_PlayerIndex]->SetPosition(MWParams::kPlayerStartX,
                                             MWParams::kPlayerStartY);
  m_AtlasSprites[m_PlayerIndex]->SetScale(20.0, 20.0);
  m_AtlasSprites[m_PlayerIndex]->Build(COUNT);
  m_SpriteCount++;

  MLPoint startPosition = MLPointMake(MWParams::kPlayerStartX / PTM_RATIO, MWParams::kPlayerStartY / PTM_RATIO);
  
  //Creating bullet bodies for the player's ship
  for (int i=0; i<m_AtlasSprites[m_PlayerIndex]->m_NumParticles; i++) {
    AtlasSprite *bullet = m_AtlasSprites[m_PlayerIndex]->m_AtlasSprites[i];
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
  

  //This the body for the player ship
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
  
  fseek(m_LevelFileHandles->at(MWParams::kPlayerGunMLFileIndex)->fp, m_LevelFileHandles->at(MWParams::kPlayerGunMLFileIndex)->off, 0);

  BulletMLParser* bp = new BulletMLParserTinyXML(m_LevelFileHandles->at(MWParams::kPlayerGunMLFileIndex)->fp, m_LevelFileHandles->at(MWParams::kPlayerGunMLFileIndex)->len);
  bp->build();
  mpBulletCommandPlayer = new BulletCommand(bp, m_AtlasSprites[m_PlayerIndex]);
  
  b2Body *center_body = m_World->CreateBody(&bd);
  b2MouseJointDef mouse_joint_def;
  mouse_joint_def.bodyA = center_body;
  mouse_joint_def.bodyB = m_PlayerBody;
  mouse_joint_def.target = b2Vec2(startPosition.x, startPosition.y);
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
  m_AtlasSprites[m_SpaceShipIndex]->SetPosition(MWParams::kEnemyStartX, MWParams::kEnemyStartY);
  m_AtlasSprites[m_SpaceShipIndex]->SetScale(MWParams::kEnemyHalfPixelDimX, MWParams::kEnemyHalfPixelDimY);
  m_AtlasSprites[m_SpaceShipIndex]->Build(COUNT);
  m_SpriteCount++;

  
  MLPoint startPosition = MLPointMake(MWParams::kEnemyStartX / PTM_RATIO, MWParams::kEnemyStartY / PTM_RATIO);

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
 
  fseek(m_LevelFileHandles->at(EEnemyBulletMLFileIndex_ENEMY)->fp, m_LevelFileHandles->at(EEnemyBulletMLFileIndex_ENEMY)->off, 0);

  BulletMLParser* bp = new BulletMLParserTinyXML(m_LevelFileHandles->at(EEnemyBulletMLFileIndex_ENEMY)->fp, m_LevelFileHandles->at(EEnemyBulletMLFileIndex_ENEMY)->len);
  bp->build();
  bc = new BulletCommand(bp, m_AtlasSprites[m_SpaceShipIndex]);

}


void AncientDawn::DestroySpaceShip() {
}


void AncientDawn::CreateLandscape() {
  m_LandscapeStartIndex = m_SpriteCount;
  for (unsigned int i=0; i<3; i++) {
    m_AtlasSprites.push_back(new SpriteGun(m_LandscapeDraw, NULL));
    m_AtlasSprites[m_SpriteCount]->m_Fps = 0;
    m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
    m_AtlasSprites[m_SpriteCount]->SetPosition(0, i * 512);
    m_AtlasSprites[m_SpriteCount]->SetScale(512, 512);
    m_AtlasSprites[m_SpriteCount]->Build(0);
    m_SpriteCount++;
  }
  m_LandscapeStopIndex = m_SpriteCount;
}


void AncientDawn::DestroyLandscape() {
}


int AncientDawn::LevelProgress() {
  if (m_SimulationTime > MWParams::kNextLevelTime || m_PlayerHealth == 0.0f) {
    return END_LEVEL; //TODO: Should Kick to Results Screen
  }

  return CONTINUE_LEVEL;
}


void AncientDawn::RestartLevel() {
  LOGV("Restarting Level\n");
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
  m_SolveTimeout += m_DeltaTime;
  m_World->m_Solve = (m_SolveTimeout > 0.125);

  m_World->Step(m_DeltaTime, velocityIterations, positionIterations);
    
  if (m_World->m_Solve) {
    m_SolveTimeout = 0.0;
    m_ColliderSwitch = COLLIDE_PLAYER;
    aabb.lowerBound.Set((-10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO), (-10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO));
    aabb.upperBound.Set((10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO), (10.0f / PTM_RATIO) + (m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO));
    m_World->QueryAABB(this, aabb);
    
    {
      m_ColliderSwitch = COLLIDE_ENEMY;
      aabb.lowerBound.Set((-MWParams::kEnemyHalfPixelDimX/PTM_RATIO) + (m_AtlasSprites[m_SpaceShipIndex]->m_Position[0]/PTM_RATIO),
                          (-MWParams::kEnemyHalfPixelDimY/PTM_RATIO) + (m_AtlasSprites[m_SpaceShipIndex]->m_Position[1]/PTM_RATIO));
      aabb.upperBound.Set((MWParams::kEnemyHalfPixelDimX/PTM_RATIO) + (m_AtlasSprites[m_SpaceShipIndex]->m_Position[0]/PTM_RATIO),
                          (MWParams::kEnemyHalfPixelDimY/PTM_RATIO) + (m_AtlasSprites[m_SpaceShipIndex]->m_Position[1]/PTM_RATIO));
      m_World->QueryAABB(this, aabb);
    }
                        
    
    {
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

  if(hitState == 2 || hitState == -1)
  {
    //if the player is touch up(hitstate is 2) or touch cancled(hit state -1) then we stop shooting
    mpBulletCommandPlayer->EnableShooting(false);
  }
  else
  {
    //Otherwise the player has thier finger on the screen and we are shooting
    mpBulletCommandPlayer->EnableShooting(true);
  }
}


int AncientDawn::Simulate() {

  //Java Script is always simulated, regardless if the game is started.
  m_WebViewTimeout += m_DeltaTime;
  if (m_WebViewTimeout > (0.33)) {
    push_pop_function(m_JavascriptTick.c_str());
    m_WebViewTimeout = 0.0;
    m_JavascriptTick = "";
  }

  return _gameSimulate();
}

int AncientDawn::_gameSimulate()
{
  if(!mbGameStarted)
  {
    //Don't not run any simulation for a game that is not started
    return 0;
  }
  
  int chosen_state = 0;
  
  switch(LevelProgress()) {
    case CONTINUE_LEVEL:
      chosen_state = 1;
      break;
    
    case END_LEVEL:
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

  int this_bulletml_turn = (int)(m_SimulationTime * 100.0);
  if (this_bulletml_turn != m_LastBulletCommandTurn) {  
    if (bc) {
      bc->run(this_bulletml_turn);
    }
    
    if(mpBulletCommandPlayer)
    {
      mpBulletCommandPlayer->run(this_bulletml_turn);
    }
    m_LastBulletCommandTurn = this_bulletml_turn;
  }

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
          body->SetAwake(false);
        }
      }
    }
  }
  
  for (unsigned int i=m_LandscapeStartIndex; i<m_LandscapeStopIndex; i++) {
    float landscape_speed = -500.0;
    //m_AtlasSprites[i]->m_Position[0] += landscape_speed * m_DeltaTime;
    m_AtlasSprites[i]->m_Position[1] += landscape_speed * m_DeltaTime;
    if (m_AtlasSprites[i]->m_Position[1] < -(m_ScreenHeight * 0.75)) {
      m_AtlasSprites[i]->m_Position[1] = (m_ScreenHeight * 0.75);
    }
  }
  
  if (m_PlayerBulletIsLaser) {
    for (unsigned int i=0; i<m_AtlasSprites[m_PlayerIndex]->m_NumParticles; i++) {
      b2Body *body = (b2Body *)m_AtlasSprites[m_PlayerIndex]->m_AtlasSprites[i]->m_UserData;
      b2Vec2 new_pos = b2Vec2(m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO, body->GetPosition().y);
      body->SetTransform(new_pos, 0);
    }
  }
  
  return chosen_state;
}


bool AncientDawn::ReportFixture(b2Fixture* fixture) {
  b2Body* body = fixture->GetBody();
  AtlasSprite *sprite = (AtlasSprite *)body->GetUserData();
 
  switch (m_ColliderSwitch) {
    case COLLIDE_PLAYER:
    {
      bool bCollidingSpriteIsBullet = (sprite->m_IsAlive && 
                                        sprite != m_AtlasSprites[m_PlayerIndex] && 
                                        sprite->m_Parent != m_AtlasSprites[m_PlayerIndex]);
      if (bCollidingSpriteIsBullet) 
      {
        sprite->m_IsAlive = false;
        m_PlayerHealth = MAX(0.0f, m_PlayerHealth - MWParams::kEnemyBulletDamageAmount);
        m_JavascriptTick += string_format("player_health = %d;", (int)m_PlayerHealth);
      }
    }
    break;
    
    case COLLIDE_ENEMY:
    {
        bool bCollidingSpriteIsPlayerBullet = (sprite->m_IsAlive &&
                                                sprite->m_Parent == m_AtlasSprites[m_PlayerIndex]);
        if(bCollidingSpriteIsPlayerBullet)
        {
            sprite->m_IsAlive = false;
            m_EnemyHealth = MAX(0.0f, m_EnemyHealth - MWParams::kPlayerBulletDamage);
            m_JavascriptTick += string_format("enemy_health = %d;", (int)m_EnemyHealth);
        }
    }
    break;

    case COLLIDE_CULLING:
      if (sprite->m_IsAlive && sprite != m_AtlasSprites[m_PlayerIndex] && sprite->m_Parent != m_AtlasSprites[m_PlayerIndex]) {
        sprite->m_IsAlive = false;
      }
      break;
  }

  return true;
  
}


void AncientDawn::RenderModelPhase() {
}


void AncientDawn::RenderSpritePhase() {
  if(!mbGameStarted)
  {
    return;
  }
  
  m_Batches[0]->m_NumBatched = 0;
  RenderSpriteRange(m_LandscapeStartIndex, m_LandscapeStopIndex, m_Batches[0]);
  AtlasSprite::RenderFoo(m_StateFoo, m_Batches[0]);
  
  m_Batches[1]->m_NumBatched = 0;
  RenderSpriteRange(0, 2, m_Batches[1]);
  AtlasSprite::RenderFoo(m_StateFoo, m_Batches[1]);
}
