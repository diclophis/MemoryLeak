// Jon Bardin GPL


#include "MemoryLeak.h"
#include "RadiantFireEightSixOne.h"


RadiantFireEightSixOne::RadiantFireEightSixOne(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(2);
  m_IsPushingAudio = true;
  m_Zoom = 2.0;
  m_RequestedFullscreen = false;
  m_Touched = false;
  CreateBox2DWorld();
  terrain = new Terrain(world, m_Textures->at(0));  
  hero = new Hero(world, m_Textures->at(1));
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 8, 8, 20, 21, 1.0, "", 8, 11, 1.0, 100.0, 100.0));
  m_AtlasSprites[m_SpriteCount]->SetPosition(100.0, 100.0);
  m_AtlasSprites[m_SpriteCount]->m_IsAlive = true;
  m_AtlasSprites[m_SpriteCount]->Build(1);
  b2BodyDef spriteBodyDef;
  spriteBodyDef.type = b2_dynamicBody;
  spriteBodyDef.position.Set(m_AtlasSprites[m_SpriteCount]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_SpriteCount]->m_Position[1] / PTM_RATIO);
  spriteBodyDef.userData = m_AtlasSprites[m_SpriteCount];
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
  //GLuint iii[1];
  //iii[0] = terrain->m_Textures.at(0);
  //glDeleteTextures(1, iii);
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
  if (!hero->awake) {
    hero->Wake();
  }
  if (m_Touched) {
    hero->Dive();
  }
  hero->LimitVelocity();
  int32 velocityIterations = 1; //4; //8;
  int32 positionIterations = 1; //2; //3;
  float dt = m_DeltaTime;
  world->Step(dt, velocityIterations, positionIterations);
  hero->UpdateNodePosition();
  //terrain->SetOffsetX(hero->position.x);
  return 1;
}


void RadiantFireEightSixOne::RenderModelPhase() {
}


void RadiantFireEightSixOne::RenderSpritePhase() {
  //glTranslatef(terrain->position.x - 128.0, -175.0, 0.0);
  //terrain->Render();
  //glEnable(GL_BLEND);
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  //hero->Render();
  //RenderSpriteRange(0, 1);
  //glDisable(GL_BLEND);
}
