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

#define kMaxTankSpeed 30.0
#define kTurnRate 1.0
#define kTankAcceleration 0.5
#define kPlayerHeight 0.0

/* Global ambient light. */
static const GLfloat globalAmbient[4]      = { 0.8, 0.8, 0.8, 1.0 };

/* Lamp parameters. */
static const GLfloat lightDiffuseLamp[4]   = { 1.0, 1.0, 1.0, 1.0 };
static const GLfloat lightAmbientLamp[4]   = { 0.4, 0.4, 0.4, 1.0 };
static const GLfloat lightPositionLamp[4]  = { 25.0, 25.0, 25.0, 25.0 };

#ifdef DESKTOP
  #define glClipPlanef glClipPlane
#endif

MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_CameraIndex = 2;
	
	leftSliderValue = 0.0;
	rightSliderValue = 0.0;
	
	m_CameraRotation = -10.0;
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
	
	snprintf(path, sizeof(s), "%d", 2);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();	
	
	snprintf(path, sizeof(s), "%d", 1);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();
	
	snprintf(path, sizeof(s), "%d", 3);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();
	
	int model = 5;
	int texture = 5;
	int level_index = 1;

	snprintf(path, sizeof(s), "%d", model);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();
	
	m_Models.push_back(new Model(m_FooFoos.at(0)));
	m_Models[0]->SetTexture(m_Textures->at(0));
	m_Models[0]->SetFrame(0);
	m_Models[0]->SetPosition(50.0, kPlayerHeight, 80.0);
	m_Models[0]->SetScale(4.0, 4.0, 4.0);

	m_Models.push_back(new Model(m_FooFoos.at(2)));
	m_Models[1]->SetTexture(m_Textures->at(1));
	m_Models[1]->SetFrame(0);
	m_Models[1]->SetPosition(128.0, 0.0, 128.0);
	m_Models[1]->SetScale(256.0, 1.0, 256.0);
	
	m_Models.push_back(new Model(m_FooFoos.at(3)));
	m_Models[2]->SetTexture(m_Textures->at(texture));
	m_Models[2]->SetFrame(0);
	m_Models[2]->SetPosition(128.0, 0.0, 128.0);
	m_Models[2]->SetScale(256.0, 128.0, 256.0);
	m_Models[2]->m_Rotation[1] = 90.0;

	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 4, 4, 0, 16, 1.25, "", 0, 16, 0.25));
	m_AtlasSprites[0]->SetPosition(0.0 - (0.4 * m_ScreenWidth), 0.0);
	m_AtlasSprites[0]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[0]->m_IsAlive = true;
	m_AtlasSprites[0]->m_IsReady = true;
	m_AtlasSprites[0]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[0]->Build(10);
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 4, 4, 0, 16, 1.25, "", 0, 16, 2.25));
	m_AtlasSprites[1]->SetPosition(0.0 + (0.4 * m_ScreenWidth), 0.0);
	m_AtlasSprites[1]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[1]->m_IsAlive = false;
	m_AtlasSprites[1]->m_IsReady = true;
	m_AtlasSprites[1]->Build(0);
	
	BuildParticles(100);
	
	
	unsigned char *level = (unsigned char *)malloc(sizeof(unsigned char) * m_LevelFoos->at(level_index)->len);
	
	fseek(m_LevelFoos->at(level_index)->fp, m_LevelFoos->at(level_index)->off, SEEK_SET);
	fread(level, sizeof(unsigned char), m_LevelFoos->at(level_index)->len, m_LevelFoos->at(level_index)->fp);
	
	
	m_Space = new Octree<int>(256 * 256, -1);
	
	int x = 0;
	int y = 0;
	int f = m_NumParticles + 3;
	for (unsigned int i=m_LevelFoos->at(level_index)->len - 1; i>0; i--) {
		float yy = ((float)level[i] / 255.0) * 1.0;
		m_Space->set(x, 0, y, f);
		m_Models.push_back(new Model(m_FooFoos.at(2)));
		m_Models[f]->SetTexture(m_Textures->at(1));
		m_Models[f]->SetFrame(0);
		m_Models[f]->SetPosition(x, 3.0, y);
		m_Models[f]->SetScale(yy, 1.0, yy);
		m_Models[f]->m_Life = yy * 2.5;
		if (level[i] < 64) {
			m_Models[f]->m_IsHelpfulToPlayers = true;
		}
		x++;
		if (x == 256) {
			x = 0;
			y++;
		}
		f++;
	}
	
	free(level);
}

MainMenu::~MainMenu() {
}

void MainMenu::Hit(float x, float y, int hitState) {
	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;

	float dpx;
	float dpy;
	
	if (y < (0.15 * m_ScreenHeight)) {
		if (hitState == 2) {
			m_CameraIndex++;
			if (m_CameraIndex > 3) {
				m_CameraIndex = 0;
			}
		}
	} else {
		if (xx < 0) {
			dpx = m_AtlasSprites[0]->m_Position[0] - xx;
			dpy = m_AtlasSprites[0]->m_Position[1] - yy;
			xx = m_AtlasSprites[0]->m_Position[0] = -
			(0.4 * m_ScreenWidth);
			yy = m_AtlasSprites[0]->m_Position[1] - dpy;
			m_AtlasSprites[0]->SetPosition(xx, yy);
			rightSliderValue = yy;
		} else {
			dpx = m_AtlasSprites[1]->m_Position[0] - xx;
			dpy = m_AtlasSprites[1]->m_Position[1] - yy;
			xx = m_AtlasSprites[1]->m_Position[0] = (0.4 * m_ScreenWidth);
			yy = m_AtlasSprites[1]->m_Position[1] - dpy;
			m_AtlasSprites[1]->SetPosition(xx, yy);
			leftSliderValue = yy;
		}
	}
}

void MainMenu::Build() {
  m_IsPushingAudio = true;
}

int MainMenu::Simulate() {
	m_AtlasSprites[0]->Simulate(m_DeltaTime);

	int shot_this_tick = 0;
	int not_shot_this_tick = 0;
	float m_ShotMaxLife = 0.5;
	
	float g = 200.0;
	float x = 0.0;
	float y = 0.0;
	float a = 0.0;
	
	bool moveForward = false;
	bool moveBackward = false;
	bool turnLeft = false;
	bool turnRight = false;
	float sliderDelta;
	
	int collided_index = m_Space->at(m_Models[0]->m_Position[0], 0, m_Models[0]->m_Position[2]);
	if (collided_index > 0) {
		if (m_Models[collided_index]->m_IsHelpfulToPlayers) {
			m_Models[0]->m_Position[1] = kPlayerHeight;
			a = kTankAcceleration;
			moveForward = true;
		} else {
			m_Models[0]->m_Position[1] = kPlayerHeight + (m_Models[collided_index]->m_Life);
			m_Models[0]->m_Velocity[0] = 10.0;
			moveForward = true;
		}
	}
	
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		int o = m_ParticlesOffset + idx;

		if ((shot_this_tick < 12) && ((m_Models[o]->m_Life > m_ShotMaxLife) || !m_Models[o]->m_IsAlive)) {
			ShootParticle(o);
			shot_this_tick++;
		} else {
			not_shot_this_tick++;
			if ((m_Models[o]->m_Life > m_ShotMaxLife)) {
				ResetParticle(o);
			}
		}

		float theta = m_Models[o]->m_Theta;
		float v = m_Models[o]->m_Velocity[0];
		m_Models[o]->m_Life += m_DeltaTime;
		
		x = v * cos(theta) * m_Models[o]->m_Life;
		y = (v * fastSinf(theta) * m_Models[o]->m_Life) - (0.5 * g * (m_Models[o]->m_Life * m_Models[o]->m_Life));
		
		float tx = -sin(DEGREES_TO_RADIANS(m_Models[o]->m_Rotation[1]));
		float tz = cos(DEGREES_TO_RADIANS(m_Models[o]->m_Rotation[1]));
		
		m_Models[o]->m_Position[0] = (x * -(tx)) + m_Models[0]->m_Position[0] - (tx * 2.0);
		m_Models[o]->m_Position[1] = y;
		m_Models[o]->m_Position[2] = (x * -(tz)) + m_Models[0]->m_Position[2] - (tz * 2.0);
		
	}

	sliderDelta = leftSliderValue - rightSliderValue;
	
	float absDelta = fastAbs(sliderDelta);
		
	if (absDelta > 20.0) {
		if (sliderDelta < 0.0) {
			turnLeft = true;
		} else {
			turnRight = true;
		}
	}
	
	/*
	//TODO: figure this out
	if (rightSliderValue > 0.95 && leftSliderValue > 0.95) {
		moveForward = true;
	} else if (rightSliderValue < 0.04 && leftSliderValue < 0.04) {
		moveBackward = true;
	}
	
	//moveForward = true;
	*/
	
	if (turnLeft) {
		if (m_Models[0]->m_Rotation[0] > 30) {
			m_Models[0]->m_Rotation[0] = 30;
		} else {
			m_Models[0]->m_Rotation[0] += 1.0;
		}
		m_Models[0]->m_Rotation[1] += kTurnRate * absDelta * m_DeltaTime;
	} else if (turnRight) {
		if (m_Models[0]->m_Rotation[0] < -30) {
			m_Models[0]->m_Rotation[0] = -30;
		} else {
			m_Models[0]->m_Rotation[0] -= 1.0;
		}
		m_Models[0]->m_Rotation[1] -= kTurnRate * absDelta * m_DeltaTime;
	} else {
		if (m_Models[0]->m_Rotation[0] > 0) {
			m_Models[0]->m_Rotation[0] -= 1.0;
		} else {
			m_Models[0]->m_Rotation[0] += 1.0;
		}
	}
	
	if (moveForward) {
		m_Models[0]->m_Velocity[0] += a;
		if (m_Models[0]->m_Velocity[0] > kMaxTankSpeed) {
			m_Models[0]->m_Velocity[0] = kMaxTankSpeed;
		}
	} else if (moveBackward) {
		m_Models[0]->m_Velocity[0] -= a;
		if (m_Models[0]->m_Velocity[0] < -kMaxTankSpeed) {
			m_Models[0]->m_Velocity[0] = -kMaxTankSpeed;
		}
	}
	
	m_Models[0]->m_Life += m_DeltaTime;
	m_Models[0]->Simulate(m_DeltaTime, false);
	
	m_Models[0]->m_Rotation[2] = 0.0; //fastSinf(m_Life * 2.0) * 5.0;

	if (m_CameraIndex == 0) {
		m_CameraTarget[0] = m_Models[0]->m_Position[0];
		m_CameraTarget[1] = m_Models[0]->m_Position[1];
		m_CameraTarget[2] = m_Models[0]->m_Position[2];

		m_CameraRotation += DEGREES_TO_RADIANS(0.5);
		m_CameraHeight = 1.25; // + (fastSinf(m_SimulationTime * 0.5) * 5.0);
		float m_CameraDiameter = 20.0; // + fastAbs(fastSinf(m_SimulationTime * 0.1) * 25.0);
		float cx = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
		float cz = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];
		
		m_CameraPosition[0] = cx;
		m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
		m_CameraPosition[2] = cz;
	} else if (m_CameraIndex == 1) {
		float tx = -sin(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1]));
		float tz = cos(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1]));
		
		float txx = -sin(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1] + 90.0));
		float tzz = cos(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1] + 90.0));
		
		/*
		m_CameraTarget[0] = m_Models[0]->m_Position[0] + (tx * 60.0);
		m_CameraTarget[1] = 0.0125;
		m_CameraTarget[2] = m_Models[0]->m_Position[2] + (tz * 60.0);
		m_CameraPosition[0] = m_Models[0]->m_Position[0] - (tx * 0.25) - (txx * 0.4);
		m_CameraPosition[1] = 0.125;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] - (tz * 0.25) - (tzz * 0.4);
		*/
		
		m_CameraTarget[0] = m_Models[0]->m_Position[0] + (tx * 60.0);
		m_CameraTarget[1] = 0.1;
		m_CameraTarget[2] = m_Models[0]->m_Position[2] + (tz * 60.0);
		m_CameraPosition[0] = m_Models[0]->m_Position[0] - (tx * 10.0) - (txx * 0.0);
		m_CameraPosition[1] = 1.0;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] - (tz * 10.0) - (tzz * 0.0);
	} else if (m_CameraIndex == 2) {
		m_CameraTarget[0] = 128.0;
		m_CameraTarget[1] = 0.0;
		m_CameraTarget[2] = 128.0;
		m_CameraPosition[0] = 0.0;
		m_CameraPosition[1] = 512.0;
		m_CameraPosition[2] = 0.0;
	} else if (m_CameraIndex == 3) {
		m_CameraTarget[0] = m_Models[0]->m_Position[0];
		m_CameraTarget[1] = m_Models[0]->m_Position[1];
		m_CameraTarget[2] = m_Models[0]->m_Position[2];
		m_CameraPosition[0] = m_Models[0]->m_Position[0] + 1;
		m_CameraPosition[1] = m_Models[0]->m_Position[1] + 60.0;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] + 1;
	}
	
	return 1;
}

void MainMenu::RenderModelPhase() {
	
	glEnable(GL_LIGHTING);
	
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, globalAmbient );
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,  lightDiffuseLamp  );
	glLightfv(GL_LIGHT0, GL_AMBIENT,  lightAmbientLamp  );
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDiffuseLamp  );
	glLightfv(GL_LIGHT0, GL_POSITION, lightPositionLamp );
	
	//float old_scale = m_Models[0]->m_Scale[1];
	//m_Models[0]->m_Scale[1] = old_scale + (fastSinf(m_SimulationTime * 1.0) * 0.2);
	
	glFrontFace(GL_CW);
	DrawPlayer(-1.0);
	glFrontFace(GL_CCW);

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	//RenderModelRange(1, 2);
	glDisable(GL_BLEND);

	//m_Models[0]->m_Scale[1] = old_scale;
	
	if (m_CameraIndex == 2) {
		RenderModelRange(3 + m_NumParticles, 256 * 256);
	} else {
		RenderModelRange(2, 3);
		DrawPlayer(1.0);
	}
	
	glDisable(GL_LIGHTING);
}


void MainMenu::DrawPlayer(float yScale) {
	// Store the model view matrix
	glPushMatrix();
	
	// Reflection and the light position must be mirrored
	glScalef(1.f, yScale, 1.f);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPositionLamp);
	
    // Define the water clip plane that prevents the duck reflection
    // from being drawn above the water
#ifdef DESKTOP
	GLdouble coeff[4] =  { 0.f, 1.f, 0.f, 0.f };
#else
	GLfloat coeff[4] =  { 0.f, 1.f, 0.f, 0.f };
#endif

	glClipPlanef( GL_CLIP_PLANE0, coeff );
	glEnable( GL_CLIP_PLANE0 );
	
	RenderModelRange(0, 1);
	
	if (m_Models[0]->m_Velocity[0] > 25.0) {
		glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
		glBlendFunc(GL_ONE, GL_ONE);
		RenderModelRange(3, 3 + m_NumParticles);
		glDisable(GL_BLEND);
	}
	
	glDisable( GL_CLIP_PLANE0 );
	
	glPopMatrix();
}


void MainMenu::RenderSpritePhase() {
	RenderSpriteRange(0, 2);
}


void MainMenu::BuildParticles(int n) {
	m_NumParticles = n;
	m_ShootInterval = 1.0;
	m_ParticleStreamIndex = 0;
	m_ParticlesOffset = m_Models.size();
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		int o = m_ParticlesOffset + idx;  
		m_Models.push_back(new Model(m_FooFoos.at(1)));
		m_Models[o]->SetTexture(m_Textures->at(1));
		m_Models[o]->SetFrame(0);
		ResetParticle(o);
	}
}


void MainMenu::ResetParticles() {
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		ResetParticle(idx);
	}
}


void MainMenu::ResetParticle(int idx) {	
	m_Models[idx]->SetPosition(m_Models[0]->m_Position[0], m_Models[0]->m_Position[1], m_Models[0]->m_Position[2]);
	m_Models[idx]->m_Life = 0.0 - (randf() * 20);
	m_Models[idx]->SetScale(0.5, 0.5, 0.5);
	m_Models[idx]->m_Theta = DEGREES_TO_RADIANS(45);
	m_Models[idx]->m_Velocity[0] = (fastAbs(randf()) * 25.0) + 15.0;
	m_Models[idx]->m_IsAlive = false;
}


void MainMenu::ShootParticle(int idx) {
	m_Models[idx]->SetPosition(m_Models[0]->m_Position[0], m_Models[0]->m_Position[1], m_Models[0]->m_Position[2]);
	switch (m_ParticleStreamIndex++ % 3) {
		case 0:
			m_Models[idx]->m_Theta = DEGREES_TO_RADIANS((fastAbs(randf()) * 30.0) + 20.0);
			m_Models[idx]->m_Rotation[1] = (m_Models[0]->m_Rotation[1] + 25) + (randf() * 7.0);
			break;
		case 1:
			m_Models[idx]->m_Theta = DEGREES_TO_RADIANS((fastAbs(randf()) * 5.0) + 20.0);
			m_Models[idx]->m_Rotation[1] = (m_Models[0]->m_Rotation[1]) + (randf() * 2.0);
			break;
		case 2:
			m_Models[idx]->m_Theta = DEGREES_TO_RADIANS((fastAbs(randf()) * 30.0) + 20.0);
			m_Models[idx]->m_Rotation[1] = (m_Models[0]->m_Rotation[1] - 25) + (randf() * 7.0);
			break;
	};
	
	m_Models[idx]->m_Life = 0.0;
	m_Models[idx]->m_IsAlive = true;
}
