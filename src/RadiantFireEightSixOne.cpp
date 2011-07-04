// Jon Bardin GPL


#include "MemoryLeak.h"
#include "RadiantFireEightSixOne.h"


RadiantFireEightSixOne::RadiantFireEightSixOne(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("alloc RadiantFire\n");
  LoadSound(2);
  m_IsPushingAudio = true;
  m_Zoom = 3.0;

  m_Touched = false;



  CreateBox2DWorld();

  terrain = new Terrain(world, m_Textures->at(0));
  terrain->SetOffsetX(0.0);

  hero = new Hero(world, m_Textures->at(1));

  //hero->sprite->m_Texture += 2;
    
}


void RadiantFireEightSixOne::CreateBox2DWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0f, -9.8f);
  world = new b2World(gravity, false);

/*
render = new GLESDebugDraw(PTM_RATIO);
world->SetDebugDraw(render);
uint32 flags = 0;
flags += b2Draw::e_shapeBit;
render->SetFlags(flags);
*/

}


RadiantFireEightSixOne::~RadiantFireEightSixOne() {
  LOGV("dealloc RadiantFire\n");

  delete terrain;
  delete hero;
  delete world;
}


void RadiantFireEightSixOne::Hit(float x, float y, int hitState) {
  //LOGV("hit RadiantFire\n");

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
      //LOGV("wake\n");
      hero->Wake();
      m_Touched = false;
    } else {
      //LOGV("dive\n");
      hero->Dive();
    }
  }

  hero->LimitVelocity();
  int32 velocityIterations = 8;
  int32 positionIterations = 3;
  float dt = m_DeltaTime;
  //LOGV("dt: %f\n", dt);
  world->Step(dt, velocityIterations, positionIterations);

  hero->UpdateNodePosition();
  terrain->SetOffsetX(hero->position.x);

  return 1;
  
/*
float height = _hero.position.y;
const float minHeight = screenH*4/5;
if (height < minHeight) {
height = minHeight;
}
float scale = minHeight / height;
_terrain.scale = scale;
_terrain.offsetX = _hero.position.x;
[_sky setOffsetX:_terrain.offsetX*0.2f];
[_sky setScale:1.0f-(1.0f-scale)*0.75f];

*/

}


void RadiantFireEightSixOne::RenderModelPhase() {
}


void RadiantFireEightSixOne::RenderSpritePhase() {
  glTranslatef(0.0, -200.0, 0.0);

  //AtlasSprite::Scrub();
  terrain->Render();

  AtlasSprite::Scrub();

  hero->Render();
}
