// Jon Bardin GPL


enum {
  CONTINUE_LEVEL,
  RESTART_LEVEL,
  START_NEXT_LEVEL,
  END_LEVEL
};

enum {
  COLLIDE_PLAYER,
  COLLIDE_ENEMY,
  COLLIDE_CULLING
};

enum EBulletMLFileIndex {
    EBulletMLFileIndex_ENEMY = 0,
    EBulletMLFileIndex_PLAYER,
    EBulletMLFileIndex_COUNT,
};

//TODO names
enum EArmorType {
    EArmorType_1 = 0,
    EArmorType_2,
    EArmorType_3,
};

class AncientDawn : public Engine, b2QueryCallback {

public:

  // In the constructor, load these things
  //  * Textures
  //  * Sounds
  // Then start first level
	AncientDawn(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s);

  // Create a foo for these things
  // * Player
  // * Spaceship
  // * Bullet
  // * Terrain
  // * And 3 batch foos
  void CreateFoos();

  // Destroy all of the Foo created earlier
  void DestroyFoos();

  // Stop the level, and destroy textures sounds levels
	~AncientDawn();

  // Track the player location using hitState=1
	void Hit(float x, float y, int hitState);

  // Relate a b2Body to a AtlasSprite
  AtlasSprite *SpriteForBody(b2Body *body);

  // Create a physical body with these attributes
  b2Body *CreateBody();

  // Every tick update these things
  // * Step physics
  // * For every physical object
  // * * Recycle Bullet if lived past lifetime
  // * * Update physical position of sprite
  // * For every collision, Detect if between bullet and player
  // * * If bullet and player collide, cause damage to player
  // * Change level based on current level progress
  int Simulate();
private:
  int _gameSimulate();
public:
  void StepPhysics();
  void UpdatePhysicialPositionOfSprite(AtlasSprite *sprite, float x, float y);
  b2Body *BodyCollidingWithPlayer(b2Body *a, b2Body *b);

  // Level progression
  int LevelProgress();
  void RestartLevel();
  void StartNextLevel();
  int FirstLevel();
  int NextLevel();

  // Starting a level should do these things:
  // * Reset the State Foo
  // * Reset the game parameters
  // * Create foos
  // * Create the physical world
  // * * Create a debug drawer for the world
  // * * Create a Contact Listener for the world
  // * Create the Player
  // * Create the spaceships
  // * Create the Landscape
  void StartLevel(int level_index);
  void ResetGame();
  void CreateWorld();
  void CreateDebugDraw();
  void CreateContactListener();
  void CreatePlayer();
  void CreateSpaceShip();
  void CreateLandscape();

  // When stopping a level do
  // * Destroy foos
  // * Destroy physical world
  // * * Destroy debug drawer
  // * * Destroy Contact Listener
  // * Destroy the player
  // * Destroy the spaceships
  // * Destroy the Landscape
  void StopLevel();
  void DestroyWorld();
  void DestroyDebugDraw();
  void DestroyContactListener();
  void DestroyPlayer();
  void DestroySpaceShip();
  void DestroyLandscape();

  // Render callbacks
	void RenderModelPhase();
	void RenderSpritePhase();


  foofoo *m_PlayerDraw;
  foofoo *m_SpaceShipDraw;
  foofoo *m_BulletDraw;
  foofoo *m_SpaceShipBulletDraw;
  foofoo *m_LandscapeDraw;
  foofoo *m_FirstBatch;
  foofoo *m_SecondBatch;
  foofoo *m_ThirdBatch;

  // level
  int m_CurrentLevel;

  // physics
  BulletHellWorld *m_World;
  GLESDebugDraw *m_DebugDraw;
  b2Body *m_PlayerBody;
  b2Body *m_EnemyBody;
  b2MouseJoint *m_PlayerMouseJoint;

  // drawing
  int m_PlayerIndex;
  int m_SpaceShipIndex;
  int m_LandscapeIndex;
  int m_SpaceShipsStartIndex;
  int m_SpaceShipsStopIndex;
  int m_LastRecycledIndex;

  // input mechanics
  bool m_TouchedLeft;
  bool m_TouchedRight;
  bool m_DebugDrawToggle;
  float m_TouchOffsetX;
  float m_TouchOffsetY;

  // collision detection
  int m_ColliderSwitch;
  float m_SolveTimeout;
  float m_BossShootTimeout;
  float m_ShootTimeout;
  float m_PhysicsTimeout;
  float m_BulletSpeed;
  int m_Batch;
  bool m_Force;
  b2AABB aabb;
  bool ReportFixture(b2Fixture* fixture);
  
  // Game State logic
  bool mbGameStarted;
  
  // Player Logic
  float m_PlayerHealth;
  int m_PlayerArmor;
  bool mbPlayerIsShooting;

  BulletCommand* bc;
  BulletCommand* mpBulletCommandPlayer;
  
  float m_WebViewTimeout;
  
  std::string m_JavascriptTick;
  
  int m_LastBulletCommandTurn;

};
