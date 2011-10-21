// Jon Bardin GPL


#include "MemoryLeak.h"
#include "MainMenu.h"


MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("main menu alloc\n");
  LoadSound(0);
  m_IsPushingAudio = true;
  m_RequestedFullscreen = false;
  LoadModel(1, 15, 58);
  Model *f = new Model(m_FooFoos.at(0), m_Textures->at(3));
	m_Models.push_back(f);
  m_Models[0 + 0]->SetPosition(0, 0, 0);
  m_Models[0 + 0]->SetScale(1.0, 1.0, 1.0);

  m_CameraX = 0.0;
  m_CameraY = 0.0;
  m_CameraZ = 0.0;
  m_CameraR = 0.0;
}


MainMenu::~MainMenu() {
  Model::ReleaseBuffers();
  LOGV("main menu dealloc\n");
}


void MainMenu::Hit(float x, float y, int hitState) {
  if (hitState == 1) {
    PushMessageToWebView(CreateWebViewFunction("fullscreen()"));
  } else if (hitState == 0) {
    PushMessageToWebView(CreateWebViewFunction("show()"));
  }
}


int MainMenu::Simulate() {
  
  for (unsigned int i=0; i<m_ModelCount; i++) {
    m_Models[i]->Simulate(m_DeltaTime);
  }

  m_CameraTarget[0] = 0.0;
  m_CameraTarget[1] = 0.0;
  m_CameraTarget[2] = 0.0;
  m_CameraPosition[0] = 50.0 + m_SimulationTime * 0.75;//219.5;
  m_CameraPosition[1] = 50.0 + m_SimulationTime * 0.75;//300.0;
  m_CameraPosition[2] = 50.0 + m_SimulationTime * 0.75;//219.5;
  
  
  for (unsigned int i=0; i<m_ModelCount; i++) {
    m_Models[i]->m_Rotation[1] += m_DeltaTime * 100.0;
  }

  return 1;
}


void MainMenu::RenderModelPhase() {
  RenderModelRange(0, m_ModelCount);
}


void MainMenu::RenderSpritePhase() {
}
