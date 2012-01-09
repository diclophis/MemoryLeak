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
  for (unsigned int i=0; i<100; i++) {
    m_Models.push_back(new Model(m_FooFoos.at(0)));
    m_Models[i]->SetPosition(i * 105.0, 0.0, i * 105.0);
    m_Models[i]->SetScale(1.0, 1.0, 1.0);
    m_ModelCount++;
  }
  m_BatchFoo = Model::GetBatchFoo(m_Textures->at(5), m_FooFoos[0]->m_numFaces, m_ModelCount);
  ResetStateFoo();
}


void MainMenu::DestroyFoos() {
}


void MainMenu::Hit(float x, float y, int hitState) {
}


int MainMenu::Simulate() {
  m_CameraTarget[0] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraTarget[1] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraTarget[2] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraPosition[0] = 150.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0); //m_SimulationTime * 100.0;//219.5;
  m_CameraPosition[1] = 500.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0); //m_SimulationTime * 0.75;//300.0;
  m_CameraPosition[2] = -150.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0); //m_SimulationTime * 0.75;//219.5;
  m_Models[0]->Simulate(m_DeltaTime, false);
  return 1;
}


void MainMenu::RenderModelPhase() {
  RenderModelRange(0, m_ModelCount, m_BatchFoo);
  Model::RenderFoo(m_StateFoo, m_BatchFoo);
}


void MainMenu::RenderSpritePhase() {
}
