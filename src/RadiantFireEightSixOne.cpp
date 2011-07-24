// Jon Bardin GPL


#include "MemoryLeak.h"
#include "RadiantFireEightSixOne.h"


RadiantFireEightSixOne::RadiantFireEightSixOne(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("alloc RadiantFire\n");
  LoadSound(2);
  m_IsPushingAudio = true;
  m_Zoom = 2.0;

  m_RequestedFullscreen = false;
  m_Touched = false;

  
  CreateBox2DWorld();
  terrain = new Terrain(world, m_Textures->at(0));  
  hero = new Hero(world, m_Textures->at(1));
  
}


void RadiantFireEightSixOne::CreateBox2DWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0f, -9.8f);
  world = new b2World(gravity, false);
}


RadiantFireEightSixOne::~RadiantFireEightSixOne() {
  LOGV("dealloc RadiantFire\n");
  
  GLuint iii[1];
  iii[0] = terrain->m_Textures.at(0);
  LOGV("destroy in other?: %d\n", iii[0]);
  glDeleteTextures(1, iii);
  
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
  
  if (hitState == 1) {
    //PushMessageToWebView(CreateWebViewFunction("fullscreen()"));
  }

  //terrain->Reset();
  //hero->Reset();
}


int RadiantFireEightSixOne::Simulate() {
  /*
  if (m_RequestedFullscreen) {
    // do nothing really
  } else {
    if (PushMessageToWebView(CreateWebViewFunction("fullscreen()"))) {
      m_RequestedFullscreen = true;
    }
  }
  */
  
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
  terrain->SetOffsetX(hero->position.x);

  return 1;

}


void RadiantFireEightSixOne::RenderModelPhase() {
}


void RadiantFireEightSixOne::RenderSpritePhase() {
  glPushMatrix();
  {
    glTranslatef(terrain->position.x - 128.0, -175.0, 0.0);

    terrain->Render();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    AtlasSprite::Scrub();
    hero->Render();
    glDisable(GL_BLEND);
  }
  glPopMatrix();
}
