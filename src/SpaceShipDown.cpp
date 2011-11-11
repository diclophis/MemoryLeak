// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SpaceShipDown.h"

#define GRAVITY -35.0
#define PART_DENSITY 10.0
#define PART_FRICTION 0.5
#define PLAYER_DENSITY 2.0
#define PLAYER_FRICTION 2.0
#define PLAYER_HORIZONTAL_THRUST 1500.0
#define PLAYER_VERTICAL_THRUST 3000.0
#define PLAYER_MAX_VELOCITY_X 20.0
#define PLAYER_MAX_VELOCITY_Y 15.0

SpaceShipDown::SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(0);
  m_IsPushingAudio = true;
  m_SpaceShipPartsStartIndex = -1;
  m_SpaceShipPartsStopIndex = -1;
  m_PickedUpPartIndex = -1;
  m_PickupTimeout = -1;

  m_PickupJointDef = new b2RopeJointDef();
  m_PickupJointDef->localAnchorA = b2Vec2(0.0, 0.0);
  m_PickupJointDef->localAnchorB = b2Vec2(0.0, 0.0);
  m_PickupJointDef->maxLength = 300.0 / PTM_RATIO;

  m_LandscapeIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 1, 1, 0, 64, 1.0, "", 0, 64, 0.0, 2048 * 2.0, 2048 * 2.0));
  m_AtlasSprites[m_LandscapeIndex]->Build(0);
  m_AtlasSprites[m_LandscapeIndex]->SetPosition(0.0, 0.0);
  m_SpriteCount++;

  CreateWorld();
  CreatePlayer();
  CreateSpaceShipPart(-200.0, -100.0);
  CreateSpaceShipPart(200.0, 100.0);
  CreatePlatform(600.0, 1000.0, 512.0, 25.0);
  CreatePlatform(-600.0, 500.0, 512.0, 25.0);
  CreatePlatform(1200.0, 200.0, 512.0, 25.0);
  CreatePlatform(-1200.0, 0.0, 512.0, 25.0);

  m_DebugDraw = new GLESDebugDraw(PTM_RATIO);
  world->SetDebugDraw(m_DebugDraw);

  m_ContactListener = new SpaceShipDownContactListener();
  world->SetContactListener(m_ContactListener);
  
  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  flags += b2Draw::e_jointBit;
  flags += b2Draw::e_aabbBit;
  flags += b2Draw::e_pairBit;
  flags += b2Draw::e_centerOfMassBit;
  m_DebugDraw->SetFlags(flags);

  m_TouchedLeft = false;
  m_TouchedRight = false;

}


SpaceShipDown::~SpaceShipDown() {
  delete m_DebugDraw;
  delete m_ContactListener;
  delete world;
  delete m_PickupJointDef;
}


void SpaceShipDown::CreatePlayer() {
  float radius = 64.0;

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 0, 64, 1.0, "", 11, 64, 0.25, 256.0, 256.0));
  m_AtlasSprites[m_PlayerIndex]->SetPosition(0.0, 1024.0);
  m_AtlasSprites[m_PlayerIndex]->Build(5);
  m_SpriteCount++;

  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0f;
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
  m_PlayerBody->CreateFixture(&fd);
  m_PlayerBody->SetActive(true);
}


void SpaceShipDown::CreateSpaceShipPart(float x, float y) {
  int part_index = m_SpriteCount;

  if (m_SpaceShipPartsStartIndex == -1) {
    m_SpaceShipPartsStartIndex = part_index;
  }

  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 1, 2, 1.0, "", 0, 64, 0.0, 256.0, 256.0));
  m_AtlasSprites[part_index]->Build(0);
  m_AtlasSprites[part_index]->SetPosition(x, y);
  m_SpriteCount++;
  m_SpaceShipPartsStopIndex = m_SpriteCount;

  float radius = 128.0;
  MLPoint startPosition = MLPointMake(m_AtlasSprites[part_index]->m_Position[0] / PTM_RATIO, m_AtlasSprites[part_index]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0f;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);

  //part
  b2Body *part_body;
  part_body = world->CreateBody(&bd);
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
  shape.m_radius = (radius * 1.1) / PTM_RATIO;
  fd.shape = &shape;
  fd.density = 0.0;
  fd.restitution = 0.0;
  fd.friction = 0.0;
  fd.isSensor = true;
  part_body->CreateFixture(&fd);

  part_body->SetActive(true);

}


void SpaceShipDown::CreatePlatform(float x, float y, float w, float h) {
  // Define the ground body.
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(x / PTM_RATIO, ((y - h) / PTM_RATIO));// bottom-left corner
  
  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  groundBody->SetUserData((void *)-1);
  
  // Define the ground box shape.
  b2PolygonShape groundBox;
  
  // bottom
  groundBox.SetAsBox(w / PTM_RATIO, h / PTM_RATIO);
  groundBody->CreateFixture(&groundBox, 0);
}


void SpaceShipDown::CreateWorld() {
  b2Vec2 gravity;
  b2BodyDef borderBodyDef;
  b2PolygonShape borderBox;
  b2Body* borderBody;

  gravity.Set(0.0, GRAVITY);
  world = new b2World(gravity, false);

  float x = 0.0;
  float y = 0.0;
  float w = 1024.0 * 2.0;
  float h = 1024.0 * 2.0;
  float t = 10.0;
  float hs = 0.0;
  float vs = 0.0;

  x = 0;
  y = -512 * 4;
  hs = w;
  vs = t;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);

  x = -512 * 4;
  y = 0;
  hs = t;
  vs = w;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);

  x = 0;
  y = 512 * 4;
  hs = w;
  vs = t;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);

  x = 512 * 4;
  y = 0;
  hs = t;
  vs = w;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);
}


void SpaceShipDown::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
  LOGV("state: %d %f %f\n", hitState, x, y);
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


int SpaceShipDown::Simulate() {
  m_PickupTimeout += m_DeltaTime;
  m_Zoom = 4096.0 / (float)m_ScreenWidth;

  int velocityIterations = 8;
  int positionIterations = 3;

  world->Step(m_DeltaTime, velocityIterations, positionIterations);

  m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);

  b2Vec2 forcePosition = m_PlayerBody->GetWorldCenter();

  float thrust_x = 0;
  float thrust_y = PLAYER_VERTICAL_THRUST;

  if (m_TouchedLeft) {
    thrust_x += -PLAYER_HORIZONTAL_THRUST;
  }

  if (m_TouchedRight) {
    thrust_x += PLAYER_HORIZONTAL_THRUST;
  }

  if (m_TouchedLeft || m_TouchedRight) {
    if (m_PickedUpPartIndex != -1) {
      thrust_x *= 10;
      thrust_y *= 10;
    }
    m_PlayerBody->ApplyForce(b2Vec2(thrust_x, thrust_y), forcePosition);
    m_AtlasSprites[m_PlayerIndex]->Fire();
  }

  for (b2Body* b = world->GetBodyList(); b; b = b->GetNext()) {
    int body_index = (int) b->GetUserData();
    if (body_index > 0) {
      float x = b->GetPosition().x * PTM_RATIO;
      float y = b->GetPosition().y * PTM_RATIO;
      if (body_index == m_PlayerIndex) {
        b2Vec2 player_velocity = b->GetLinearVelocity();
        if (player_velocity.x > PLAYER_MAX_VELOCITY_X) {
          player_velocity.x = PLAYER_MAX_VELOCITY_X;
        }
        if (player_velocity.x < -PLAYER_MAX_VELOCITY_X) {
          player_velocity.x = -PLAYER_MAX_VELOCITY_X;
        }
        if (player_velocity.y > PLAYER_MAX_VELOCITY_Y) {
          player_velocity.y = PLAYER_MAX_VELOCITY_Y;
        }
        b->SetLinearVelocity(player_velocity);
        m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[0] = 0.0;
        m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[1] = -1500.0;
      }
      m_AtlasSprites[body_index]->m_Rotation = RadiansToDegrees(b->GetAngle());
      m_AtlasSprites[body_index]->SetPosition(x, y);
    }
  }

  std::vector<MLContact>::iterator pos;
  for (pos = m_ContactListener->m_Contacts.begin(); pos != m_ContactListener->m_Contacts.end(); ++pos) {
    MLContact contact = *pos;
    b2Body *bodyA = contact.fixtureA->GetBody();
    b2Body *bodyB = contact.fixtureB->GetBody();
    int indexA = (int)bodyA->GetUserData();
    int indexB = (int)bodyB->GetUserData();
    if (indexA == m_PlayerIndex || indexB == m_PlayerIndex) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA <= m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB <= m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex == -1) {
          m_PickupTimeout = 0.0;
          if (bodyA == m_PlayerBody) {
            m_PickedUpPartIndex = indexB;
          } else {
            m_PickedUpPartIndex = indexA;
          }
          m_PickupJointDef->bodyA = bodyA;
          m_PickupJointDef->bodyB = bodyB;
          m_PickupJoint = (b2RopeJoint *)world->CreateJoint(m_PickupJointDef);
          LOGV("player touched part\n");
        }
      }
    } else if (indexA == -1 || indexB == -1) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA <= m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB <= m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex != -1 && (indexA == m_PickedUpPartIndex || indexB == m_PickedUpPartIndex)) {
          if (m_PickupTimeout > 2.0) {
            m_PickedUpPartIndex = -1;
            world->DestroyJoint(m_PickupJoint);
            LOGV("break joint\n");
          }
        }
      }
    }
  }

  return 1;
}


void SpaceShipDown::RenderModelPhase() {
}


void SpaceShipDown::RenderSpritePhase() {

  RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1);
  AtlasSprite::ReleaseBuffers();

  if (true) {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    world->DrawDebugData();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1);
  RenderSpriteRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex);
  glDisable(GL_BLEND);
  AtlasSprite::ReleaseBuffers();

}

    /*
    b2Vec2 vel = m_PlayerBody->GetLinearVelocity();
    float angle = atan2f(vel.y, vel.x);
    float rotation = RadiansToDegrees(angle);
    */

  //b2DistanceJointDef dj;
  //dj.Initialize(part_body, m_PlayerBody, part_body->GetPosition(), m_PlayerBody->GetPosition());
  //dj.collideConnected = true;
  //b2DistanceJoint *m_distanceJoint = (b2DistanceJoint*) world->CreateJoint(&dj);

/*
  if (false) {
    b2RopeJointDef *rjd = new b2RopeJointDef();
    rjd->bodyA = part_body;
    rjd->bodyB = m_PlayerBody;
    rjd->localAnchorA = b2Vec2(0.0, 0.0);
    rjd->localAnchorB = b2Vec2(0.0, 0.0);
    rjd->maxLength = 500.0 / PTM_RATIO;
    b2RopeJoint *rj = (b2RopeJoint *)world->CreateJoint(rjd);
  }
*/
