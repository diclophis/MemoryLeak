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
}

MainMenu::~MainMenu() {
}

void MainMenu::Hit(float x, float y, int hitState) {
	m_IsPushingAudio = !m_IsPushingAudio;
}

void MainMenu::Build() {
  m_IsPushingAudio = true;
}

int MainMenu::Simulate() {
	return 1;
}

void MainMenu::Render() {
}

