#ifndef B2_BULLET_HELL_WORLD_H
#define B2_BULLET_HELL_WORLD_H

#include <Box2D/Common/b2Math.h>
#include <Box2D/Common/b2BlockAllocator.h>
#include <Box2D/Common/b2StackAllocator.h>
#include <Box2D/Dynamics/b2ContactManager.h>
#include <Box2D/Dynamics/b2WorldCallbacks.h>
#include <Box2D/Dynamics/b2TimeStep.h>

struct b2AABB;
struct b2BodyDef;
struct b2Color;
struct b2JointDef;
class b2Body;
class b2Draw;
class b2Fixture;
class b2Joint;
class b2BulletHellBody;

/// The world class manages all physics entities, dynamic simulation,
/// and asynchronous queries. The world also contains efficient memory
/// management facilities.
class BulletHellWorld : public b2World {
public:
	BulletHellWorld(const b2Vec2& gravity, bool doSleep);
  void Step(float32 dt, int32 velocityIterations, int32 positionIterations);
	bool IsLocked() const;
  //b2BulletHellBody* CreateBody(const b2BodyDef* def);
	void Solve(const b2TimeStep& step);
  bool m_Solve;
};

inline bool BulletHellWorld::IsLocked() const
{
  return true;
  //return (m_flags & e_locked) == e_locked;
}

#endif
