// Jon Bardin GPL


#include "MemoryLeak.h"
#include "RadiantFireEightSixOne.h"


RadiantFireEightSixOne::RadiantFireEightSixOne(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(2);
  m_IsPushingAudio = true;
  m_Zoom = 2.0;
  m_RequestedFullscreen = false;
  m_Touched = false;
  b2BodyDef spriteBodyDef;
  spriteBodyDef.type = b2_dynamicBody;
  CreateBox2DWorld();
  terrain = new Terrain(world, m_Textures->at(0));  
  hero = NULL;
  hero = new Hero(world, m_Textures->at(1));
  spriteBodyDef.position.Set(hero->position.x / PTM_RATIO, hero->position.y / PTM_RATIO);
  b2Body *spriteBody = world->CreateBody(&spriteBodyDef);
  b2PolygonShape spriteShape;
  spriteShape.SetAsBox(100.0 / PTM_RATIO / 2, 100.0 / PTM_RATIO / 2);
  b2FixtureDef spriteShapeDef;
  spriteShapeDef.shape = &spriteShape;
  spriteShapeDef.density = 10.0;
  spriteShapeDef.isSensor = true;
  spriteBody->CreateFixture(&spriteShapeDef);
}


void RadiantFireEightSixOne::CreateBox2DWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0f, -9.8f);
  world = new b2World(gravity, false);
}


RadiantFireEightSixOne::~RadiantFireEightSixOne() {
  delete terrain;
  delete hero;
  delete world;
}


void RadiantFireEightSixOne::Hit(float x, float y, int hitState) {
  if (hitState == 0) {
    m_Touched = true;
  } else if (hitState == 2) {
    m_Touched = false;
  }
}


int RadiantFireEightSixOne::Simulate() {
  int32 velocityIterations = 1;
  int32 positionIterations = 1;

  world->Step(m_DeltaTime, velocityIterations, positionIterations);

  if (hero) {
    if (!hero->awake) {
      hero->Wake();
    }
    if (m_Touched) {
      hero->Dive();
    }
    hero->LimitVelocity();
    hero->UpdateNodePosition();
    terrain->SetOffsetX(hero->position.x);
  }
  
  //terrain->SetOffsetX(m_SimulationTime * 300);

  return 1;
}


void RadiantFireEightSixOne::RenderModelPhase() {
}


void RadiantFireEightSixOne::RenderSpritePhase() {
  glPushMatrix();
  {
    glTranslatef(terrain->position.x - 128.0, -175.0, 0.0);
    terrain->Render();
    if (hero) {
      hero->Render();
    }
    AtlasSprite::ReleaseBuffers();
  }
  glPopMatrix();
}
