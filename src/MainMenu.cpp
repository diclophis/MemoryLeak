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
  LoadModel(3, 0, 10);
  //LoadModel(0, 0, 30);
  CreateFoos();
}


MainMenu::~MainMenu() {
  LOGV("main menu dealloc\n");
}


void MainMenu::CreateFoos() {
	m_Models.push_back(new Model(m_FooFoos.at(0)));
  m_Models[0]->SetPosition(0, 0, 0);
  m_Models[0]->SetScale(1.0, 1.0, 1.0);
  ResetStateFoo();
  m_BatchFoo = Model::GetBatchFoo(m_Textures->at(5), 2600, 1);
}


void MainMenu::DestroyFoos() {
}


void MainMenu::Hit(float x, float y, int hitState) {
}


int MainMenu::Simulate() {
  m_CameraTarget[0] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraTarget[1] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraTarget[2] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 10.0) - 5.0);
  m_CameraPosition[0] = 50.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0); //m_SimulationTime * 100.0;//219.5;
  m_CameraPosition[1] = 0.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0); //m_SimulationTime * 0.75;//300.0;
  m_CameraPosition[2] = -50.0 + ((fastSinf(m_SimulationTime * 1.0) * 10.0) - 5.0); //m_SimulationTime * 0.75;//219.5;
  m_Models[0]->Simulate(m_DeltaTime, false);
  return 1;
}


void MainMenu::RenderModelPhase() {

  RenderModelRange(0, 1, m_BatchFoo);
  Model::RenderFoo(m_StateFoo, m_BatchFoo);
  
}


void MainMenu::RenderSpritePhase() {
}
