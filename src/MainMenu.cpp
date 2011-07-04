// Jon Bardin GPL


#include "MemoryLeak.h"
#include "MainMenu.h"


static const GLfloat globalAmbient[4]      = { 5.9, 5.9, 5.9, 1.0 };
static const GLfloat lightDiffuseLamp[4]   = { 1.0, 1.0, 1.0, 1.0 };
static const GLfloat lightAmbientLamp[4]   = { 0.74, 0.74, 0.74, 1.0 };
static const GLfloat lightPositionLamp[4]  = { 0.0, 0.0, 0.0, 0.0 };


MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LOGV("main menu alloc\n");
  LoadSound(0);
  m_IsPushingAudio = true;
  m_RequestedFullscreen = false;
  LoadModel(0, 0, 9);
  Model *f = new Model(m_FooFoos.at(0), m_Textures->at(2));
	m_Models.push_back(f);

  float r = 0;
  float d = 40.0;
  int c = 0;

  for (unsigned int i=0; i<c; i++) {
    float tx = -sin(DEGREES_TO_RADIANS(r));
    float tz = cos(DEGREES_TO_RADIANS(r));
    r += 360 / c;
    float x = (tx * d);
    float z = (tz * d);
    f = new Model(m_FooFoos.at(0), m_Textures->at(0));
	  m_Models.push_back(f);
    //delete f;
    m_ModelCount++;
    m_Models[i + 1]->SetPosition(x, 0, z);
    m_Models[i + 1]->SetScale(1.0, 10.0, 1.0);
    //m_Models[i + 1]->m_Frame = (i % (9 * 30));
  }

  float r2 = 0;
  float d2 = 80.0;
  int c2 = 0;

  for (unsigned int i=0; i<c2; i++) {
    float tx = -sin(DEGREES_TO_RADIANS(r2));
    float tz = cos(DEGREES_TO_RADIANS(r2));
    r2 += 360 / c2;
    float x = (tx * d2);
    float z = (tz * d2);
    f = new Model(m_FooFoos.at(0), m_Textures->at(0));
	  m_Models.push_back(f);
    //delete f;
    m_ModelCount++;
    m_Models[i + 1 + c]->SetPosition(x, 0, z);
    m_Models[i + 1 + c]->SetScale(1.0, 100.0, 1.0);
    //m_Models[i + 1 + c]->m_Frame = (i % (9 * 30));
  }

  m_CameraX = 0.0;
  m_CameraY = 0.0;
  m_CameraZ = 0.0;
  m_CameraR = 0.0;



  //delete f;
}


MainMenu::~MainMenu() {
  LOGV("main menu dealloc\n");
}


void MainMenu::Hit(float x, float y, int hitState) {
  LOGV("main menu hit\n");
}


int MainMenu::Simulate() {
  if (m_RequestedFullscreen) {
    // do nothing really
  } else {
    if (PushMessageToWebView(CreateWebViewFunction("fullscreen()"))) {
      m_RequestedFullscreen = true;
    }
  }

  for (unsigned int i=0; i<m_ModelCount; i++) {
    m_Models[i]->Simulate(m_DeltaTime);
  }

  float vf = -fastSinf(m_SimulationTime) * 0.75;
  float vu = 0.0;
  float vs = 0.0;

  float tx = -sin(DEGREES_TO_RADIANS(m_CameraR));
  float tz = cos(DEGREES_TO_RADIANS(m_CameraR));

  float txx = -sin(DEGREES_TO_RADIANS(m_CameraR + 90.0));
  float tzz = cos(DEGREES_TO_RADIANS(m_CameraR + 90.0));

  /*
  m_CameraX += (tx * vf * m_DeltaTime) + (txx * vs * m_DeltaTime);
  m_CameraY += (vu * m_DeltaTime);
  m_CameraZ += (tz * vf * m_DeltaTime) + (tzz * vs * m_DeltaTime);

  m_CameraTarget[0] = 0.0; m_CameraX + (tx * 100.0);
  m_CameraTarget[1] = 0.0; m_CameraY + fastSinf(m_SimulationTime * 0.25) * 15.0;
  m_CameraTarget[2] = 0.0; m_CameraZ + (tz * 100.0);
  m_CameraPosition[0] = m_CameraX;
  m_CameraPosition[1] = m_CameraY;
  m_CameraPosition[2] = m_CameraZ;

  m_CameraR -= 0.0; //m_DeltaTime * (0.0 + fastSinf(m_SimulationTime) * 0.0);

  */
  
  
  m_CameraTarget[0] = 0.0;
  m_CameraTarget[1] = 0.0;
  m_CameraTarget[2] = 0.0;
  m_CameraPosition[0] = 8.5;
  m_CameraPosition[1] = 1.0;
  m_CameraPosition[2] = 8.5;
  
  
  for (unsigned int i=0; i<m_ModelCount; i++) {
    m_Models[i]->m_Rotation[1] += m_DeltaTime * 20.0;
  }

  return 1;
}


void MainMenu::RenderModelPhase() {
  float FogCol[3] = {0.91f,0.91f,0.91f}; // Define a nice light grey
  glClearColor(FogCol[0], FogCol[1], FogCol[2], 1.0);
  
  /* Global ambient light. */
  static const GLfloat globalAmbient[4]      = { 0.5, 0.5, 0.5, 1.0 };
  
  /* Lamp parameters. */
  static const GLfloat lightDiffuseLamp[4]   = { 10.0, 10.0, 10.0, 1.0 };
  static const GLfloat lightAmbientLamp[4]   = { 0.5, 0.5, 0.5, 1.0 };
  static const GLfloat lightPositionLamp[4]  = { 0.0, 0.0, 0.0, 0.0 };
  
  glEnable(GL_FOG);
  glFogfv(GL_FOG_COLOR, FogCol);     // Set the fog color
  glFogx(GL_FOG_MODE, GL_EXP); // Note the 'i' after glFog - the GL_LINEAR constant is an integer.
  glFogf(GL_FOG_START, 0.0);
  glFogf(GL_FOG_END, 1.0);
  glFogf(GL_FOG_DENSITY, 0.05);
  
  
  glEnable(GL_LIGHTING);
  glLightModelfv( GL_LIGHT_MODEL_AMBIENT, globalAmbient );
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuseLamp  );
  glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbientLamp  );
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightDiffuseLamp  );
  glLightfv(GL_LIGHT0, GL_POSITION, lightPositionLamp );

  
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  
  
  
  
  //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
  
  RenderModelRange(0, m_ModelCount);
  
  /*
  int jitter;
  int numJitters = 1;
  for(jitter=0; jitter<numJitters; jitter++) { //draw scene 8 times
    GLfloat eyedx = 0.04 * jitter;
    GLfloat eyedy = 0.04 * jitter;
    GLfloat eyedz = 0.04 * jitter;

    GLfloat focus = -6;
    
    GLfloat dx,dy;
    glLoadIdentity();
    glueLookAt(m_CameraPosition[0] + eyedx, m_CameraPosition[1] + eyedy, m_CameraPosition[2] + eyedz, m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);

    glColor4f(1.0, 1.0, 1.0, 0.05);
    RenderModelRange(0, m_ModelCount);
  }
  */
  
  glDisable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_FOG);
}


void MainMenu::RenderSpritePhase() {
}
