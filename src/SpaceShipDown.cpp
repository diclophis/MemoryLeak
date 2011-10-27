// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SpaceShipDown.h"


SpaceShipDown::SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(2);
  m_IsPushingAudio = true;
  m_Zoom = 1.0;

  m_LandscapeIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(4), 1, 1, 0, 64, 1.0, "", 0, 64, 0.0, 1024, 1024));
  m_AtlasSprites[m_LandscapeIndex]->Build(0);
  m_AtlasSprites[m_LandscapeIndex]->SetPosition(0.0, 0.0);
  m_SpriteCount++;

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 1, 1, 0, 64, 1.0, "", 0, 64, 0.0, 50, 50));
  m_AtlasSprites[m_PlayerIndex]->Build(0);
  m_AtlasSprites[m_PlayerIndex]->SetPosition(0.0, 0.0);
  m_SpriteCount++;

  CreateBox2DWorld();

  /*
  b2BodyDef spriteBodyDef;
  spriteBodyDef.type = b2_dynamicBody;
  spriteBodyDef.position.Set(m_AtlasSprites[m_PlayerIndex]->position[0] / PTM_RATIO, m_AtlasSprites[m_PlayerIndex]->position[0] / PTM_RATIO);
  b2Body *spriteBody = world->CreateBody(&spriteBodyDef);
  b2PolygonShape spriteShape;
  spriteShape.SetAsBox(100.0 / PTM_RATIO / 2, 100.0 / PTM_RATIO / 2);
  b2FixtureDef spriteShapeDef;
  spriteShapeDef.shape = &spriteShape;
  spriteShapeDef.density = 10.0;
  spriteShapeDef.isSensor = true;
  spriteBody->CreateFixture(&spriteShapeDef);
  */

  float radius = 5;
  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_PlayerIndex]->m_Position[0], m_AtlasSprites[m_PlayerIndex]->m_Position[1]);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0f;
  bd.fixedRotation = true;
  m_PlayerBody = world->CreateBody(&bd);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = 2.0f;
  fd.restitution = 0.0;
  fd.friction = 0.0;
  m_PlayerBody->CreateFixture(&fd);
  m_PlayerBody->SetActive(true);

  float desiredAngle = atan2f(1.0, 1.0);
  m_PlayerBody->SetTransform(m_PlayerBody->GetPosition(), desiredAngle);

/*
  b2PolygonShape polygonShape;
  bd.type = b2_staticBody;
  bd.position.Set(0, 0);
  polygonShape.SetAsBox(10.0 / PTM_RATIO, 10.0 / PTM_RATIO);
  world->CreateBody(&bd)->CreateFixture(&fd);
*/

  m_TouchedLeft = false;
  m_TouchedRight = false;

}


void SpaceShipDown::CreateBox2DWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0, -9.8);
  world = new b2World(gravity, false);
}


SpaceShipDown::~SpaceShipDown() {
  delete world;
}


void SpaceShipDown::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;

  LOGV("%f %f\n", xx, yy);
  if (hitState == 0) {
    if (xx > 0) {
      m_TouchedRight = true;
    } else {
      m_TouchedLeft = true;
    }
  } else if (hitState == 2) {
    m_TouchedLeft = false;
    m_TouchedRight = false;
  }
}


int SpaceShipDown::Simulate() {
  int velocityIterations = 1;
  int positionIterations = 1;

  world->Step(m_DeltaTime, velocityIterations, positionIterations);

  b2Vec2 forcePosition = m_PlayerBody->GetWorldCenter();

  if (m_TouchedLeft) {
    m_PlayerBody->ApplyForce(b2Vec2(-0.5, 5), forcePosition);
  }

  if (m_TouchedRight) {
    m_PlayerBody->ApplyForce(b2Vec2(0.5, 5), forcePosition);
  }

  float x = m_PlayerBody->GetPosition().x * PTM_RATIO;
  float y = m_PlayerBody->GetPosition().y * PTM_RATIO;
  MLPoint position = MLPointMake(x, y);

  /*
  b2Vec2 vel = m_PlayerBody->GetLinearVelocity();
  float angle = atan2f(vel.y, vel.x);
  float rotation = RadiansToDegrees(angle);
  */

  m_AtlasSprites[m_PlayerIndex]->m_Rotation = RadiansToDegrees(m_PlayerBody->GetAngle());

  m_AtlasSprites[m_PlayerIndex]->SetPosition(position.x, position.y);

  return 1;
}


void SpaceShipDown::RenderModelPhase() {
}


void SpaceShipDown::RenderSpritePhase() {
  glPushMatrix();
  {
    glTranslatef(-m_AtlasSprites[m_PlayerIndex]->m_Position[0], -m_AtlasSprites[m_PlayerIndex]->m_Position[1], 0.0);
    RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1);
    RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1);
    AtlasSprite::ReleaseBuffers();
  }
  glPopMatrix();
}
