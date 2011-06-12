// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"
#include "FlightControl.h"

#define SUBDIVIDE 50.0
#define BARREL_ROTATE_TIMEOUT 0.33
#define BARREL_ROTATE_PER_TICK 0 
#define SHOOT_VELOCITY 425.0
#define GRID_X 13 
#define GRID_Y 13 
#define COLLIDE_TIMEOUT 0.001
#define BARREL_SHOT_LENGTH 7 

/* Global ambient light. */
static const GLfloat globalAmbient[4]      = { 5.9, 5.9, 5.9, 1.0 };

/* Lamp parameters. */
static const GLfloat lightDiffuseLamp[4]   = { 1.0, 1.0, 1.0, 1.0 };
static const GLfloat lightAmbientLamp[4]   = { 0.74, 0.74, 0.74, 1.0 };
static const GLfloat lightPositionLamp[4]  = { 0.0, 0.0, 0.0, 0.0 };

enum colliders {
  BARREL = 1,
  MIRROR = 2 
};


FlightControl::FlightControl(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {

  m_OpenFeintTimeout = 0.0;

  m_LastTouchX = 0.0;
	m_LastTouchY = 0.0;

  m_CameraX = 0.0;
  m_CameraY = 0.0;
  m_CameraZ = 0.0;

  m_CameraR = 180.0;

  m_PadCenters[0][0] = 90;
  m_PadCenters[0][1] = 0;
  m_PadCenters[1][0] = -90;
  m_PadCenters[1][1] = 0;

	m_Space = new Octree<int>(16 * 16, -63);
	m_Gravity = 600.0;

	int m_PostProcessFlags = aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;
	char path[128];
	
	snprintf(path, sizeof(s), "%d", 0);
	m_Importer.ReadFile(path, m_PostProcessFlags);
	LOGV("%s\n", m_Importer.GetErrorString());
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();	

	snprintf(path, sizeof(s), "%d", 3);
	m_Importer.ReadFile(path, m_PostProcessFlags);
	LOGV("%s\n", m_Importer.GetErrorString());
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 4));
	m_Importer.FreeScene();	

	m_Models.push_back(new Model(m_FooFoos.at(1)));
	m_Models[0]->SetTexture(m_Textures->at(3));
	m_Models[0]->SetFrame(0);
	m_Models[0]->SetPosition(0.0, 0.0, 0.0);
	m_Models[0]->SetScale(0.1, 0.1, 0.1);
  m_Models[0]->m_IsAlive = false;

/*
	m_Models.push_back(new Model(m_FooFoos.at(0)));
	m_Models[1]->SetTexture(m_Textures->at(2));
	m_Models[1]->SetFrame(0);
	m_Models[1]->SetPosition(5.0, 5.0, 5.0);
	m_Models[1]->SetScale(1.0, 1.0, 1.0);
*/

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, 64.0, 64.0));
	m_AtlasSprites[0]->SetPosition(m_PadCenters[0][0], m_PadCenters[0][1]);
	m_AtlasSprites[0]->Build(0);

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 1, 1, 0, 1, 1.0, "", 0, 0, 0.0, 64.0, 64.0));
	m_AtlasSprites[1]->SetPosition(m_PadCenters[1][0], m_PadCenters[1][1]);
	m_AtlasSprites[1]->Build(0);

  m_LastPadTouched = -1;
}


FlightControl::~FlightControl() {
}


void FlightControl::Hit(float x, float y, int hitState) {
	float xx = (x - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - y) * m_Zoom;
  //LOGV("hit %f %f %f %f\n", x, y, xx, yy);
  if (yy > 1) {
    if (hitState == 0) {
      PushMessageToWebView(CreateWebViewFunction("show()"));
    } else if (hitState == 2) {
      PushMessageToWebView(CreateWebViewFunction("hide()"));
    }
  } else {
    if (hitState == 2 && m_LastPadTouched >= 0) {
      m_AtlasSprites[m_LastPadTouched]->SetPosition(m_PadCenters[m_LastPadTouched][0], m_PadCenters[m_LastPadTouched][1]);
      m_LastPadTouched = -1;
    } else if (hitState == 1 && m_LastPadTouched >= 0) {
      m_LastTouchX = xx;
      m_LastTouchY = yy;
    } else {
      m_LastTouchX = xx;
      m_LastTouchY = yy;
      //determine moving pad
      if (xx > 0) { //right side
        m_LastPadTouched = 0;
      } else {//left side
        m_LastPadTouched = 1;
      }
    }
  }
}


void FlightControl::Build() {
}


int FlightControl::Simulate() {

  m_OpenFeintTimeout += m_DeltaTime;

  if (m_OpenFeintTimeout > 0.1) {
    m_OpenFeintTimeout = 0.0;
    const char *foo = PopMessageFromWebView();
  }

  m_Models[0]->Simulate(m_DeltaTime);

  //bool was_falling_before = ((m_LastCollideIndex < 0) || (m_CollideTimeout < 0.1));
	//float collide_x = ((m_AtlasSprites[0]->m_Position[0]) + (SUBDIVIDE * 0.5));
	//float collide_y = ((m_AtlasSprites[0]->m_Position[1]) + (SUBDIVIDE * 0.5));
  //int collide_index = m_Space->at((collide_x / SUBDIVIDE), (collide_y / SUBDIVIDE), 0);
  //LOGV("%f\n", RadiansToDegrees(player_theta));

  float vf = 0.0;
  float vu = 0.0;
  float vs = 0.0;
  float sc = 0.105;

  if (m_LastPadTouched >= 0) {
    float dx = m_PadCenters[m_LastPadTouched][0] - m_LastTouchX;
    float dy = m_PadCenters[m_LastPadTouched][1] - m_LastTouchY;
    float player_theta = atan2f(dy, dx);
    float deg = RadiansToDegrees(player_theta);
    float len = sqrt(dx * dx + dy * dy);
    m_AtlasSprites[m_LastPadTouched]->SetPosition(m_LastTouchX, m_LastTouchY);
    if (len > 1.0) {
      if (deg < -60 && deg > -120) {
        //LOGV("up\n");
        if (m_LastPadTouched == 0) {
          vf = sc * len;
        } else {
          vu = sc * len;
        }
      } else if (deg > 60 && deg < 120) {
        //LOGV("down\n");
        if (m_LastPadTouched == 0) {
          vf = -sc * len;
        } else {
          vu = -sc * len;
        }
      } else if (deg < 30 && deg > -30) {
        //LOGV("left\n");
        if (m_LastPadTouched == 0) {
          vs = -sc * len;
        } else {
          m_CameraR -= sc * len * 0.9 * m_DeltaTime;
        }
      } else if (deg < -150 || deg > 150) {
        //LOGV("right\n");
        if (m_LastPadTouched == 0) {
          vs = sc * len;
        } else {
          m_CameraR += sc * len * 0.9 * m_DeltaTime;
        }
      }
    }
  }

  float tx = -sin(DEGREES_TO_RADIANS(m_CameraR));
  float tz = cos(DEGREES_TO_RADIANS(m_CameraR));

  float txx = -sin(DEGREES_TO_RADIANS(m_CameraR + 90.0));
  float tzz = cos(DEGREES_TO_RADIANS(m_CameraR + 90.0));

  m_CameraX += (tx * vf * m_DeltaTime) + (txx * vs * m_DeltaTime);
  //m_CameraX += m_DeltaTime;
  //m_CameraX = 0;
  m_CameraY += (vu * m_DeltaTime);
  m_CameraZ += (tz * vf * m_DeltaTime) + (tzz * vs * m_DeltaTime);

  m_CameraTarget[0] = m_CameraX + (tx * 100.0);
  m_CameraTarget[1] = m_CameraY + fastSinf(m_SimulationTime * 10.0) * 1.0;
  m_CameraTarget[2] = m_CameraZ + (tz * 100.0);
  m_CameraPosition[0] = m_CameraX;
  m_CameraPosition[1] = m_CameraY; // + fastSinf(m_SimulationTime * 10.0) * 1.0;
  m_CameraPosition[2] = m_CameraZ;

  m_Models[0]->m_Rotation[1] += m_DeltaTime * 100.0;
  //LOGV("%f\n", m_CameraX);

/*
  m_CameraTarget[0] = 0.0;
  m_CameraTarget[1] = 0.0; 
  m_CameraTarget[2] = 0.0;
  m_CameraPosition[0] = 5.0;
  m_CameraPosition[1] = 5.0;
  m_CameraPosition[2] = 5.0;
*/

	return 1;
}


void FlightControl::RenderModelPhase() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_LIGHTING);
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, globalAmbient );
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuseLamp  );
	glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbientLamp  );
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDiffuseLamp  );
	glLightfv(GL_LIGHT0, GL_POSITION, lightPositionLamp );

	RenderModelRange(0, 1);

  glDisable(GL_BLEND);
  glDisable(GL_LIGHTING);
}


void FlightControl::RenderSpritePhase() {
  AtlasSprite::Scrub();
	RenderSpriteRange(0, 2);
}
