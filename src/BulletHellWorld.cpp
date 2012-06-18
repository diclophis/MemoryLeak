// Jon Bardin GPL

#include "MemoryLeak.h"
#include <Box2D/Dynamics/b2Island.h>


BulletHellWorld::BulletHellWorld(const b2Vec2& gravity, bool doSleep) : b2World(gravity, doSleep) {
  m_Solve = false;
}


void BulletHellWorld::Step(float32 dt, int32 velocityIterations, int32 positionIterations) {
	b2Timer stepTimer;

	// If new fixtures were added, we need to find the new contacts.
	//if (m_flags & e_newFixture)
	//{
	//	m_contactManager.FindNewContacts();
	//	m_flags &= ~e_newFixture;
	//}

	m_flags = 0; //|= e_locked;

	b2TimeStep step;
	step.dt = dt;
	step.velocityIterations	= velocityIterations;
	step.positionIterations = positionIterations;
	if (dt > 0.0f)
	{
		step.inv_dt = 1.0f / dt;
	}
	else
	{
		step.inv_dt = 0.0f;
	}

	step.dtRatio = m_inv_dt0 * dt;

	step.warmStarting = m_warmStarting;
	
	// Update contacts. This is where some contacts are destroyed.
	{
		b2Timer timer;
		//m_contactManager.Collide();
		m_profile.collide = timer.GetMilliseconds();
	}

	// Integrate velocities, solve velocity constraints, and integrate positions.
	if (m_stepComplete && step.dt > 0.0f)
	{
		b2Timer timer;
		Solve(step);
		m_profile.solve = timer.GetMilliseconds();
	}

	// Handle TOI events.
	if (m_continuousPhysics && step.dt > 0.0f)
	{
		b2Timer timer;
		//SolveTOI(step);
		m_profile.solveTOI = timer.GetMilliseconds();
	}
}


void BulletHellWorld::Solve(const b2TimeStep& step)
{
    m_profile.solveInit = 0.0f;
    m_profile.solveVelocity = 0.0f;
    m_profile.solvePosition = 0.0f;
    
    // Size the island for the worst case.
    b2Island island(m_bodyCount,
                    m_contactManager.m_contactCount,
                    m_jointCount,
                    &m_stackAllocator,
                    m_contactManager.m_contactListener);
    
    // Clear all the island flags.
    for (b2Body* b = m_bodyList; b; b = b->m_next)
    {
      b->m_flags &= ~b2Body::e_islandFlag;
    }
    for (b2Contact* c = m_contactManager.m_contactList; c; c = c->m_next)
    {
      c->m_flags &= ~b2Contact::e_islandFlag;
    }
    for (b2Joint* j = m_jointList; j; j = j->m_next)
    {
      j->m_islandFlag = false;
    }
    
    // Build and simulate all awake islands.
    int32 stackSize = m_bodyCount;
    b2Body** stack = (b2Body**)m_stackAllocator.Allocate(stackSize * sizeof(b2Body*));
    for (b2Body* seed = m_bodyList; seed; seed = seed->m_next)
    {
      if (seed->m_flags & b2Body::e_islandFlag)
      {
        continue;
      }
      
      if (seed->IsAwake() == false || seed->IsActive() == false)
      {
        continue;
      }
      
      // The seed can be dynamic or kinematic.
      if (seed->GetType() == b2_staticBody)
      {
        continue;
      }
      
      // Reset island and stack.
      island.Clear();
      int32 stackCount = 0;
      stack[stackCount++] = seed;
      seed->m_flags |= b2Body::e_islandFlag;
      
      // Perform a depth first search (DFS) on the constraint graph.
      while (stackCount > 0)
      {
        // Grab the next body off the stack and add it to the island.
        b2Body* b = stack[--stackCount];
        b2Assert(b->IsActive() == true);
        island.Add(b);
        
        // Make sure the body is awake.
        b->SetAwake(true);
        
        // To keep islands as small as possible, we don't
        // propagate islands across static bodies.
        if (b->GetType() == b2_staticBody)
        {
          continue;
        }
        
        // Search all contacts connected to this body.
        for (b2ContactEdge* ce = b->m_contactList; ce; ce = ce->next)
        {
          b2Contact* contact = ce->contact;
          
          // Has this contact already been added to an island?
          if (contact->m_flags & b2Contact::e_islandFlag)
          {
            continue;
          }
          
          // Is this contact solid and touching?
          if (contact->IsEnabled() == false ||
              contact->IsTouching() == false)
          {
            continue;
          }
          
          // Skip sensors.
          bool sensorA = contact->m_fixtureA->m_isSensor;
          bool sensorB = contact->m_fixtureB->m_isSensor;
          if (sensorA || sensorB)
          {
            continue;
          }
          
          island.Add(contact);
          contact->m_flags |= b2Contact::e_islandFlag;
          
          b2Body* other = ce->other;
          
          // Was the other body already added to this island?
          if (other->m_flags & b2Body::e_islandFlag)
          {
            continue;
          }
          
          b2Assert(stackCount < stackSize);
          stack[stackCount++] = other;
          other->m_flags |= b2Body::e_islandFlag;
        }
        
        // Search all joints connect to this body.
        for (b2JointEdge* je = b->m_jointList; je; je = je->next)
        {
          if (je->joint->m_islandFlag == true)
          {
            continue;
          }
          
          b2Body* other = je->other;
          
          // Don't simulate joints connected to inactive bodies.
          if (other->IsActive() == false)
          {
            continue;
          }
          
          island.Add(je->joint);
          je->joint->m_islandFlag = true;
          
          if (other->m_flags & b2Body::e_islandFlag)
          {
            continue;
          }
          
          b2Assert(stackCount < stackSize);
          stack[stackCount++] = other;
          other->m_flags |= b2Body::e_islandFlag;
        }
      }
      
      b2Profile profile;
      island.Solve(&profile, step, m_gravity, m_allowSleep);
      m_profile.solveInit += profile.solveInit;
      m_profile.solveVelocity += profile.solveVelocity;
      m_profile.solvePosition += profile.solvePosition;
      
      // Post solve cleanup.
      for (int32 i = 0; i < island.m_bodyCount; ++i)
      {
        // Allow static bodies to participate in other islands.
        b2Body* b = island.m_bodies[i];
        if (b->GetType() == b2_staticBody)
        {
          b->m_flags &= ~b2Body::e_islandFlag;
        }
      }
    }
    
    m_stackAllocator.Free(stack);
    
    if (m_Solve)
    {
      b2Timer timer;
      // Synchronize fixtures, check for out of range bodies.
      for (b2Body* b = m_bodyList; b; b = b->GetNext())
      {
        // If a body was not in an island then it did not move.
        if ((b->m_flags & b2Body::e_islandFlag) == 0)
        {
          continue;
        }
        
        if (b->GetType() == b2_staticBody)
        {
          continue;
        }
        
        // Update fixtures (for broad-phase).
        b->SynchronizeFixtures();
      }
      
      // Look for new contacts.
      //m_contactManager.FindNewContacts();
      m_profile.broadphase = timer.GetMilliseconds();
    }
  }
