// Jon Bardin GPL



#include "MemoryLeak.h"
#include "OpenSteer/Draw.h"
#include "SpaceShipDown.h"

#define GRAVITY -100.0
#define PART_DENSITY 5.75
#define PART_FRICTION 0.5
#define PLAYER_DENSITY 7.0
#define PLAYER_FRICTION 0.25
#define ENEMY_DENSITY 0.0
#define ENEMY_FRICTION 0.0
#define PLAYER_MAX_VELOCITY_X 5.0
#define PLAYER_MAX_VELOCITY_Y 5.0
#define BLOCK_WIDTH 54.0
#define PLAYER_AFTERBURNER_COUNT 128
#define ROCKET_AFTERBURNER_COUNT 128

using namespace OpenSteer;

std::vector<EnemyVehicle*> g_EnemyVehicles;
SOG BaseVehicle::allObstacles;
int BaseVehicle::obstacleCount = 0;
const Vec3 g_HomeBaseCenter(0, 0, 0);
const float g_HomeBaseRadius = 1.0;
const float g_ObstacleRadius = 5.0;
const float g_MinStartRadius = 1000;
const float g_MaxStartRadius = 2000;
const float g_BrakingRate = 0.5;
const float g_AvoidancePredictTimeMin  = 0.1f;
const float g_AvoidancePredictTimeMax  = 0.5;
float g_AvoidancePredictTime = g_AvoidancePredictTimeMin;


SpaceShipDown::SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(0);
  LoadSound(1);
  LoadSound(2);
  LoadModel(0, 0, 1);
  LoadModel(1, 0, 1);
  LoadModel(2, 0, 1);
  LoadModel(3, 0, 1);
  LoadModel(4, 0, 1);
  CreateFoos();
  StartLevel(0);
}


void SpaceShipDown::CreateFoos() {


  ResetStateFoo();
  //BLOCK_WIDTH * 1.2, BLOCK_WIDTH * 1.2
  m_PlayerFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 1, 2, 0.0);
  m_PlayerAfterburnerFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 3, 4, 1.0);
  m_SpaceShipPartBaseFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 4, 5, 0.0);
  m_SpaceShipPartTopFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 6, 7, 0.0);
  m_SpaceShipPartMiddleFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 5, 6, 0.0);
  m_SpaceShipPartAfterburnerFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 7, 8, 1.0);
  m_DropZoneFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 8, 16, 1.0);
  m_PlatformFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 0, 1, 0.0);
  m_EnemyFoo = AtlasSprite::GetFoo(m_Textures->at(3), 4, 4, 2, 3, 1.0);
  m_BatchFoo = AtlasSprite::GetBatchFoo(m_Textures->at(3), 1024 + (PLAYER_AFTERBURNER_COUNT + ROCKET_AFTERBURNER_COUNT));

  m_LandscapeFoo = AtlasSprite::GetFoo(m_Textures->at(2), 1, 1, 0, 1, 0.0);
  m_SecondBatchFoo = AtlasSprite::GetBatchFoo(m_Textures->at(2), 1);

  if (m_SimulationTime > 0.0) {
    for (unsigned int i=0; i<m_SpriteCount; i++) {
      if (i == m_PlayerIndex) {
        m_AtlasSprites[i]->ResetFoo(m_PlayerFoo, m_PlayerAfterburnerFoo);
      } else if (i == m_LandscapeIndex) {
        m_AtlasSprites[i]->ResetFoo(m_LandscapeFoo, NULL);
      } else if (i >= m_SpaceShipPartsStartIndex && i < m_SpaceShipPartsStopIndex) {
        int sprite_index = i - (m_SpaceShipPartsStartIndex);
        if (sprite_index == 0) {
          m_AtlasSprites[i]->ResetFoo(m_SpaceShipPartBaseFoo, m_SpaceShipPartAfterburnerFoo);
        } else if (sprite_index == 1) {
          m_AtlasSprites[i]->ResetFoo(m_SpaceShipPartTopFoo, NULL);
        } else {
          m_AtlasSprites[i]->ResetFoo(m_SpaceShipPartMiddleFoo, NULL);
        }
      } else if (i >= m_PlatformsStartIndex && i < m_PlatformsStopIndex) {
        m_AtlasSprites[i]->ResetFoo(m_PlatformFoo, NULL);
      } else if (i >= m_EnemiesStartIndex && i < m_EnemiesStopIndex) {
        m_AtlasSprites[i]->ResetFoo(m_EnemyFoo, NULL);
      } else if (i >= m_DropZonesStartIndex && i < m_DropZonesStopIndex) {
      } else {
        LOGV("wtf %d %d %d %d %d %d %d %d %d %d\n", i, m_PlayerIndex, m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex, m_PlatformsStartIndex, m_PlatformsStopIndex, m_EnemiesStartIndex, m_EnemiesStopIndex, m_DropZonesStartIndex, m_DropZonesStopIndex);
      }
    }
  }
  m_ThirdBatchFoo = Model::GetBatchFoo(m_Textures->at(5), m_FooFoos[0]->m_numFaces, 100);
}


void SpaceShipDown::DestroyFoos() {
  delete m_PlayerFoo;
  delete m_PlayerAfterburnerFoo;
  delete m_SpaceShipPartBaseFoo;
  delete m_SpaceShipPartTopFoo;
  delete m_SpaceShipPartMiddleFoo;
  delete m_SpaceShipPartAfterburnerFoo;
  delete m_DropZoneFoo;
  delete m_PlatformFoo;
  delete m_LandscapeFoo;
  delete m_EnemyFoo;
  delete m_BatchFoo;
  delete m_SecondBatchFoo;
  delete m_ThirdBatchFoo;
}


void SpaceShipDown::StartLevel(int level_index) {
  m_LevelIndex = level_index;
  if (m_LevelIndex > m_LevelFoos->size() - 1) {
    m_LevelIndex = 0;
  }
  m_SimulationTime = 0.0;
  m_SpaceShipPartsStartIndex = -1;
  m_SpaceShipPartsStopIndex = -1;
  m_PlatformsStartIndex = -1;
  m_PlatformsStopIndex = -1;
  m_DropZonesStartIndex = -1;
  m_DropZonesStopIndex = -1;
  m_PickedUpPartIndex = -1;
  m_RequiredPartIndex = 0;
  m_EnemiesStartIndex = -1;
  m_EnemiesStopIndex = -1;
  m_PickupTimeout = -1;
  m_ThrustLevel = 0.0;
  m_WorldWidth = 0.0;
  m_WorldHeight = 0.0;
  m_DebugDrawToggle = true;
  m_TouchedLeft = false;
  m_TouchedRight = false;
  m_StackCount = 0;
  m_TakeoffTimeout = -1;
  m_CameraOffsetX = 0;
  m_CameraOffsetY = 0;
  CreateWorld();
  CreatePickupJoints();
  CreateDebugDraw();
  LoadLevel(m_LevelIndex, 2);
  LoadLevel(m_LevelIndex, 1);
  LoadLevel(m_LevelIndex, 0);
  LoadLevel(m_LevelIndex, 3);
  CreateBorder(m_WorldWidth, m_WorldHeight);
  //CreateLandscape();
  //CreateVehicles();
  m_CurrentSound = m_LevelIndex;
}


void SpaceShipDown::StopLevel() {
  for (std::vector<b2JointDef*>::iterator i = m_PickupJointDefs.begin(); i != m_PickupJointDefs.end(); ++i) {
    delete *i;
  }
  m_PickupJointDefs.clear();
  for (std::vector<EnemyVehicle*>::iterator i = g_EnemyVehicles.begin(); i != g_EnemyVehicles.end(); ++i) {
    delete *i;
  }
  g_EnemyVehicles.clear();
  for (std::vector<SphereObstacle*>::iterator i = BaseVehicle::allObstacles.begin(); i != BaseVehicle::allObstacles.end(); ++i) {
    delete *i;
  }
  BaseVehicle::allObstacles.clear();
  for (std::vector<b2MouseJoint*>::iterator i = m_EnemyMouseJoints.begin(); i != m_EnemyMouseJoints.end(); ++i) {
    world->DestroyJoint(*i);
  }
  m_EnemyMouseJoints.clear();
  delete m_DebugDraw;
  delete m_ContactListener;
  delete world;
  delete m_FrictionJointDef;
  delete m_PollJointDef;
}


SpaceShipDown::~SpaceShipDown() {
  StopLevel();
  DestroyFoos();
}


void SpaceShipDown::CreateVehicles() {
  for (int i = 0; i<5; i++) {
    EnemyVehicle *enemy = new EnemyVehicle;
    g_EnemyVehicles.push_back(enemy);
    CreateEnemy();
  }
}


void SpaceShipDown::CreateEnemy() {
  float radius = 30.0;

  int enemy_index = m_SpriteCount;
  if (m_EnemiesStartIndex == -1) {
    m_EnemiesStartIndex = enemy_index;
    m_EnemiesStopIndex = m_EnemiesStartIndex;
  }
  m_AtlasSprites.push_back(new SpriteGun(m_EnemyFoo, NULL));
  m_AtlasSprites[enemy_index]->SetScale(BLOCK_WIDTH / 2, BLOCK_WIDTH / 2);
  m_AtlasSprites[enemy_index]->Build(0);
  m_SpriteCount++;
  m_EnemiesStopIndex = m_SpriteCount;

  int enemy_vehicle_index = enemy_index - m_EnemiesStartIndex;

  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0;
  bd.fixedRotation = true;
  bd.gravityScale = 0.0;
  bd.position.Set(0, 0);
  b2Body *enemy_body = world->CreateBody(&bd);
  enemy_body->SetUserData((void *)enemy_index);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = ENEMY_DENSITY;
  fd.restitution = 0.0;
  fd.friction = ENEMY_FRICTION;
  fd.filter.categoryBits = 0x0004;
  fd.filter.maskBits = 0x0002 | 0x0004;
  enemy_body->CreateFixture(&fd);
  enemy_body->SetActive(true);

  b2MouseJointDef mouse_joint_def;
  mouse_joint_def.bodyA = m_GroundBody;
  mouse_joint_def.bodyB = enemy_body;
  mouse_joint_def.target = b2Vec2(0.0, 0.0);
  mouse_joint_def.maxForce = 50.0f * enemy_body->GetMass();
  mouse_joint_def.dampingRatio = 50.0;
  mouse_joint_def.frequencyHz = 100.0;
  m_EnemyMouseJoints.push_back((b2MouseJoint *)world->CreateJoint(&mouse_joint_def));
  enemy_body->SetTransform(b2Vec2(g_EnemyVehicles[enemy_vehicle_index]->position().x / PTM_RATIO, -g_EnemyVehicles[enemy_vehicle_index]->position().z / PTM_RATIO), 0);
}


void SpaceShipDown::CreatePlayer(float x, float y) {
  float radius = 30.0;

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerFoo, m_PlayerAfterburnerFoo));
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 4;
  m_AtlasSprites[m_PlayerIndex]->SetPosition(x, y);
  m_AtlasSprites[m_PlayerIndex]->SetScale(BLOCK_WIDTH / 2, BLOCK_WIDTH / 2);
  m_AtlasSprites[m_PlayerIndex]->Build(PLAYER_AFTERBURNER_COUNT);
  m_SpriteCount++;

  m_Models.push_back(new Model(m_FooFoos.at(1)));
  LOGV("wha1: %d\n", m_PlayerIndex);
  m_Models[m_PlayerIndex]->m_Scale[0] = 2.0;
  m_Models[m_PlayerIndex]->m_Scale[1] = 2.0;
  m_Models[m_PlayerIndex]->m_Scale[2] = 2.0;
  m_Models[m_PlayerIndex]->m_Position[0] = m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO;
  m_Models[m_PlayerIndex]->m_Position[1] = m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO;
  m_Models[m_PlayerIndex]->m_Position[2] = 0.0;
  m_ModelCount++;

  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);
  m_PlayerBody = world->CreateBody(&bd);
  m_PlayerBody->SetUserData((void *)m_PlayerIndex);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = PLAYER_DENSITY;
  fd.restitution = 0.0;
  fd.friction = PLAYER_FRICTION;
  fd.filter.categoryBits = 0x0002;
  m_PlayerBody->CreateFixture(&fd);
  m_PlayerBody->SetActive(true);
  m_FrictionJointDef->bodyB = m_PlayerBody;
}


void SpaceShipDown::CreateSpaceShipPart(float x, float y) {
  int part_index = m_SpriteCount;

  if (m_SpaceShipPartsStartIndex == -1) {
    m_SpaceShipPartsStartIndex = part_index;
    m_SpaceShipPartsStopIndex = m_SpaceShipPartsStartIndex;
  }

  int sprite_index = m_SpaceShipPartsStopIndex - m_SpaceShipPartsStartIndex;
  int build_count = 0;
  float scale = 1.0;
  if (sprite_index == 0) {
    m_AtlasSprites.push_back(new SpriteGun(m_SpaceShipPartBaseFoo, m_SpaceShipPartAfterburnerFoo));
    build_count = ROCKET_AFTERBURNER_COUNT;
    m_Models.push_back(new Model(m_FooFoos.at(4)));
    scale = 1.6;
  } else if (sprite_index == 1) {
    m_AtlasSprites.push_back(new SpriteGun(m_SpaceShipPartTopFoo, NULL));
    m_Models.push_back(new Model(m_FooFoos.at(2)));
    scale = 2.2;
  } else {
    m_AtlasSprites.push_back(new SpriteGun(m_SpaceShipPartMiddleFoo, NULL));
    m_Models.push_back(new Model(m_FooFoos.at(3)));
    scale = 1.1;
  }
  m_AtlasSprites[part_index]->SetPosition(x, y);
  m_AtlasSprites[part_index]->SetScale(BLOCK_WIDTH / 2, BLOCK_WIDTH / 2);
  m_AtlasSprites[part_index]->Build(build_count);
  m_SpriteCount++;
  m_SpaceShipPartsStopIndex = m_SpriteCount;

  LOGV("wha2: %d\n", part_index);
  m_Models[part_index]->m_Scale[0] = scale;
  m_Models[part_index]->m_Scale[1] = scale;
  m_Models[part_index]->m_Scale[2] = scale;
  m_Models[part_index]->m_Position[0] = m_AtlasSprites[part_index]->m_Position[0] / PTM_RATIO;
  m_Models[part_index]->m_Position[1] = m_AtlasSprites[part_index]->m_Position[1] / PTM_RATIO;
  m_Models[part_index]->m_Position[2] = 0.0;
  m_ModelCount++;

  float radius = 23.5;
  MLPoint startPosition = MLPointMake(m_AtlasSprites[part_index]->m_Position[0] / PTM_RATIO, m_AtlasSprites[part_index]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 1.0f;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);

  //part
  b2Body *part_body;
  part_body = world->CreateBody(&bd);
  if (sprite_index == 0) {
    m_SpaceShipBaseBody = part_body;
    m_RequiredPartIndex = part_index;
  }
  part_body->SetUserData((void *)part_index);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = PART_DENSITY;
  fd.restitution = 0.0;
  fd.friction = PART_FRICTION;
  fd.isSensor = false;
  //fd.filter.maskBits = 0x0004;
  //fd.filter.categoryBits = 0x0002;
  part_body->CreateFixture(&fd);

  //part pickup sensor
  shape.m_radius = (radius * 1.25) / PTM_RATIO;
  fd.shape = &shape;
  fd.density = 0.0;
  fd.restitution = 0.0;
  fd.friction = 0.0;
  fd.isSensor = true;
  part_body->CreateFixture(&fd);
  part_body->SetActive(true);
}


void SpaceShipDown::CreateDropZone(float x, float y, float w, float h) {
  int drop_zone_index = m_SpriteCount;

  if (m_DropZonesStartIndex == -1) {
    m_DropZonesStartIndex = drop_zone_index;
  }

  // Define the ground body.
  // bottom-left corner
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(x / PTM_RATIO, ((y - h) / PTM_RATIO));
  
  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  groundBody->SetUserData((void *)-1);
  

  // Define the ground box shape.
  b2PolygonShape groundBox;
  groundBox.SetAsBox(w / PTM_RATIO, h / PTM_RATIO);

  b2FixtureDef fd;
  fd.shape = &groundBox;
  fd.isSensor = true;
  
  groundBody->CreateFixture(&fd);

/*
  m_AtlasSprites.push_back(new SpriteGun(m_DropZoneFoo, NULL));
  m_AtlasSprites[drop_zone_index]->SetScale(BLOCK_WIDTH / 2, BLOCK_WIDTH / 2);
  m_AtlasSprites[drop_zone_index]->SetPosition(x, y - BLOCK_WIDTH * 0.5);
  m_AtlasSprites[drop_zone_index]->m_IsAlive = true;
  m_AtlasSprites[drop_zone_index]->m_Frame = (drop_zone_index - (m_DropZonesStartIndex)) % 16;
  m_AtlasSprites[drop_zone_index]->m_Fps = 10;
  m_AtlasSprites[drop_zone_index]->Build(0);
  m_SpriteCount++;
*/
  m_DropZonesStopIndex = m_SpriteCount;
}


void SpaceShipDown::CreatePlatform(float x, float y, float w, float h) {
  int platform_index = m_SpriteCount;

  if (m_PlatformsStartIndex == -1) {
    m_PlatformsStartIndex = platform_index;
  }

  // Define the ground body.
  // bottom-left corner
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(x / PTM_RATIO, ((y - h) / PTM_RATIO));
  
  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  groundBody->SetActive(true);
  groundBody->SetUserData((void *)-2);
  
  // Define the ground box shape.
  b2PolygonShape groundBox;
  
  // bottom
  groundBox.SetAsBox(w / PTM_RATIO, h / PTM_RATIO);
  groundBody->CreateFixture(&groundBox, 0);

  m_AtlasSprites.push_back(new SpriteGun(m_PlatformFoo, NULL));
  m_AtlasSprites[platform_index]->SetScale(BLOCK_WIDTH / 2, BLOCK_WIDTH / 2);
  m_AtlasSprites[platform_index]->SetPosition(x, y - BLOCK_WIDTH * 0.5);
  m_AtlasSprites[platform_index]->Build(0);
  m_SpriteCount++;
  m_PlatformsStopIndex = m_SpriteCount;

  m_Models.push_back(new Model(m_FooFoos.at(0)));
  LOGV("wha3: %d\n", platform_index);
  m_Models[platform_index]->m_Scale[0] = 1.5;
  m_Models[platform_index]->m_Scale[1] = 1.5;
  m_Models[platform_index]->m_Scale[2] = 1.5;
  m_Models[platform_index]->m_Position[0] = m_AtlasSprites[platform_index]->m_Position[0] / PTM_RATIO;
  m_Models[platform_index]->m_Position[1] = m_AtlasSprites[platform_index]->m_Position[1] / PTM_RATIO;
  m_Models[platform_index]->m_Position[2] = 0.0;
  m_ModelCount++;
  
}


void SpaceShipDown::CreateWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0, GRAVITY);
  world = new b2World(gravity, false);
  m_ContactListener = new SpaceShipDownContactListener();
  world->SetContactListener(m_ContactListener);
}


void SpaceShipDown::CreateBorder(float width, float height) {
  b2BodyDef borderBodyDef;
  b2PolygonShape borderBox;
  b2FixtureDef fd;
  fd.shape = &borderBox;
  fd.filter.maskBits = 0x0002;
  b2Body* borderBody;

  float x = 0.0;
  float y = 0.0;
  float w = width;
  //float h = height;
  float t = 10.0;
  float hs = 0.0;
  float vs = 0.0;

  x = 0;
  y = -height;
  hs = w;
  vs = t;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  m_GroundBody = borderBody;
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&fd);
  borderBody->SetUserData((void *)-2);

  x = -width;
  y = 0;
  hs = t;
  vs = w;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&fd);
  borderBody->SetUserData((void *)-2);

  x = 0;
  y = height;
  hs = w;
  vs = t;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&fd);
  borderBody->SetUserData((void *)-2);

  x = width;
  y = 0;
  hs = t;
  vs = w;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&fd);
  borderBody->SetUserData((void *)-2);
}


void SpaceShipDown::CreatePickupJoints() {
  b2RopeJointDef *rope_joint_def = new b2RopeJointDef();
  rope_joint_def->localAnchorA = b2Vec2(0.0, 0.0);
  rope_joint_def->localAnchorB = b2Vec2(0.0, 0.0);
  rope_joint_def->maxLength = 75.0 / PTM_RATIO;
  m_PickupJointDefs.push_back(rope_joint_def);

  b2DistanceJointDef *distance_joint_def = new b2DistanceJointDef();
  distance_joint_def->length = 500.0 / PTM_RATIO;
  distance_joint_def->dampingRatio = 0.0;
  m_PickupJointDefs.push_back(distance_joint_def);

  m_FrictionJointDef = new b2FrictionJointDef();
  m_FrictionJointDef->maxForce = 30.0;
  m_FrictionJointDef->maxTorque = 0.0;

  m_PollJointDef = new b2PrismaticJointDef();
}


void SpaceShipDown::CreateLandscape() {
  float m_WorldWidthInPixels = m_WorldWidth * 2.0;
  float m_WorldHeightInPixels = m_WorldHeight * 2.0;

  m_LandscapeIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_LandscapeFoo, NULL));
  m_AtlasSprites[m_LandscapeIndex]->SetScale(1024, 1024);
  m_AtlasSprites[m_LandscapeIndex]->SetPosition(0.0, 0.0);
  m_AtlasSprites[m_LandscapeIndex]->Build(0);
  m_SpriteCount++;
}


void SpaceShipDown::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	//float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
  //LOGV("state: %d %f %f\n", hitState, x, y);
  if (hitState == 0 && x < 150.0 && y < 150.0) {
    m_DebugDrawToggle = !m_DebugDrawToggle;
    LOGV("m_DebugDrawToggle is now: %d\n", m_DebugDrawToggle);
  } else {
    if (hitState != 2) {
      if (xx > 0) {
        m_TouchedRight = true;
      } else {
        m_TouchedLeft = true;
      }
    } else {
      if (xx > 0) {
        m_TouchedRight = false;
      } else {
        m_TouchedLeft = false;
      }
    }
  }
}


void SpaceShipDown::AdjustZoom() {
  m_Zoom = ((m_WorldWidth * 2.0) / (float)m_ScreenWidth) * 0.5;
  //m_Zoom = ((m_WorldWidth * 2.0) / (float)m_ScreenWidth) * 2.75;
}


int SpaceShipDown::Simulate() {

  AdjustZoom();

  m_PickupTimeout += m_DeltaTime;

  // update each enemy
  for (unsigned int i = 0; i < g_EnemyVehicles.size(); i++) {
    g_EnemyVehicles[i]->update(m_SimulationTime, m_DeltaTime);
    Vec3 steer (0, 0, 0);
    Vec3 avoidance = g_EnemyVehicles[i]->steerToAvoidObstacles(g_AvoidancePredictTimeMin, (ObstacleGroup&) BaseVehicle::allObstacles);
    steer = avoidance + g_EnemyVehicles[i]->steerForSeek(Vec3(m_AtlasSprites[m_PlayerIndex]->m_Position[0], 0, -m_AtlasSprites[m_PlayerIndex]->m_Position[1]));
    g_EnemyVehicles[i]->applySteeringForce(steer, m_DeltaTime);
  }


  int velocityIterations = 32;
  int positionIterations = 32;

  world->Step(m_DeltaTime, velocityIterations, positionIterations);

  m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);
  for (unsigned int i=m_DropZonesStartIndex; i<m_DropZonesStopIndex; i++) {
    m_AtlasSprites[i]->Simulate(m_DeltaTime);
  }

  b2Vec2 forcePosition = m_PlayerBody->GetWorldCenter();
  b2Vec2 player_velocity = m_PlayerBody->GetLinearVelocity();

  float thrust_x = 400.0;
  float thrust_y = 1200.0;

  if (m_TouchedLeft) {
    //thrust_x += -PLAYER_HORIZONTAL_THRUST;
    thrust_x = -thrust_x;
  }

  if (m_TouchedLeft && m_TouchedRight) {
    //thrust_x += PLAYER_HORIZONTAL_THRUST;
    thrust_x = 0.0;
    //thrust_y *= 1.1;
  }

  if (m_TouchedLeft || m_TouchedRight) {
    thrust_y *= 2.0;
    if (m_PickedUpPartIndex != -1) {
      //thrust_x *= 2.0;
      //thrust_y = 1.0;
      thrust_y *= 2.0;
    }
    if (player_velocity.y < PLAYER_MAX_VELOCITY_Y) {
      //m_ThrustLevel += 40000.0 * thrust_y * m_DeltaTime;
    }
    m_PlayerBody->ApplyForce(b2Vec2(thrust_x, thrust_y), forcePosition);
    //m_AtlasSprites[m_PlayerIndex]->Fire();
    //m_AtlasSprites[m_PlayerIndex]->m_RenderBullets = true;
  } else {
    m_ThrustLevel = 0.0;
    if (m_PickedUpPartIndex != -1) {
      thrust_y *= 2.0;
    }
    m_PlayerBody->ApplyForce(b2Vec2(0.0, thrust_y), forcePosition);
    //m_AtlasSprites[m_PlayerIndex]->m_RenderBullets = false;
  }

  m_AtlasSprites[m_PlayerIndex]->Fire();

  for (b2Body* b = world->GetBodyList(); b; b = b->GetNext()) {
    intptr_t body_index = (intptr_t) b->GetUserData();
    body_index = fastAbs(body_index);
    float x = b->GetPosition().x * PTM_RATIO;
    float y = b->GetPosition().y * PTM_RATIO;
    if (body_index == m_PlayerIndex) {
      if (player_velocity.x > PLAYER_MAX_VELOCITY_X) {
        player_velocity.x = PLAYER_MAX_VELOCITY_X;
      }
      if (player_velocity.x < -PLAYER_MAX_VELOCITY_X) {
        player_velocity.x = -PLAYER_MAX_VELOCITY_X;
      }
      if (player_velocity.y > PLAYER_MAX_VELOCITY_Y) {
        player_velocity.y = PLAYER_MAX_VELOCITY_Y;
      }
      if (player_velocity.y < -PLAYER_MAX_VELOCITY_Y * 2.5) {
        player_velocity.y = -PLAYER_MAX_VELOCITY_Y * 2.5;
      }
      b->SetLinearVelocity(player_velocity);
      //m_AtlasSprites[m_SpaceShipPartsStartIndex]->m_EmitVelocity[0] = fastSinf(m_SimulationTime * 8.0) * 200.0;
      //m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[0] = fastSinf(m_SimulationTime * 4.0) * 1000.0; 
      //m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[1] = -300.0; //player_velocity.y - 800.0 ;
      m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[0] = 0;
      m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[1] = 0;

    } else if (body_index >= m_EnemiesStartIndex && body_index <= m_EnemiesStopIndex) {
      int mouse_joint_index = (m_EnemiesStopIndex - 1) - body_index;
      float rot1a = 0.0;
      Vec3 pos1a = g_EnemyVehicles[mouse_joint_index]->position();
      Vec3 vel1a = g_EnemyVehicles[mouse_joint_index]->velocity();
      if (vel1a.x != 0.0) {
        rot1a = atan2(vel1a.z, vel1a.x);
      }
      m_AtlasSprites[body_index]->m_Rotation = -(rot1a);
      //m_AtlasSprites[m_EnemiesStartIndex + i]->m_Position[0] = pos1a.x;
      //m_AtlasSprites[m_EnemiesStartIndex + i]->m_Position[1] = -pos1a.z;
      //LOGV("enemy: %d %d %d %d\n", mouse_joint_index, body_index, m_EnemiesStartIndex, m_EnemiesStopIndex);
      m_EnemyMouseJoints.at(mouse_joint_index)->SetTarget(b2Vec2(pos1a.x / PTM_RATIO, -pos1a.z / PTM_RATIO));
    }
    //m_AtlasSprites[body_index]->m_Rotation = RadiansToDegrees(b->GetAngle());
    m_AtlasSprites[body_index]->SetPosition(x, y);
    m_Models[body_index]->SetPosition(x / PTM_RATIO, y / PTM_RATIO, 0.0);
  }

  std::vector<MLContact>::iterator pos;
  for (pos = m_ContactListener->m_Contacts.begin(); pos != m_ContactListener->m_Contacts.end(); ++pos) {
    MLContact contact = *pos;
    b2Body *bodyA = contact.fixtureA->GetBody();
    b2Body *bodyB = contact.fixtureB->GetBody();
    intptr_t indexA = (intptr_t)bodyA->GetUserData();
    intptr_t indexB = (intptr_t)bodyB->GetUserData();
    if (indexA == m_PlayerIndex || indexB == m_PlayerIndex) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA < m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB < m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex == -1) {
          if (bodyA == m_PlayerBody) {
            m_PickedUpPartIndex = indexB;
            m_FrictionJointDef->bodyA = bodyB;
          } else {
            m_PickedUpPartIndex = indexA;
            m_FrictionJointDef->bodyA = bodyA;
          }
          if (m_PickedUpPartIndex == m_RequiredPartIndex) {
            m_PickupTimeout = 0.0;
            b2JointDef *pickup_joint_def = m_PickupJointDefs.at(0);
            pickup_joint_def->bodyA = bodyA;
            pickup_joint_def->bodyB = bodyB;
            m_PickupJoint = (b2RopeJoint *)world->CreateJoint(pickup_joint_def);
            m_FrictionJoint = (b2FrictionJoint *)world->CreateJoint(m_FrictionJointDef);
          } else {
            m_PickedUpPartIndex = -1;
          }
        }
      } else if ((indexA >= m_EnemiesStartIndex && indexA < m_EnemiesStopIndex) || (indexB >= m_EnemiesStartIndex && indexB < m_EnemiesStopIndex)) {
        int enemy_index = -1;
        b2Body *enemy_body = NULL;
        if (bodyA == m_PlayerBody) {
          enemy_index = indexB;
          enemy_body = bodyB;
        } else {
          enemy_index = indexA;
          enemy_body = bodyA;
        }
        int enemy_vehicle_index = enemy_index - m_EnemiesStartIndex;
        g_EnemyVehicles[enemy_vehicle_index]->reset();
        enemy_body->SetTransform(b2Vec2(g_EnemyVehicles[enemy_vehicle_index]->position().x / PTM_RATIO, -g_EnemyVehicles[enemy_vehicle_index]->position().z / PTM_RATIO), 0);
      }
    } else if (indexA == -1 || indexB == -1) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA < m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB < m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex != -1 && (indexA == m_PickedUpPartIndex || indexB == m_PickedUpPartIndex)) {
          float x1 = bodyA->GetPosition().x;
          float x2 = bodyB->GetPosition().x;
          if ((fastAbs(x1 - x2) < 1.0) && m_PickupTimeout > 1.0) {
            b2Vec2 player_velocity;
            if (indexA == -1) {
              bodyB->SetUserData((void *)-indexB);
            } else {
              bodyA->SetUserData((void *)-indexA);
            }
            m_PollJointDef->bodyA = bodyA;
            m_PollJointDef->bodyB = bodyB;
            m_PollJointDef->localAxisA.Set(0.0f, 1.0f);
            (b2PrismaticJointDef *)world->CreateJoint(m_PollJointDef);
            if (m_RequiredPartIndex == m_SpaceShipPartsStartIndex) {
              m_RequiredPartIndex += 2;
            } else if (m_RequiredPartIndex == m_SpaceShipPartsStopIndex - 1) {
              m_RequiredPartIndex = m_SpaceShipPartsStartIndex + 1;
            } else {
              m_RequiredPartIndex++;
            }
            m_PickedUpPartIndex = -1;
            world->DestroyJoint(m_PickupJoint);
            world->DestroyJoint(m_FrictionJoint);
            m_StackCount++;
          }
        }
      }
    }
  }

  if (m_StackCount == (m_SpaceShipPartsStopIndex - m_SpaceShipPartsStartIndex)) {
    m_TakeoffTimeout += m_DeltaTime;
    if (m_TakeoffTimeout > 2.0) {
      b2Vec2 forcePosition = m_SpaceShipBaseBody->GetWorldCenter();
      m_SpaceShipBaseBody->ApplyForce(b2Vec2(0.0, 1500.0 * m_StackCount), forcePosition);
      m_AtlasSprites[m_SpaceShipPartsStartIndex]->Simulate(m_DeltaTime);
      m_AtlasSprites[m_SpaceShipPartsStartIndex]->Fire();
      m_AtlasSprites[m_SpaceShipPartsStartIndex]->m_RenderBullets = true;
      m_AtlasSprites[m_SpaceShipPartsStartIndex]->m_EmitVelocity[0] = fastSinf(m_SimulationTime * 8.0) * 200.0;
      m_AtlasSprites[m_SpaceShipPartsStartIndex]->m_EmitVelocity[1] = -100.0 * m_StackCount;
    }
  }

  float tx = -m_AtlasSprites[m_PlayerIndex]->m_Position[0];
  float ty = -m_AtlasSprites[m_PlayerIndex]->m_Position[1];

  if (tx > m_WorldWidth * 0.5) {
    tx = m_WorldWidth * 0.5;
  }
  if (tx < -m_WorldWidth * 0.5) {
    tx = -m_WorldWidth * 0.5;
  }
  if (ty > m_WorldWidth * 0.5) {
    ty = m_WorldWidth * 0.5;
  }
  if (ty < -m_WorldWidth * 0.5) {
    ty = -m_WorldWidth * 0.5;
  }

  m_CameraOffsetX += -(0.75 * m_DeltaTime * (-tx + m_CameraOffsetX));
  m_CameraOffsetY += -(0.75 * m_DeltaTime * (-ty + m_CameraOffsetY));

  float space_ship_height = m_AtlasSprites[m_SpaceShipPartsStartIndex + 1]->m_Position[1];
  if (space_ship_height > m_WorldHeight * 1.5) {
    StopLevel();
    StartLevel(m_LevelIndex + 1);
  }

/*
  RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1, m_SecondBatchFoo);
  RenderSpriteRange(m_PlatformsStartIndex, m_PlatformsStopIndex, m_BatchFoo);
  RenderSpriteRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex, m_BatchFoo);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_BatchFoo);
  RenderSpriteRange(m_EnemiesStartIndex, m_EnemiesStopIndex, m_BatchFoo);
*/

/*
  m_BatchFoo->m_NumBatched = 0;
  m_SecondBatchFoo->m_NumBatched = 0;

  RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1, m_SecondBatchFoo);
  RenderSpriteRange(m_DropZonesStartIndex, m_DropZonesStopIndex, m_BatchFoo);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_BatchFoo);
  RenderSpriteRange(m_PlatformsStartIndex, m_PlatformsStopIndex, m_BatchFoo);
  RenderSpriteRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex, m_BatchFoo);
  RenderSpriteRange(m_EnemiesStartIndex, m_EnemiesStopIndex, m_BatchFoo);
*/

  m_Models[m_PlayerIndex]->m_Position[0] = m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO;
  m_Models[m_PlayerIndex]->m_Position[1] = m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO;
  m_Models[m_PlayerIndex]->m_Position[2] = 0.0;

  //m_Models[0]->m_Position[1] = m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO;

  m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0]; // - (m_CameraOffsetX / PTM_RATIO);
  m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1]; // - 30.0;// (m_CameraOffsetY / PTM_RATIO);
  m_CameraTarget[2] = 0.0;

  //LOGV("%f %f %f\n", m_CameraOffsetX, m_Zoom, m_CameraOffsetX / m_Zoom);
  //LOGV("%f %f\n", m_AtlasSprites[m_PlayerIndex]->m_Position[0], m_AtlasSprites[m_PlayerIndex]->m_Position[1]);

  m_CameraPosition[0] = 0.0 - ((m_CameraOffsetX / (PTM_RATIO)) * 8.0); //m_CameraPosition[1]+(m_CameraOffsetX);
  m_CameraPosition[1] = 30.0 - (m_CameraOffsetY / (PTM_RATIO)); //m_Models[m_PlayerIndex]->m_Position[1] + 60.0; //m_CameraPosition[1]+(m_CameraOffsetY);
  m_CameraPosition[2] = 125.0; //m_CameraOffsetY / m_Zoom;

  //LOGV("%f -- %f %f %f %f\n", (m_CameraTarget[0] - m_CameraPosition[0]), m_CameraTarget[0], m_CameraTarget[1], m_CameraPosition[0], m_CameraPosition[1]);
  //LOGV("%f %f\n", m_Models[0]->m_Position[0], m_Models[1]->m_Position[0]);

  m_ThirdBatchFoo->m_NumBatched = 0;
  RenderModelRange(m_PlatformsStartIndex, m_PlatformsStopIndex, m_ThirdBatchFoo);
  RenderModelRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex, m_ThirdBatchFoo);
  RenderModelRange(m_PlayerIndex, m_PlayerIndex+1, m_ThirdBatchFoo);

  return 1;
}


const char *SpaceShipDown::byte_to_binary(int x) {
	static char b[5];
	b[0] = '\0';
	int z;
	for (z = 16; z > 0; z >>= 1) {
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}


void SpaceShipDown::LoadLevel(int level_index, int cursor_index) {
  float limits[4];
  limits[0] = 0.0;
  limits[1] = 0.0;
  limits[2] = 0.0;
  limits[3] = 0.0;

	int current[4];
	current[0] = current[1] = current[2] = current[3] = 0;
	char *level = (char *)malloc(sizeof(char) * m_LevelFoos->at(level_index)->len);

	fseek(m_LevelFoos->at(level_index)->fp, m_LevelFoos->at(level_index)->off, SEEK_SET);
	fread(level, sizeof(char), m_LevelFoos->at(level_index)->len, m_LevelFoos->at(level_index)->fp);

	unsigned int i = 0;
	unsigned int l = m_LevelFoos->at(level_index)->len;

	//char *pos = NULL;
	const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int idx = -1;
	int *data = (int *)malloc(sizeof(int) * l);
	const char *code;
	for (unsigned int j=0; j<l; j++) {
		const char *pos = index(dictionary, level[j]);
		if (pos != NULL) {
			idx = pos - dictionary;
			data[j] = idx;
		} else {
			throw 666;
		}
	}

	while ( i < l ) {
		code = byte_to_binary(data[i++]);
		if ( code[1] == '1' ) current[0] += data[i++] - 32;
		if ( code[2] == '1' ) current[1] += data[i++] - 32;
		if ( code[3] == '1' ) current[2] += data[i++] - 32;
		if ( code[4] == '1' ) current[3] += data[i++] - 32;
		if ( code[0] == '1' ) {
			
			int ii = 0;
			
			if (current[3] == 8) {
				ii = 1;
			} else if (current[3] == 1) {
				ii = 3;
			} else if (current[3] == 2) {
				ii = 4;
			} else if (current[3] == 0) {
				ii = 2;
			} else {
			}
			
      float world_x = current[0] * 50.0;
      float world_y = current[2] * -50.0;

      if (world_x < limits[0]) {
        limits[0] = world_x;
      }

      if (world_x > limits[2]) {
        limits[2] = world_x;
      }

      if (world_y < limits[1]) {
        limits[1] = world_y;
      }

      if (world_y > limits[3]) {
        limits[3] = world_y;
      }

      float r;
      Vec3 c;
      if (cursor_index == current[3]) {
        switch (current[3]) {
          case 9:
            //black
            break;

          case 8:
            //white
            break;

          case 1:
            //yellow
            CreateSpaceShipPart(world_x, world_y);
            break;
            
          case 2:
            //green
            CreatePlayer(world_x, world_y);
            break;

          case 3:
            //green
            CreateDropZone(world_x, world_y, 5.0, 25.0);
            break;

          case 0:
            //red
            CreatePlatform(world_x, world_y, 25.0, 25.0);
            r = 32.0;
            c = Vec3(world_x, 0, -world_y + 25.0);
            BaseVehicle::allObstacles.push_back(new SphereObstacle (r, c));
            break;
            
          default:
            break;
        }
      }
		}
	}

  //LOGV("%f %f\n", limits[2] - limits[0], limits[3] - limits[1]);

  float max_width = (limits[2] - limits[0]);
  float max_height = (limits[3] - limits[1]);

  if (max_width > max_height) {
    m_WorldWidth = m_WorldHeight = max_width;
  } else {
    m_WorldWidth = m_WorldHeight = max_height;
  }

  m_WorldWidth += 100.0;
  m_WorldHeight += 100.0;
	
	free(level);
	free(data);
}


void SpaceShipDown::CreateDebugDraw() {
  m_DebugDraw = new GLESDebugDraw(PTM_RATIO);
  world->SetDebugDraw(m_DebugDraw);
  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  flags += b2Draw::e_jointBit;
  flags += b2Draw::e_aabbBit;
  flags += b2Draw::e_pairBit;
  flags += b2Draw::e_centerOfMassBit;
  m_DebugDraw->SetFlags(flags);
}


void SpaceShipDown::RenderModelPhase() {
  Model::RenderFoo(m_StateFoo, m_ThirdBatchFoo);
}


void SpaceShipDown::RenderSpritePhase() {
  return;
  glTranslatef(m_CameraOffsetX, m_CameraOffsetY, 0);
  if (m_DebugDrawToggle) {
    AtlasSprite::ReleaseBuffers();
    Model::ReleaseBuffers();
    ResetStateFoo();
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    world->DrawDebugData();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    Color bodyColor;
    bodyColor.set (1.0f, 1.0f, 1.0f);
    //drawXZDisk (g_PlayerVehicle->radius(), g_PlayerVehicle->position(), bodyColor, 40);
	  for (unsigned int i=0; i<g_EnemyVehicles.size(); i++) {
      drawXZDisk (g_EnemyVehicles[i]->radius(), g_EnemyVehicles[i]->position(), bodyColor, 40);
    }
	  for (unsigned int i=0; i<BaseVehicle::allObstacles.size(); i++) {
      drawXZDisk (BaseVehicle::allObstacles[i]->radius, BaseVehicle::allObstacles[i]->center, bodyColor, 40);
    }

    const Vec3 up (0, 0.01f, 0);
    const Color atColor (0.3f, 0.3f, 0.5f);
    const Color noColor = gGray50;
    const bool reached = false;
    const Color baseColor = (reached ? atColor : noColor);

    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
  } else {
    glEnable(GL_BLEND);

    //RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1, m_SecondBatchFoo);
    //RenderSpriteRange(m_DropZonesStartIndex, m_DropZonesStopIndex, m_BatchFoo);
    RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1, m_BatchFoo);
    RenderSpriteRange(m_PlatformsStartIndex, m_PlatformsStopIndex, m_BatchFoo);
    RenderSpriteRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex, m_BatchFoo);
    RenderSpriteRange(m_EnemiesStartIndex, m_EnemiesStopIndex, m_BatchFoo);

    AtlasSprite::RenderFoo(m_StateFoo, m_SecondBatchFoo);
    AtlasSprite::RenderFoo(m_StateFoo, m_BatchFoo);

    glDisable(GL_BLEND);
  }
}


BaseVehicle::BaseVehicle() {
	reset();
}


EnemyVehicle::EnemyVehicle() {
  reset();
}


void EnemyVehicle::update(const float currentTime, const float elapsedTime)
{
  // determine upper bound for pursuit prediction time
  //const float seekerToGoalDist = Vec3::distance (gHomeBaseCenter, gSeeker->position());
  //const float adjustedDistance = seekerToGoalDist - radius() - gHomeBaseRadius;
  //const float seekerToGoalTime = ((adjustedDistance < 0 ) ? 0 : (adjustedDistance/gSeeker->speed()));
  //const float maxPredictionTime = 10.0; //seekerToGoalTime * 0.9f;
 
  // determine steering (pursuit, obstacle avoidance, or braking)
  Vec3 steer (0, 0, 0);
  //if (g_PlayerVehicle->state == running) {
    //Vec3 avoidance = steerToAvoidObstacles(g_AvoidancePredictTimeMin, (ObstacleGroup&) allObstacles);
    //steer = avoidance + steerForPursuit(*g_PlayerVehicle, maxPredictionTime);
  //} else {
  //  applyBrakingForce (g_BrakingRate, elapsedTime);
  //}
  applySteeringForce (steer, elapsedTime);
}


void BaseVehicle::reset (void) {
	SimpleVehicle::reset();  // reset the vehicle 
	setSpeed(0);             // speed along Forward direction.
	setMaxForce(0.0);        // steering force is clipped to this magnitude
	setMaxSpeed(0.0);        // velocity is clipped to this magnitude
	avoiding = false;         // not actively avoiding
}


void EnemyVehicle::reset (void) {
	BaseVehicle::reset();
	randomizeStartingPositionAndHeading();
	setRadius(20.0);
	setSpeed(200.0);
	setMaxSpeed(100.0);
	setMaxForce(10000.0);
}


void BaseVehicle::randomizeStartingPositionAndHeading (void) {
	// randomize position on a ring between inner and outer radii
	// centered around the home base
	const float rRadius = frandom2 (g_MinStartRadius, g_MaxStartRadius);
	const Vec3 randomOnRing = RandomUnitVectorOnXZPlane () * rRadius;
	setPosition (g_HomeBaseCenter + randomOnRing);
	
	// are we are too close to an obstacle?
	if (minDistanceToObstacle (position()) < radius() * 10) {
		// if so, retry the randomization (this recursive call may not return
		// if there is too little free space)
		randomizeStartingPositionAndHeading ();
	} else {
		// otherwise, if the position is OK, randomize 2D heading
		randomizeHeadingOnXZPlane ();
	}
}


Vec3 EnemyVehicle::steerToEvadeAllOtherEnemies (void) {
	// sum up weighted evasion
	Vec3 evade (0, 0, 0);
	for (int i = 0; i < g_EnemyVehicles.size(); i++) {
		const EnemyVehicle& e = *g_EnemyVehicles[i];
		if (position() != e.position()) {
			
			const Vec3 eOffset = e.position() - position();
			const float eDistance = eOffset.length();
			
			// xxx maybe this should take into account e's heading? xxx
			const float timeEstimate = 2.0 * eDistance / e.speed(); //xxx
			const Vec3 eFuture = e.predictFuturePosition (timeEstimate);
			
			// steering to flee from eFuture (enemy's future position)
			const Vec3 flee = xxxsteerForFlee (eFuture);
			
			const float eForwardDistance = forward().dot (eOffset);
			const float behindThreshold = radius() * -50.0;
			
			const float distanceWeight = 0.1; //1 / eDistance;
			const float forwardWeight = 1.0; //((eForwardDistance > behindThreshold) ? 1.0f : 0.5f);
			
			const Vec3 adjustedFlee = flee * distanceWeight * forwardWeight;
			
			evade += adjustedFlee;
		}
	}
	return evade;
}


float BaseVehicle::minDistanceToObstacle (const Vec3 point) {
	//float r = 0;
	Vec3 c = point;
	float minClearance = FLT_MAX;
	for (SOI so = allObstacles.begin(); so != allObstacles.end(); so++)
	{
		//testOneObstacleOverlap ((**so).radius, (**so).center);
	}
	return minClearance;
}
