// Jon Bardin GPL

#include "SpaceShipDownContactListener.h"

class BaseVehicle : public SimpleVehicle {
public:
  BaseVehicle();
  void reset (void);
  void identify();
  void randomizeStartingPositionAndHeading (void);
  enum seekerState {running, tagged, atGoal};
  bool avoiding;
  static void initializeObstacles (void);
  static void addOneObstacle (void);
  static void removeOneObstacle (void);
  float minDistanceToObstacle (const Vec3 point);
  static int obstacleCount;
  static SOG allObstacles;
};

class EnemyVehicle : public BaseVehicle {
public:
  EnemyVehicle();
  void identify();
  void reset (void);
  void update (const float currentTime, const float elapsedTime);
  Vec3 steerToEvadeAllOtherEnemies (void);
};

class PlayerVehicle : public BaseVehicle {
public:
  PlayerVehicle();
  void reset (void);
  void identify();
  void update (const float currentTime, const float elapsedTime);
  void updateX (const float currentTime, const float elapsedTime, Vec3 inputSteering);
  bool clearPathToGoal (void);
  Vec3 steeringForSeeker (void);
  void updateState (const float currentTime);
  Vec3 steerToEvadeAllDefenders (void);
  Vec3 XXXsteerToEvadeAllDefenders (void);
  void adjustObstacleAvoidanceLookAhead (const bool clearPath);
  seekerState state;
  bool evading;
  float lastRunningTime;
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


  b2World *world;
  b2Body *m_PlayerBody;
  b2Joint *m_PickupJoint;
  b2Body *m_SpaceShipBaseBody;
  std::vector<b2JointDef*> m_PickupJointDefs;
  b2FrictionJointDef *m_FrictionJointDef;
  b2FrictionJoint *m_FrictionJoint;

  int m_LandscapeIndex;
  int m_PlayerIndex;
  int m_SpaceShipPartsStartIndex;
  int m_SpaceShipPartsStopIndex;
  int m_PlatformsStartIndex;
  int m_PlatformsStopIndex;
  int m_DropZonesStartIndex;
  int m_DropZonesStopIndex;
  int m_PickedUpPartIndex;
  int m_StackCount;
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
  

};
