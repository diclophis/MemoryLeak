// Jon Bardin GPL


#include "MemoryLeak.h"
#include "MainMenu.h"


MainMenu::MainMenu(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {
  LOGV("main menu alloc\n");

  LoadTexture(0);
  LoadSound(1);
  CreateFoos();

  m_AtlasSprites.push_back(new SpriteGun(m_NinePatchFoo, NULL));
  m_AtlasSprites[m_SpriteCount]->m_IsNinePatch = true;
  m_AtlasSprites[m_SpriteCount]->SetPosition(0, 0);
  m_AtlasSprites[m_SpriteCount]->SetScale(128, 128);
  m_AtlasSprites[m_SpriteCount]->Build(0);
  m_SpriteCount++;

  m_SwapTimeout = 0;
}


MainMenu::~MainMenu() {
  LOGV("main menu dealloc\n");
  DestroyFoos();
}


void MainMenu::CreateFoos() {
  m_NinePatchFoo = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 254, 255, 0.0);
  m_BatchFoo = AtlasSprite::GetBatchFoo(m_Textures.at(0), 9);
}


void MainMenu::DestroyFoos() {
  delete m_NinePatchFoo;
  delete m_BatchFoo;
}


void MainMenu::Hit(float x, float y, int hitState) {
}


int MainMenu::Simulate() {
  //if ((m_SwapTimeout += m_DeltaTime) > 3.0) {
  //  m_AtlasSprites[0]->m_IsNinePatch = !m_AtlasSprites[0]->m_IsNinePatch;
  //  m_SwapTimeout = 0;
  //}
  m_AtlasSprites[0]->m_Rotation += 0.1 * m_DeltaTime;
  return 1;
}


void MainMenu::RenderModelPhase() {
}


void MainMenu::RenderSpritePhase() {
  RenderSpriteRange(0, m_SpriteCount, m_BatchFoo);
  AtlasSprite::RenderFoo(m_StateFoo, m_BatchFoo);
}
