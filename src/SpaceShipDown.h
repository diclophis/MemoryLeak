// Jon Bardin GPL

#include "SpaceShipDownContactListener.h"

typedef std::vector<OpenSteer::SphereObstacle*> SOG; // SphereObstacle group
typedef SOG::const_iterator SOI; // SphereObstacle iterator

class BaseVehicle : public OpenSteer::SimpleVehicle {
public:
  BaseVehicle();
  void reset (void);
  void identify();
  void randomizeStartingPositionAndHeading (void);
  enum seekerState {running, tagged, atGoal};
  bool avoiding;
  float minDistanceToObstacle (const OpenSteer::Vec3 point);
  static int obstacleCount;
  static SOG allObstacles;
};

class EnemyVehicle : public BaseVehicle {
public:
  EnemyVehicle();
  void update (const float currentTime, const float elapsedTime);
  void identify();
  void reset (void);
  OpenSteer::Vec3 steerToEvadeAllOtherEnemies (void);
};

class SpaceShipDown : public Engine {

public:

	SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s);
	~SpaceShipDown();
	void Hit(float x, float y, int hitState);
	int Simulate();
	void RenderModelPhase();
	void RenderSpritePhase();
  void CreateWorld();
  void CreateBorder(float width, float height);
  void CreatePlayer(float x, float y);
  void CreateSpaceShipPart(float x, float y);
  void CreatePlatform(float x, float y, float w, float h);
  void CreateDropZone(float x, float y, float w, float h);
  void CreatePickupJoints();
  void CreateDebugDraw();
  void AdjustZoom();
  const char *byte_to_binary(int x);
  void LoadLevel(int level_index, int cursor_index);
  void CreateLandscape();
  void CreateVehicles();
  void CreateEnemy();
  void StartLevel(int level_index);
  void StopLevel();
  void CreateFoos();
  void DestroyFoos();


  b2World *world;
  b2Body *m_PlayerBody;
  b2Body *m_GroundBody;
  b2Joint *m_PickupJoint;
  b2Body *m_SpaceShipBaseBody;
  std::vector<b2JointDef*> m_PickupJointDefs;
  b2FrictionJointDef *m_FrictionJointDef;
  b2FrictionJoint *m_FrictionJoint;
  std::vector<b2MouseJoint*> m_EnemyMouseJoints;
  b2PrismaticJointDef *m_PollJointDef;

  int m_LandscapeIndex;
  int m_PlayerIndex;
  int m_PlayerIndex2;
  int m_SpaceShipPartsStartIndex;
  int m_SpaceShipPartsStopIndex;
  int m_PlatformsStartIndex;
  int m_PlatformsStopIndex;
  int m_DropZonesStartIndex;
  int m_DropZonesStopIndex;
  int m_PickedUpPartIndex;
  int m_RequiredPartIndex;
  int m_EnemiesStartIndex;
  int m_EnemiesStopIndex;
  int m_StackCount;
  int m_LevelIndex;
  float m_TakeoffTimeout;

  float m_PickupTimeout;
  float m_ThrustLevel;

  bool m_TouchedLeft;
  bool m_TouchedRight;
  bool m_DebugDrawToggle;

  GLESDebugDraw *m_DebugDraw;

  SpaceShipDownContactListener *m_ContactListener;

  float m_WorldWidth;
  float m_WorldHeight;

  float m_CameraOffsetX;
  float m_CameraOffsetY;

  foofoo *m_PlayerFoo;
  foofoo *m_PlayerAfterburnerFoo;
  foofoo *m_SpaceShipPartBaseFoo;
  foofoo *m_SpaceShipPartTopFoo;
  foofoo *m_SpaceShipPartMiddleFoo;
  foofoo *m_SpaceShipPartAfterburnerFoo;
  foofoo *m_DropZoneFoo;
  foofoo *m_PlatformFoo;
  foofoo *m_LandscapeFoo;
  foofoo *m_EnemyFoo;
  foofoo *m_BatchFoo;
  foofoo *m_SecondBatchFoo;
  foofoo *m_ThirdBatchFoo;
  

};
