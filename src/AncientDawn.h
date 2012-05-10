// Jon Bardin GPL


enum {
  CONTINUE_LEVEL,
  RESTART_LEVEL,
  START_NEXT_LEVEL
};


class AncientDawn : public Engine {

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
  // * For every sprite
  // * * Recycle Bullet if lived past lifetime
  // * * Update physical position
  // * For every collision, Detect if between bullet and player
  // * * If bullet and player collide, cause damage to player
  // * Change level based on current level progress
	int Simulate();
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

  // Draw foos, really need a better name than foofoo
  foofoo *m_PlayerDraw;
  foofoo *m_SpaceShipDraw;
  foofoo *m_BulletDraw;
  foofoo *m_LandscapeDraw;
  foofoo *m_FirstBatch;
  foofoo *m_SecondBatch;
  foofoo *m_ThirdBatch;

  //
  int m_CurrentLevel;

  b2World *m_World;
  GLESDebugDraw *m_DebugDraw;
  SpaceShipDownContactListener *m_ContactListener;

  b2Body *m_PlayerBody;
  int m_PlayerIndex;

  int m_LandscapeIndex;

  int m_SpaceShipsStartIndex;
  int m_SpaceShipsStopIndex;

  // input mechanics
  bool m_TouchedLeft;
  bool m_TouchedRight;
  bool m_DebugDrawToggle;


};
