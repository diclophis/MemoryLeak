// Jon Bardin GPL


#include "MemoryLeak.h"
#include "MainMenu.h"


MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("main menu alloc\n");

  m_CameraX = 0.0;
  m_CameraY = 0.0;
  m_CameraZ = 0.0;
  m_CameraR = 0.0;

  m_IsThreeD = true;

  LoadSound(0);
  LoadModel(1, 0, 1);
  CreateFoos();
}


MainMenu::~MainMenu() {
  LOGV("main menu dealloc\n");
}


void MainMenu::CreateFoos() {
  float x = 0;
  float y = 0;
  for (unsigned int i=0; i<400; i++) {
    m_Models.push_back(new Model(m_FooFoos.at(0)));
    m_Models[i]->SetPosition(x, fastSinf((float)i * 0.5) * 100.0, y);
    x += 105.0;
    m_ModelCount++;
    if ((m_ModelCount % 20) == 0) {
      x = 0.0;
      y += 105.0;
    }
  }
  m_BatchFoo = Model::GetBatchFoo(m_Textures->at(5), m_FooFoos[0]->m_numFaces, m_ModelCount);
  ResetStateFoo();
}


void MainMenu::DestroyFoos() {
}


void MainMenu::Hit(float x, float y, int hitState) {
}


int MainMenu::Simulate() {
  m_CameraTarget[0] = 200.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraTarget[1] = -200.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraTarget[2] = 200.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraPosition[0] = 1500.0 + ((fastSinf(m_SimulationTime * 1.0) * 900.0) - 300.0);
  m_CameraPosition[1] = 300.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0);
  m_CameraPosition[2] = 1500.0 + ((fastSinf(m_SimulationTime * 1.0) * 900.0) - 300.0);
  m_Models[0]->Simulate(m_DeltaTime, false);
  RenderModelRange(0, m_ModelCount, m_BatchFoo);
  return 1;
}


void MainMenu::RenderModelPhase() {
  Model::RenderFoo(m_StateFoo, m_BatchFoo);
}


void MainMenu::RenderSpritePhase() {
}
