// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SpaceShipDownContactListener.h"
#include "AncientDawn.h"


AncientDawn::AncientDawn(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : Engine(w, h, t, m, l, s) {
  LOGV("alloc AncientDawn %d %d %d\n", CONTINUE_LEVEL, RESTART_LEVEL, START_NEXT_LEVEL);
  LoadSound(0);
  LoadTexture(0);
  StartLevel(FirstLevel());
}


AncientDawn::~AncientDawn() {
  LOGV("dealloc AncientDawn\n");
  StopLevel();
}


void AncientDawn::CreateFoos() {
  m_PlayerDraw = AtlasSprite::GetFoo(m_Textures.at(0), 16, 16, 128, 132, 0.0);
  m_SpaceShipDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
  m_BulletDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
  m_LandscapeDraw = AtlasSprite::GetFoo(m_Textures.at(0), 1, 1, 0, 1, 0.0);
  m_FirstBatch = AtlasSprite::GetBatchFoo(m_Textures.at(0), 1);
  m_SecondBatch = AtlasSprite::GetBatchFoo(m_Textures.at(0), 1);
  m_ThirdBatch = AtlasSprite::GetBatchFoo(m_Textures.at(0), 1);
}


void AncientDawn::DestroyFoos() {
  delete m_PlayerDraw;
  delete m_SpaceShipDraw;
  delete m_BulletDraw;
  delete m_LandscapeDraw;
  delete m_FirstBatch;
  delete m_SecondBatch;
  delete m_ThirdBatch;
}


void AncientDawn::StartLevel(int level_index) {
  LOGV("Starting Level: %d\n", level_index);

  m_CurrentLevel = level_index;

  ResetStateFoo();
  ResetGame();
  CreateFoos();
  CreateWorld();
  CreateDebugDraw();
  CreateContactListener();
  CreatePlayer();
  CreateSpaceShip();
  CreateLandscape();
}


void AncientDawn::ResetGame() {
  m_SimulationTime = 0.0;
  m_DebugDrawToggle = false;
  m_TouchedLeft = false;
  m_TouchedRight = false;
  m_CurrentSound = 0;
  m_PlayerIndex = 0;
}


void AncientDawn::CreateWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0, 0.0);
  m_World = new b2World(gravity, false);
}


void AncientDawn::DestroyWorld() {
  delete m_World;
}


void AncientDawn::CreateDebugDraw() {
  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  flags += b2Draw::e_jointBit;
  flags += b2Draw::e_aabbBit;
  flags += b2Draw::e_pairBit;
  flags += b2Draw::e_centerOfMassBit;
  m_DebugDraw = new GLESDebugDraw(PTM_RATIO);
  m_World->SetDebugDraw(m_DebugDraw);
  m_DebugDraw->SetFlags(flags);
}


void AncientDawn::DestroyDebugDraw() {
  delete m_DebugDraw;
}


void AncientDawn::CreateContactListener() {
  m_ContactListener = new SpaceShipDownContactListener();
  m_World->SetContactListener(m_ContactListener);
}


void AncientDawn::DestroyContactListener() {
  delete m_ContactListener;
}


void AncientDawn::StopLevel() {
  DestroyFoos();
  ClearSprites();
  DestroyWorld();
  DestroyDebugDraw();
  DestroyContactListener();
  DestroyPlayer();
  DestroySpaceShip();
  DestroyLandscape();
}


void AncientDawn::CreatePlayer() {
  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_PlayerDraw, NULL));
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 10;
  m_AtlasSprites[m_PlayerIndex]->m_IsAlive = 30;
  m_AtlasSprites[m_PlayerIndex]->SetPosition(0.0, 0.0);
  m_AtlasSprites[m_PlayerIndex]->SetScale(16.0, 16.0);
  m_AtlasSprites[m_PlayerIndex]->Build(0);
  m_SpriteCount++;
}


void AncientDawn::DestroyPlayer() {
}


void AncientDawn::CreateSpaceShip() {
}


void AncientDawn::DestroySpaceShip() {
}


void AncientDawn::CreateLandscape() {
}


void AncientDawn::DestroyLandscape() {
}


int AncientDawn::LevelProgress() {
  if (m_SimulationTime > 5.0) {
    return RESTART_LEVEL;
  }

  return CONTINUE_LEVEL;
}


void AncientDawn::RestartLevel() {
  StopLevel();
  StartLevel(m_CurrentLevel);
}


void AncientDawn::StartNextLevel() {
  StopLevel();
  StartLevel(NextLevel());
}


int AncientDawn::NextLevel() {
  return 0;
}


int AncientDawn::FirstLevel() {
  return 0;
}


void AncientDawn::Hit(float x, float y, int hitState) {
}


int AncientDawn::Simulate() {

  m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);

  switch(LevelProgress()) {
    case CONTINUE_LEVEL:
      return 1;

    case RESTART_LEVEL:
      RestartLevel();
      return 1;

    case START_NEXT_LEVEL:
      StartNextLevel();
      return 1;
  }
}


void AncientDawn::RenderModelPhase() {
}


void AncientDawn::RenderSpritePhase() {
  RenderSpriteRange(0, m_SpriteCount, m_FirstBatch);
  AtlasSprite::RenderFoo(m_StateFoo, m_FirstBatch);
}
