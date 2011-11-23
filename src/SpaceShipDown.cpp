// Jon Bardin GPL



#include "MemoryLeak.h"
#include "OpenSteer/Draw.h"
#include "SpaceShipDown.h"

#define GRAVITY -100.0
#define PART_DENSITY 5.75
#define PART_FRICTION 0.5
#define PLAYER_DENSITY 6.5
#define PLAYER_FRICTION 0.25
#define PLAYER_MAX_VELOCITY_X 5.0
#define PLAYER_MAX_VELOCITY_Y 5.0
#define BLOCK_WIDTH 54.0

using namespace OpenSteer;
//point to seeker
PlayerVehicle *g_PlayerVehicle;
std::vector<EnemyVehicle*> g_EnemyVehicles;
std::vector<BaseVehicle*> g_AllVehicles;
SOG BaseVehicle::allObstacles;
int BaseVehicle::obstacleCount = 0;
const Vec3 g_HomeBaseCenter(0, 0, 0);
const Vec3 g_PlayerVehicleCenter(100, 0, 100);
const float g_HomeBaseRadius = 1.0;
const float g_ObstacleRadius = 5.0;
const float g_MinStartRadius = 1000;
const float g_MaxStartRadius = 2000;
const float g_BrakingRate = 0.5;
const float g_AvoidancePredictTimeMin  = 0.1f;
const float g_AvoidancePredictTimeMax  = 0.5;
float g_AvoidancePredictTime = g_AvoidancePredictTimeMin;

SpaceShipDown::SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  m_PlayerFoo = AtlasSprite::GetFoo(m_Textures->at(1), 4, 4, 1, 2, 1.0, BLOCK_WIDTH * 1.2, BLOCK_WIDTH * 1.2);
  m_PlayerAfterburnerFoo = AtlasSprite::GetFoo(m_Textures->at(1), 4, 4, 12, 17, 1.0, BLOCK_WIDTH * 1.2, BLOCK_WIDTH * 1.2);
  m_SpaceShipPartFoo = AtlasSprite::GetFoo(m_Textures->at(1), 4, 4, 0, 1, 0.0, BLOCK_WIDTH * 1.2, BLOCK_WIDTH * 1.2);
  m_SpaceShipPartAfterburnerFoo = AtlasSprite::GetFoo(m_Textures->at(1), 4, 4, 0, 1, 1.0, BLOCK_WIDTH * 1.2, BLOCK_WIDTH * 1.2);
  m_DropZoneFoo = AtlasSprite::GetFoo(m_Textures->at(1), 4, 4, 0, 1, 0.0, BLOCK_WIDTH, BLOCK_WIDTH);
  m_PlatformFoo = AtlasSprite::GetFoo(m_Textures->at(1), 4, 4, 0, 1, 0.0, BLOCK_WIDTH, BLOCK_WIDTH);
  m_LandscapeFoo = AtlasSprite::GetFoo(m_Textures->at(2), 1, 1, 0, 1, 0.0, 1024, 1024);
  StartLevel(1);
}


void SpaceShipDown::StartLevel(int level_index) {
  m_SimulationTime = 0.0;
  m_SpaceShipPartsStartIndex = -1;
  m_SpaceShipPartsStopIndex = -1;
  m_PlatformsStartIndex = -1;
  m_PlatformsStopIndex = -1;
  m_DropZonesStartIndex = -1;
  m_DropZonesStopIndex = -1;
  m_PickedUpPartIndex = -1;
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
  LoadSound(0);
  CreateWorld();
  CreatePickupJoints();
  CreateDebugDraw();
  LoadLevel(level_index, 2);
  LoadLevel(level_index, 1);
  LoadLevel(level_index, 0);
  LoadLevel(level_index, 3);
  CreateBorder(m_WorldWidth, m_WorldHeight);
  CreateLandscape();
  CreateVehicles();
}


void SpaceShipDown::StopLevel() {
  delete m_DebugDraw;
  delete m_ContactListener;
  delete world;
  delete m_FrictionJointDef;
  for (std::vector<b2JointDef*>::iterator i = m_PickupJointDefs.begin(); i != m_PickupJointDefs.end(); ++i) {
    delete *i;
  }
  m_PickupJointDefs.clear();
  for (std::vector<BaseVehicle*>::iterator i = g_AllVehicles.begin(); i != g_AllVehicles.end(); ++i) {
    delete *i;
  }
  g_EnemyVehicles.clear();
  g_AllVehicles.clear();
}


SpaceShipDown::~SpaceShipDown() {
  StopLevel();
  delete m_PlayerFoo;
  delete m_PlayerAfterburnerFoo;
  delete m_SpaceShipPartFoo;
  delete m_SpaceShipPartAfterburnerFoo;
  delete m_DropZoneFoo;
  delete m_PlatformFoo;
  delete m_LandscapeFoo;
}


void SpaceShipDown::CreateVehicles() {
  g_PlayerVehicle = new PlayerVehicle;
  g_AllVehicles.push_back(g_PlayerVehicle);
  for (int i = 0; i<40; i++) {
    EnemyVehicle *enemy = new EnemyVehicle;
    g_EnemyVehicles.push_back(enemy);
    g_AllVehicles.push_back(g_EnemyVehicles[i]);
  }
}


void SpaceShipDown::CreatePlayer(float x, float y) {
  float radius = 30.0;

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerFoo, m_PlayerAfterburnerFoo));
  m_AtlasSprites[m_PlayerIndex]->SetPosition(x, y);
  m_AtlasSprites[m_PlayerIndex]->Build(1);
  m_SpriteCount++;

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
  }

  int sprite_index = 6 + (part_index % 3);
  if (sprite_index == 8) {
    m_AtlasSprites.push_back(new SpriteGun(m_SpaceShipPartFoo, m_SpaceShipPartAfterburnerFoo));
    m_AtlasSprites[part_index]->Build(1);
  } else {
    m_AtlasSprites.push_back(new SpriteGun(m_SpaceShipPartFoo, NULL));
    m_AtlasSprites[part_index]->Build(0);
  }
  m_AtlasSprites[part_index]->SetPosition(x, y);
  m_SpriteCount++;
  m_SpaceShipPartsStopIndex = m_SpriteCount;

  float radius = 25.0;
  MLPoint startPosition = MLPointMake(m_AtlasSprites[part_index]->m_Position[0] / PTM_RATIO, m_AtlasSprites[part_index]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 1.0f;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);

  //part
  b2Body *part_body;
  part_body = world->CreateBody(&bd);
  if (sprite_index == 8) {
    m_SpaceShipBaseBody = part_body;
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

  m_AtlasSprites.push_back(new SpriteGun(m_DropZoneFoo, NULL));
  m_AtlasSprites[drop_zone_index]->Build(0);
  m_AtlasSprites[drop_zone_index]->SetPosition(x, y - BLOCK_WIDTH * 0.5);
  m_SpriteCount++;
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
  m_AtlasSprites[platform_index]->Build(0);
  m_AtlasSprites[platform_index]->SetPosition(x, y - BLOCK_WIDTH * 0.5);
  m_SpriteCount++;
  m_PlatformsStopIndex = m_SpriteCount;
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
}


void SpaceShipDown::CreateLandscape() {
  float m_WorldWidthInPixels = m_WorldWidth * 2.0;
  float m_WorldHeightInPixels = m_WorldHeight * 2.0;

  m_LandscapeIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_LandscapeFoo, NULL));
  m_AtlasSprites[m_LandscapeIndex]->Build(0);
  m_AtlasSprites[m_LandscapeIndex]->SetPosition(0.0, 0.0);
  m_SpriteCount++;
}


void SpaceShipDown::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	//float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
  //LOGV("state: %d %f %f\n", hitState, x, y);
  if (hitState == 0 && x < 50.0 && y < 50.0) {
    m_DebugDrawToggle = !m_DebugDrawToggle;
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
  m_Zoom = ((m_WorldWidth * 2.0) / (float)m_ScreenWidth) * 0.75;
}


int SpaceShipDown::Simulate() {

  AdjustZoom();

  m_PickupTimeout += m_DeltaTime;

  //g_PlayerVehicle->update(m_SimulationTime, m_DeltaTime);
  
  // update each enemy
  for (unsigned int i = 0; i < g_EnemyVehicles.size(); i++) {
    g_EnemyVehicles[i]->update(m_SimulationTime, m_DeltaTime);
    //pos1a = ctfEnemies[i]->position();
    //vel1a = ctfEnemies[i]->velocity();
    //if (vel1a.x != 0.0) {
    //  rot1a = atan2(vel1a.z, vel1a.x);
    //}
    //myRaptors[i]->SetRotation(-RadiansToDegrees(rot1a), 0.0);
    //myRaptors[i]->SetPosition(pos1a.x, myRaptorHeight, pos1a.z);
  }


  int velocityIterations = 8;
  int positionIterations = 3;

  world->Step(m_DeltaTime, velocityIterations, positionIterations);

  m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);
  g_PlayerVehicle->setPosition(Vec3(m_AtlasSprites[m_PlayerIndex]->m_Position[0], 0, -m_AtlasSprites[m_PlayerIndex]->m_Position[1]));

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
    m_AtlasSprites[m_PlayerIndex]->Fire();
    m_AtlasSprites[m_PlayerIndex]->m_RenderBullets = true;
  } else {
    m_ThrustLevel = 0.0;
    if (m_PickedUpPartIndex != -1) {
      thrust_y *= 2.0;
    }
    m_PlayerBody->ApplyForce(b2Vec2(0.0, thrust_y), forcePosition);
    m_AtlasSprites[m_PlayerIndex]->m_RenderBullets = false;
  }
  

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
      m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[0] = 0.0; //(fastSinf(m_SimulationTime * 8.0) * 200.0);
      m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[1] = -900.0; //player_velocity.y - 800.0 ;
    }
    //m_AtlasSprites[body_index]->m_Rotation = RadiansToDegrees(b->GetAngle());
    m_AtlasSprites[body_index]->SetPosition(x, y);
  }

  std::vector<MLContact>::iterator pos;
  for (pos = m_ContactListener->m_Contacts.begin(); pos != m_ContactListener->m_Contacts.end(); ++pos) {
    MLContact contact = *pos;
    b2Body *bodyA = contact.fixtureA->GetBody();
    b2Body *bodyB = contact.fixtureB->GetBody();
    intptr_t indexA = (intptr_t)bodyA->GetUserData();
    intptr_t indexB = (intptr_t)bodyB->GetUserData();
    if (indexA == m_PlayerIndex || indexB == m_PlayerIndex) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA <= m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB <= m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex == -1) {
          if (bodyA == m_PlayerBody) {
            m_PickedUpPartIndex = indexB;
            m_FrictionJointDef->bodyA = bodyB;
          } else {
            m_PickedUpPartIndex = indexA;
            m_FrictionJointDef->bodyA = bodyA;
          }
          m_PickupTimeout = 0.0;
          b2JointDef *pickup_joint_def = m_PickupJointDefs.at(0);
          pickup_joint_def->bodyA = bodyA;
          pickup_joint_def->bodyB = bodyB;
          m_PickupJoint = (b2RopeJoint *)world->CreateJoint(pickup_joint_def);
          m_FrictionJoint = (b2FrictionJoint *)world->CreateJoint(m_FrictionJointDef);
        }
      }
    } else if (indexA == -1 || indexB == -1) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA <= m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB <= m_SpaceShipPartsStopIndex)) {
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
            b2PrismaticJointDef *joint_def = new b2PrismaticJointDef();
            joint_def->bodyA = bodyA;
            joint_def->bodyB = bodyB;
            joint_def->localAxisA.Set(0.0f, 1.0f);
            (b2PrismaticJointDef *)world->CreateJoint(joint_def);
            m_PickedUpPartIndex = -1;
            world->DestroyJoint(m_PickupJoint);
            world->DestroyJoint(m_FrictionJoint);
            m_StackCount++;
          }
        }
      }
    }
  }

  if (m_StackCount > 2) {
    m_TakeoffTimeout += m_DeltaTime;
    if (m_TakeoffTimeout > 2.0) {
      b2Vec2 forcePosition = m_SpaceShipBaseBody->GetWorldCenter();
      m_SpaceShipBaseBody->ApplyForce(b2Vec2(0.0, 5500.0), forcePosition);
      m_AtlasSprites[m_SpaceShipPartsStartIndex + 1]->Simulate(m_DeltaTime);
      m_AtlasSprites[m_SpaceShipPartsStartIndex + 1]->Fire();
      m_AtlasSprites[m_SpaceShipPartsStartIndex + 1]->m_RenderBullets = true;
      m_AtlasSprites[m_SpaceShipPartsStartIndex + 1]->m_EmitVelocity[0] = fastSinf(m_SimulationTime * 8.0) * 200.0;
      m_AtlasSprites[m_SpaceShipPartsStartIndex + 1]->m_EmitVelocity[1] = -800.0 ;
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

  if (m_SimulationTime > 30.0) {
    StopLevel();
    StartLevel(1);
  }

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

	char *pos = NULL;
	const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int idx = -1;
	int *data = (int *)malloc(sizeof(int) * l);
	const char *code;
	for (unsigned int j=0; j<l; j++) {
		pos = index(dictionary, level[j]);
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
            BaseVehicle::allObstacles.push_back (new SphereObstacle (r, c));
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
}


void SpaceShipDown::RenderSpritePhase() {
  glTranslatef(m_CameraOffsetX, m_CameraOffsetY, 0);
  if (m_DebugDrawToggle) {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    world->DrawDebugData();
    glRotatef(90.0, 1.0, 0.0, 0.0);
    Color bodyColor;
    bodyColor.set (1.0f, 1.0f, 1.0f);
    drawXZDisk (g_PlayerVehicle->radius(), g_PlayerVehicle->position(), bodyColor, 40);
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
    RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1);
    AtlasSprite::ReleaseBuffers();
    RenderSpriteRange(m_PlatformsStartIndex, m_PlatformsStopIndex);
    RenderSpriteRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex);
    RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1);
    glDisable(GL_BLEND);
    AtlasSprite::ReleaseBuffers();
  }
}


BaseVehicle::BaseVehicle() {
	reset();
}


EnemyVehicle::EnemyVehicle() {
	reset();
}


PlayerVehicle::PlayerVehicle() {
	reset();
}


void BaseVehicle::reset (void) {
	SimpleVehicle::reset ();  // reset the vehicle 
	setSpeed(0);             // speed along Forward direction.
	setMaxForce(0.0);        // steering force is clipped to this magnitude
	setMaxSpeed(0.0);        // velocity is clipped to this magnitude
	avoiding = false;         // not actively avoiding
}


void PlayerVehicle::reset (void) {
	BaseVehicle::reset();
	setPosition(g_HomeBaseCenter);
	setPosition(g_PlayerVehicleCenter);
	setRadius(32.0);
	setSpeed(0);             // speed along Forward direction.
	setMaxSpeed(0.0);        // velocity is clipped to this magnitude
	setMaxForce(0.0);        // steering force is clipped to this magnitude
	g_PlayerVehicle = this;
	state = running;
	evading = false;
}


void EnemyVehicle::reset (void) {
	BaseVehicle::reset();
	randomizeStartingPositionAndHeading();
	setRadius(20.0);
	setSpeed(35.0);
	setMaxSpeed(120.0);
	setMaxForce(5000.0);
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


void EnemyVehicle::update (const float currentTime, const float elapsedTime)
{
	// determine upper bound for pursuit prediction time
	//const float seekerToGoalDist = Vec3::distance (gHomeBaseCenter, gSeeker->position());
	//const float adjustedDistance = seekerToGoalDist - radius() - gHomeBaseRadius;
	//const float seekerToGoalTime = ((adjustedDistance < 0 ) ? 0 : (adjustedDistance/gSeeker->speed()));
	const float maxPredictionTime = 10.0; //seekerToGoalTime * 0.9f;
	
	// determine steering (pursuit, obstacle avoidance, or braking)
	Vec3 steer (0, 0, 0);
	if (g_PlayerVehicle->state == running) {
		//Vec3 avoidance = steerToAvoidObstacles(g_AvoidancePredictTimeMin, (ObstacleGroup&) allObstacles);
		steer = steerForPursuit(*g_PlayerVehicle, maxPredictionTime);
	} else {
		applyBrakingForce (g_BrakingRate, elapsedTime);
	}
	applySteeringForce (steer, elapsedTime);
	
	// detect and record interceptions ("tags") of seeker
	const float seekerToMeDist = Vec3::distance (position(), g_PlayerVehicle->position());
	const float sumOfRadii = radius() + g_PlayerVehicle->radius();
	
	if (seekerToMeDist < sumOfRadii) {
		if (g_PlayerVehicle->state == running) {
			reset();
		}
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


// are there any enemies along the corridor between us and the goal?
bool PlayerVehicle::clearPathToGoal (void) {
	const float sideThreshold = radius() * 8.0f;
	const float behindThreshold = radius() * 2.0f;
	
	const Vec3 goalOffset = g_HomeBaseCenter - position();
	const float goalDistance = goalOffset.length ();
	const Vec3 goalDirection = goalOffset / goalDistance;
	
	const bool goalIsAside = isAside (g_HomeBaseCenter, 0.5);
	
	// for annotation: loop over all and save result, instead of early return 
	bool xxxReturn = true;
	
	// loop over enemies
	for (int i = 0; i < g_EnemyVehicles.size(); i++) {
		// short name for this enemy
		const EnemyVehicle& e = *g_EnemyVehicles[i];
		const float eDistance = Vec3::distance (position(), e.position());
		const float timeEstimate = 0.3f * eDistance / e.speed(); //xxx
		const Vec3 eFuture = e.predictFuturePosition (timeEstimate);
		const Vec3 eOffset = eFuture - position();
		const float alongCorridor = goalDirection.dot (eOffset);
		const bool inCorridor = ((alongCorridor > -behindThreshold) && (alongCorridor < goalDistance));
		const float eForwardDistance = forward().dot (eOffset);
		
		// consider as potential blocker if within the corridor
		if (inCorridor) {
			const Vec3 perp = eOffset - (goalDirection * alongCorridor);
			const float acrossCorridor = perp.length();
			if (acrossCorridor < sideThreshold) {
				// not a blocker if behind us and we are perp to corridor
				const float eFront = eForwardDistance + e.radius ();
				
				const bool eIsBehind = eFront < -behindThreshold;
				const bool eIsWayBehind = eFront < (-2 * behindThreshold);
				const bool safeToTurnTowardsGoal =
				((eIsBehind && goalIsAside) || eIsWayBehind);
				
				if (!safeToTurnTowardsGoal) {
					// this enemy blocks the path to the goal, so return false
					xxxReturn = false;
				}
			}
		}
	}
	
	return xxxReturn;
}


Vec3 PlayerVehicle::steerToEvadeAllDefenders (void) {
	Vec3 evade (0, 0, 0);
	const float goalDistance = Vec3::distance (g_HomeBaseCenter, position());
	
	// sum up weighted evasion
	for (int i = 0; i < g_EnemyVehicles.size(); i++)
	{
		const EnemyVehicle& e = *g_EnemyVehicles[i];
		const Vec3 eOffset = e.position() - position();
		const float eDistance = eOffset.length();
		
		const float eForwardDistance = forward().dot (eOffset);
		const float behindThreshold = radius() * 2;
		const bool behind = eForwardDistance < behindThreshold;
		if ((!behind) || (eDistance < 5))
		{
			if (eDistance < (goalDistance * 1.2)) //xxx
			{
				// const float timeEstimate = 0.5f * eDistance / e.speed;//xxx
				const float timeEstimate = 0.15f * eDistance / e.speed();//xxx
				const Vec3 future =
				e.predictFuturePosition (timeEstimate);
				
				const Vec3 offset = future - position();
				const Vec3 lateral = offset.perpendicularComponent (forward());
				const float d = lateral.length();
				const float weight = -1000 / (d * d);
				evade += (lateral / d) * weight;
			}
		}
	}
	return evade;
}


Vec3 PlayerVehicle::XXXsteerToEvadeAllDefenders (void) {
	// sum up weighted evasion
	Vec3 evade (0, 0, 0);
	for (int i = 0; i < g_EnemyVehicles.size(); i++)
	{
		const EnemyVehicle& e = *g_EnemyVehicles[i];
		const Vec3 eOffset = e.position() - position();
		const float eDistance = eOffset.length();
		
		// xxx maybe this should take into account e's heading? xxx
		const float timeEstimate = 0.5f * eDistance / e.speed(); //xxx
		const Vec3 eFuture = e.predictFuturePosition (timeEstimate);
		
		// steering to flee from eFuture (enemy's future position)
		const Vec3 flee = xxxsteerForFlee (eFuture);
		
		const float eForwardDistance = forward().dot (eOffset);
		const float behindThreshold = radius() * -2;
		
		const float distanceWeight = 4 / eDistance;
		const float forwardWeight = ((eForwardDistance > behindThreshold) ? 1.0f : 0.5f);
		
		const Vec3 adjustedFlee = flee * distanceWeight * forwardWeight;
		
		evade += adjustedFlee;
	}
	return evade;
}


Vec3 PlayerVehicle::steeringForSeeker (void) {
	// determine if obstacle avodiance is needed
	const bool clearPath = clearPathToGoal();
	adjustObstacleAvoidanceLookAhead (clearPath);
	const Vec3 obstacleAvoidance = steerToAvoidObstacles(g_AvoidancePredictTime, (ObstacleGroup&) allObstacles);
	
	// saved for annotation
	avoiding = (obstacleAvoidance != Vec3::zero);
	
	if (avoiding) {
		// use pure obstacle avoidance if needed
		return obstacleAvoidance;
	} else {
		// otherwise seek home base and perhaps evade defenders
		const Vec3 seek = xxxsteerForSeek (g_HomeBaseCenter);
		if (clearPath) {
			Vec3 s = limitMaxDeviationAngle(seek, 0.707f, forward());
			return s;
		} else {
      //alternate evade code
			if (0) {
				// combine seek and (forward facing portion of) evasion
				const Vec3 evade = steerToEvadeAllDefenders ();
				const Vec3 steer = seek + limitMaxDeviationAngle (evade, 0.5f, forward());
				return steer;
			} else {
				const Vec3 evade = XXXsteerToEvadeAllDefenders ();
				const Vec3 steer = limitMaxDeviationAngle (seek + evade, 0.707f, forward());
				return steer;
			}
		}
	}
}


// adjust obstacle avoidance look ahead time: make it large when we are far
// from the goal and heading directly towards it, make it small otherwise.
void PlayerVehicle::adjustObstacleAvoidanceLookAhead (const bool clearPath)
{
	if (clearPath) {
		evading = false;
		const float goalDistance = Vec3::distance (g_HomeBaseCenter,position());
		const bool headingTowardGoal = isAhead (g_HomeBaseCenter, 0.98f);
		const bool isNear = (goalDistance/speed()) < g_AvoidancePredictTimeMax;
		const bool useMax = headingTowardGoal && !isNear;
		g_AvoidancePredictTime = (useMax ? g_AvoidancePredictTimeMax : g_AvoidancePredictTimeMin);
	} else {
		evading = true;
		g_AvoidancePredictTime = g_AvoidancePredictTimeMin;
	}
}


void PlayerVehicle::update (const float currentTime, const float elapsedTime) {
	updateX(currentTime, elapsedTime, Vec3::zero);
}


void PlayerVehicle::updateX (const float currentTime, const float elapsedTime, Vec3 inputSteering) {
	// determine and apply steering/braking forces
	Vec3 steer (0, 0, 0);
	if (state == running) {
		steer = steeringForSeeker();
	} else {
		applyBrakingForce (g_BrakingRate, elapsedTime);
	}
	applySteeringForce (steer, elapsedTime);
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
