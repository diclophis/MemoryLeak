// Jon Bardin GPL


#include "MemoryLeak.h"
#include "RadiantFireEightSixOne.h"


RadiantFireEightSixOne::RadiantFireEightSixOne(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("alloc RadiantFire\n");
  LoadSound(2);
  m_IsPushingAudio = true;
  m_Zoom = 1.0;

  m_Touched = false;



  CreateBox2DWorld();

  //terrain = new Terrain(world, m_Textures->at(0));
  //terrain->SetOffsetX(0.0);

  hero = new Hero(world, m_Textures->at(1));

  //hero->sprite->m_Texture += 2;
    
}


void RadiantFireEightSixOne::CreateBox2DWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0f, -9.8f);
  world = new b2World(gravity, false);
}


RadiantFireEightSixOne::~RadiantFireEightSixOne() {
  LOGV("dealloc RadiantFire\n");
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

  //terrain->Reset();
  //hero->Reset();
}


int RadiantFireEightSixOne::Simulate() {

  if (m_Touched) {
    if (!hero->awake) {
      hero->Wake();
      m_Touched = false;
    } else {
      hero->Dive();
    }
  }

  hero->LimitVelocity();
  int32 velocityIterations = 8;
  int32 positionIterations = 3;
  float dt = m_DeltaTime;
  
  world->Step(dt, velocityIterations, positionIterations);

  hero->UpdateNodePosition();
  terrain->SetOffsetX(hero->position.x);

  return 1;

}


void RadiantFireEightSixOne::RenderModelPhase() {
}


void RadiantFireEightSixOne::RenderSpritePhase() {

  if (terrain == NULL) {
    terrain = new Terrain(world, m_Textures->at(0));
  }
  
  glPushMatrix();
  {
    AtlasSprite::Scrub();
    terrain->Render();
    
    hero->sprite->m_Texture = terrain->genTexture;
    
    AtlasSprite::Scrub();
    hero->Render();
  }
  glPopMatrix();
  
  //terrain->GenerateStripesTexture();
}
