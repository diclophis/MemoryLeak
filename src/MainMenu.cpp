// Jon Bardin GPL


#include "MemoryLeak.h"
#include "MainMenu.h"


MainMenu::MainMenu(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {
  LOGV("main menu alloc\n");

  m_CameraX = 0.0;
  m_CameraY = 0.0;
  m_CameraZ = 0.0;
  m_CameraR = 0.0;

  //m_IsThreeD = true;
  m_TouchingLeft = false;
  m_TouchingRight = false;

  m_MaxDistanceX = 1.2;
  m_MaxDistanceY = 1.2;
  m_MaxDepth = -0.25;
  m_MaxHeight = 0.75;
  m_Fov = 10.0;

  LoadSound(3);
  m_CurrentTempo = ModPlug_GetCurrentTempo(m_Sounds[0]);
  //8 is spaceman
  //7 is jetpack
  //6 is truss
  LoadModel(0, 0, 1);
  CreateFoos();
}


MainMenu::~MainMenu() {
  LOGV("main menu dealloc\n");
}


void MainMenu::CreateFoos() {
  float x = 0;
  float z = 0;
  if (true) {
    m_Models.push_back(new Model(m_FooFoos.at(0)));
    m_ModelCount++;
  } else {
    int chubes = 5;
    for (unsigned int i=0; i<(chubes * chubes); i++) {
      m_Models.push_back(new Model(m_FooFoos.at(0)));
      m_Models[i]->SetPosition(x, 0.0, z);
      m_Models[i]->m_Life = (float)i;
      x += 1.0;
      m_ModelCount++;
      if ((m_ModelCount % chubes) == 0) {
        m_MaxDistanceX = x;
        x = 0.0;
        z += 1.0;
      }
      m_MaxDistanceY = z;
    }
    //m_MaxDepth = -1.0;
    //m_MaxHeight = 1.0;
  }

  m_BatchFoo = Model::GetBatchFoo(m_Textures.at(5), m_FooFoos[0]->m_numFaces, m_ModelCount);
  ResetStateFoo();
  m_BatchFoo->m_NumBatched = 0;
  RenderModelRange(0, m_ModelCount, m_BatchFoo);
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
  
  //LOGV("wtf: %d\n", ModPlug_GetCurrentSpeed(m_Sounds[0]));
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
  if (false) {
    m_CameraTarget[0] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 0.7) - 0.5);
    m_CameraTarget[1] = m_MaxDepth + ((fastSinf(m_SimulationTime * -0.7) * 1.0) - 0.5);
    m_CameraTarget[2] = 0.0 + ((fastSinf(m_SimulationTime * -1.0) * 0.7) - 0.5);
    m_CameraPosition[0] = m_MaxDistanceX + ((fastSinf(m_SimulationTime * -0.1) * 0.5 * m_MaxDistanceX) - (0.25 * m_MaxDistanceX));
    m_CameraPosition[1] = m_MaxHeight + ((fastSinf(m_SimulationTime * 0.05) * 0.5 * m_MaxHeight) - (0.25 * m_MaxHeight));
    m_CameraPosition[2] = m_MaxDistanceY + ((fastSinf(m_SimulationTime * 0.15) * 0.5 * m_MaxDistanceY) - (0.25 * m_MaxDistanceY));
  } else {
    m_CameraTarget[0] = 0.0;
    m_CameraTarget[1] = 0.0;
    m_CameraTarget[2] = 0.0;
    //m_CameraPosition[0] = 1.5 + fastSinf(m_SimulationTime * -0.25);
    //m_CameraPosition[1] = 1.5 + fastSinf(m_SimulationTime * 0.1);
    //m_CameraPosition[2] = 1.5;
    m_CameraPosition[0] = 2.75;
    m_CameraPosition[1] = 2.5;
    m_CameraPosition[2] = 2.75;
  }
  /*
  for (unsigned int i=0; i<m_ModelCount; i++) {
    m_Models[i]->Simulate(m_SimulationTime, m_DeltaTime, false);
  }
  */
  return 1;
}


void MainMenu::RenderModelPhase() {
  Model::RenderFoo(m_StateFoo, m_BatchFoo, true);
  m_BatchFoo->m_NeedsCopy = false;
}


void MainMenu::RenderSpritePhase() {
}
