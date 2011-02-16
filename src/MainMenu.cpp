// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "MainMenu.h"

MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	
	
	
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
	m_Models[1]->SetScale(1024.0, 0.25, 1024.0);

}

MainMenu::~MainMenu() {
}

void MainMenu::Hit(float x, float y, int hitState) {
	m_Models[0]->m_Fps += 1.0;
	//m_IsPushingAudio = !m_IsPushingAudio;
}

void MainMenu::Build() {
  m_IsPushingAudio = true;
}

int MainMenu::Simulate() {
	
	m_Models[0]->Simulate(m_DeltaTime, false);

	if (m_IsPushingAudio) {
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
	} else {
		m_CameraTarget[0] = 0.0;
		m_CameraTarget[1] = 0.0;
		m_CameraTarget[2] = -1.0;
		
		m_CameraPosition[0] = m_Models[0]->m_Position[0];
		m_CameraPosition[1] = m_Models[0]->m_Position[1] + 0.25;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] + 0.5;
	}
	
	return 1;
}

void MainMenu::Render() {
	
}

