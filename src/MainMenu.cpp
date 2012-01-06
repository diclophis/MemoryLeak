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
  Model::ReleaseBuffers();
  AtlasSprite::ReleaseBuffers();
  LOGV("main menu dealloc\n");
}


void MainMenu::CreateFoos() {
  Model *f = new Model(m_FooFoos.at(0));
	m_Models.push_back(f);
  m_Models[0]->SetPosition(0, 0, 0);
  m_Models[0]->SetScale(1.0, 1.0, 1.0);
  ResetStateFoo();
  m_BatchFoo = Model::GetBatchFoo(m_Textures->at(0), 1024, 4);
}


void MainMenu::DestroyFoos() {
}


void MainMenu::Hit(float x, float y, int hitState) {
}


int MainMenu::Simulate() {

  //for (unsigned int i=0; i<m_ModelCount; i++) {
  //  m_Models[i]->Simulate(m_DeltaTime);
  //}

  m_CameraTarget[0] = 50.0 + (m_SimulationTime * 0.0);
  m_CameraTarget[1] = 50.0 + (m_SimulationTime * 0.0);
  m_CameraTarget[2] = 50.0 + (m_SimulationTime * 0.0);
  m_CameraPosition[0] = 120.0 + (fastSinf(m_SimulationTime * 1.0) * 50.0); //m_SimulationTime * 100.0;//219.5;
  m_CameraPosition[1] = 120.0 + (fastSinf(m_SimulationTime * 2.0) * 50.0); //m_SimulationTime * 0.75;//300.0;
  m_CameraPosition[2] = 120.0 + (fastSinf(m_SimulationTime * 3.0) * 50.0); //m_SimulationTime * 0.75;//219.5;
  
  //for (unsigned int i=0; i<m_ModelCount; i++) {
  //  m_Models[i]->m_Rotation[1] += m_DeltaTime * 100.0;
  //}

  return 1;
}


void MainMenu::RenderModelPhase() {
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  //glScalef(m_SimulationTime, m_SimulationTime, m_SimulationTime);

  RenderModelRange(0, 1, m_BatchFoo);
  Model::RenderFoo(m_StateFoo, m_BatchFoo);
  Model::ReleaseBuffers();
  
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}


void MainMenu::RenderSpritePhase() {
}
