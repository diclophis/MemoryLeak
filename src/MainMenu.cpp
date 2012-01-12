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
  m_TouchingLeft = false;
  m_TouchingRight = false;


  LoadSound(0);
  m_CurrentTempo = ModPlug_GetCurrentTempo(m_Sounds[0]);
  LoadModel(1, 0, 1);
  CreateFoos();
}


MainMenu::~MainMenu() {
  LOGV("main menu dealloc\n");
}


void MainMenu::CreateFoos() {
  float x = 0;
  float y = 0;
  for (unsigned int i=0; i<625; i++) {
    m_Models.push_back(new Model(m_FooFoos.at(0)));
    m_Models[i]->SetPosition(x, fastSinf((float)i * 0.5) * 100.0, y);
    m_Models[i]->m_Life = (float)i;
    //130
    x += 100.0;
    m_ModelCount++;
    if ((m_ModelCount % 25) == 0) {
      x = 0.0;
      y += 100.0;
    }
  }
  m_BatchFoo = Model::GetBatchFoo(m_Textures->at(6), m_FooFoos[0]->m_numFaces, m_ModelCount);
  ResetStateFoo();
}


void MainMenu::DestroyFoos() {
}


void MainMenu::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
  if (hitState == 0 && xx > 0.0) {
    m_TouchingRight = true;
    //m_IsPushingAudio = false;
  }
  //if (hitState == 2 && xx > 0.0) {
  //  m_TouchingRight = false;
  //}
  if (hitState == 0 && xx < 0.0) {
    m_TouchingLeft = true;
    //m_IsPushingAudio = false;
  }
  
  LOGV("wtf: %d\n", ModPlug_GetCurrentSpeed(m_Sounds[0]));
  //if (hitState == 2 && xx < 0.0) {
  //  m_TouchingLeft = false;
  //}
  if (hitState == 2) {
    m_TouchingRight = false;
    m_TouchingLeft = false;
    ModPlug_SetCurrentSpeed(m_Sounds[0], m_CurrentTempo);
  }
}


int MainMenu::Simulate() {
  if (m_TouchingLeft) {
    m_SimulationTime += (m_DeltaTime * 2.0);
    //ModPlug_SetCurrentSpeed(m_Sounds[0], ModPlug_GetCurrentTempo(m_Sounds[0]) + 1);
    ModPlug_SetCurrentSpeed(m_Sounds[0], m_CurrentTempo + 60);
    //m_Fov += 50.0 * m_DeltaTime;
  }
  if (m_TouchingRight) {
    m_SimulationTime -= (m_DeltaTime * 0.5);
    //ModPlug_SetCurrentSpeed(m_Sounds[0], ModPlug_GetCurrentTempo(m_Sounds[0]) - 1);
    ModPlug_SetCurrentSpeed(m_Sounds[0], m_CurrentTempo - 60);
    //m_Fov -= 50.0 * m_DeltaTime;
  }

  if (m_Fov < 20.0) {
    m_Fov = 20.0;
  }
  if (m_Fov > 120.0) {
    m_Fov = 120.0;
  }
  //m_Fov += m_DeltaTime;
  m_CameraTarget[0] = 300.0 + ((fastSinf(m_SimulationTime * -1.5) * 200.0) - 100.0);
  m_CameraTarget[1] = -2000.0 + ((fastSinf(m_SimulationTime * -1.5) * 200.0) - 100.0);
  m_CameraTarget[2] = 300.0 + ((fastSinf(m_SimulationTime * -1.5) * 200.0) - 100.0);
  m_CameraPosition[0] = 2400.0 + ((fastSinf(m_SimulationTime * -0.4) * 1000.0) - 300.0);
  m_CameraPosition[1] = 555.0 + ((fastSinf(m_SimulationTime * 0.2) * 200.0) - 100.0);
  m_CameraPosition[2] = 2400.0 + ((fastSinf(m_SimulationTime * 0.6) * 1000.0) - 300.0);
  for (unsigned int i=0; i<m_ModelCount; i++) {
    m_Models[i]->Simulate(m_SimulationTime, m_DeltaTime, false);
  }
  m_BatchFoo->m_NumBatched = 0;
  RenderModelRange(0, m_ModelCount, m_BatchFoo);
  return 1;
}


void MainMenu::RenderModelPhase() {
  Model::RenderFoo(m_StateFoo, m_BatchFoo);
}


void MainMenu::RenderSpritePhase() {
}
