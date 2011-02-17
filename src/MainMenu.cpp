// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"

#include "MainMenu.h"

#define kMaxTankSpeed 50.0
#define kTurnRate 1.0
#define kTankAcceleration 0.05

MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	
	leftSliderValue = m_ScreenHeight * 0.5;
	rightSliderValue = m_ScreenHeight * 0.5;
	
	m_CameraRotation = -33.0;
	m_CameraRotationSpeed = 0.0;
	m_CameraHeight = 10.0;
	m_CameraClimbSpeed = 0.0;
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	m_CameraPosition[0] = 0.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;

	int m_PostProcessFlags = aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;
	char path[128];
	snprintf(path, sizeof(s), "%d", 0);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 2));
	m_Importer.FreeScene();	
	
	snprintf(path, sizeof(s), "%d", 1);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();	
	
	m_Models.push_back(new Model(m_FooFoos.at(0)));
	m_Models[0]->SetTexture(m_Textures->at(0));
	m_Models[0]->SetFrame(0);
	
	
	m_Models.push_back(new Model(m_FooFoos.at(1)));
	m_Models[1]->SetTexture(m_Textures->at(1));
	m_Models[1]->SetFrame(0);
	m_Models[1]->SetPosition(0.0, -0.675, 0.0);
	m_Models[1]->SetScale(512.0, 0.25, 512.0);

	
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 10, 10, "", 0, 60, 1.25, "", 0, 60, 2.25));
	m_AtlasSprites[0]->SetPosition(0.0 - (0.25 * m_ScreenWidth), 0.0);
	m_AtlasSprites[0]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[0]->m_IsAlive = false;
	m_AtlasSprites[0]->m_IsReady = true;
	m_AtlasSprites[0]->Build(10);
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 10, 10, "", 0, 60, 1.25, "", 0, 60, 2.25));
	m_AtlasSprites[1]->SetPosition(0.0 + (0.25 * m_ScreenWidth), 0.0);
	m_AtlasSprites[1]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[1]->m_IsAlive = false;
	m_AtlasSprites[1]->m_IsReady = true;
	m_AtlasSprites[1]->Build(10);
										
}

MainMenu::~MainMenu() {
}

void MainMenu::Hit(float x, float y, int hitState) {
	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;

	float dpx;
	float dpy;
	
	if (xx < 0) {
		dpx = m_AtlasSprites[0]->m_Position[0] - xx;
		dpy = m_AtlasSprites[0]->m_Position[1] - yy;
		xx = m_AtlasSprites[0]->m_Position[0];
		yy = m_AtlasSprites[0]->m_Position[1] - dpy;
		m_AtlasSprites[0]->SetPosition(xx, yy);
		rightSliderValue = yy;
	} else {
		dpx = m_AtlasSprites[1]->m_Position[0] - xx;
		dpy = m_AtlasSprites[1]->m_Position[1] - yy;
		xx = m_AtlasSprites[1]->m_Position[0];
		yy = m_AtlasSprites[1]->m_Position[1] - dpy;
		m_AtlasSprites[1]->SetPosition(xx, yy);
		leftSliderValue = yy;
	}
}

void MainMenu::Build() {
  m_IsPushingAudio = true;
}

int MainMenu::Simulate() {
	
	bool moveForward = false;
	bool moveBackward = false;
	bool turnLeft = false;
	bool turnRight = false;
	float sliderDelta;
	
	sliderDelta = leftSliderValue - rightSliderValue;
	
	float absDelta = fastAbs(sliderDelta);
		
	if (absDelta > 0.2) {
		if (sliderDelta < 0.0) {
			turnLeft = true;
		} else {
			turnRight = true;
		}
	}
	
	
	//TODO: figure this out
	if (rightSliderValue > 0.95 && leftSliderValue > 0.95) {
		moveForward = true;
	} else if (rightSliderValue < 0.04 && leftSliderValue < 0.04) {
		moveBackward = true;
	}
	
	moveForward = true;
	
	if (turnLeft) {
		m_Models[0]->m_Rotation[1] += kTurnRate * absDelta * m_DeltaTime;
	} else if (turnRight) {
		m_Models[0]->m_Rotation[1] -= kTurnRate * absDelta * m_DeltaTime;
	}
		
	if (moveForward) {
		m_Models[0]->m_Velocity[0] += kTankAcceleration;
		if (m_Models[0]->m_Velocity[0] > kMaxTankSpeed) {
			m_Models[0]->m_Velocity[0] = kMaxTankSpeed;
		}
	} else if (moveBackward) {
		m_Models[0]->m_Velocity[0] -= kTankAcceleration;
		if (m_Models[0]->m_Velocity[0] < -kMaxTankSpeed) {
			m_Models[0]->m_Velocity[0] = -kMaxTankSpeed;
		}
	}
	
	m_Models[0]->Simulate(m_DeltaTime, false);

	if (false) {
		m_CameraTarget[0] = m_Models[0]->m_Position[0];
		m_CameraTarget[1] = m_Models[0]->m_Position[1];
		m_CameraTarget[2] = m_Models[0]->m_Position[2];

		m_CameraRotation += DEGREES_TO_RADIANS(2.0);
		
		m_CameraHeight = 0.5; // + (fastSinf(m_SimulationTime * 0.5) * 5.0);
		float m_CameraDiameter = 5.0; // + fastAbs(fastSinf(m_SimulationTime * 0.1) * 25.0);
		float cx = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
		float cz = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];
		
		m_CameraPosition[0] = cx;
		m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
		m_CameraPosition[2] = cz;
	} else if (true) {
		float tx = -sin(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1]));
		float tz = cos(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1]));
		
		m_CameraTarget[0] = m_Models[0]->m_Position[0] - (tx * 200.0);
		m_CameraTarget[1] = 0.5;
		m_CameraTarget[2] = m_Models[0]->m_Position[2] - (tz * 200.0);
		
		m_CameraPosition[0] = m_Models[0]->m_Position[0] + (tx * 5.0);
		m_CameraPosition[1] = 1.25;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] + (tz * 5.0);
		
	} else if (false) {
		m_CameraTarget[0] = 0.0;
		m_CameraTarget[1] = 0.0;
		m_CameraTarget[2] = 0.0;
		
		m_CameraPosition[0] = 1.0;
		m_CameraPosition[1] = 100.0;
		m_CameraPosition[2] = 1.0;
	} else {
		m_CameraTarget[0] = m_Models[0]->m_Position[0];
		m_CameraTarget[1] = m_Models[0]->m_Position[1];
		m_CameraTarget[2] = m_Models[0]->m_Position[2];
		
		m_CameraPosition[0] = m_Models[0]->m_Position[0] + 1;
		m_CameraPosition[1] = m_Models[0]->m_Position[1] + 50.0;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] + 1;
	}
	
	return 1;
}

void MainMenu::Render() {
	
}

