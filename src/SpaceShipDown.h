// Jon Bardin GPL

#include "SpaceShipDownContactListener.h"

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
  

};
