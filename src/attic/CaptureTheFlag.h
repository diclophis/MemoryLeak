// ----------------------------------------------------------------------------
//
//
// OpenSteer -- Steering Behaviors for Autonomous Characters
//
// Copyright (c) 2002-2005, Sony Computer Entertainment America
// Original author: Craig Reynolds <craig_reynolds@playstation.sony.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
//
// ----------------------------------------------------------------------------
//
//
// Capture the Flag   (a portion of the traditional game)
//
// The "Capture the Flag" sample steering problem, proposed by Marcin
// Chady of the Working Group on Steering of the IGDA's AI Interface
// Standards Committee (http://www.igda.org/Committees/ai.htm) in this
// message (http://sourceforge.net/forum/message.php?msg_id=1642243):
//
//     "An agent is trying to reach a physical location while trying
//     to stay clear of a group of enemies who are actively seeking
//     him. The environment is littered with obstacles, so collision
//     avoidance is also necessary."
//
// Note that the enemies do not make use of their knowledge of the 
// seeker's goal by "guarding" it.  
//
// XXX hmm, rename them "attacker" and "defender"?
//
// 08-12-02 cwr: created 
//
//
// ----------------------------------------------------------------------------



using namespace OpenSteer;

// ----------------------------------------------------------------------------
// short names for STL vectors (iterators) of SphereObstacle pointers
// (obsolete? replace with ObstacleGroup/ObstacleIterator ?)

typedef std::vector<SphereObstacle*> SOG;  // SphereObstacle group
typedef SOG::const_iterator SOI;           // SphereObstacle iterator

class CtfBase : public SimpleVehicle
{
public:
  // constructor
  CtfBase();

  // reset state
  void reset (void);

  void identify();

  void randomizeStartingPositionAndHeading (void);
  enum seekerState {running, tagged, atGoal};

  // xxx store steer sub-state for anotation
  bool avoiding;

  // dynamic obstacle registry
  static void initializeObstacles (void);
  static void addOneObstacle (void);
  static void removeOneObstacle (void);
  float minDistanceToObstacle (const Vec3 point);
  static int obstacleCount;
  //static const int maxObstacleCount;
  static SOG allObstacles;
};


class CtfSeeker : public CtfBase
{
public:

  // constructor
  CtfSeeker();

  // reset state
  void reset (void);

  void identify();

  // per frame simulation update
  void update (const float currentTime, const float elapsedTime);
  void updateX (const float currentTime, const float elapsedTime, Vec3 inputSteering);

  // is there a clear path to the goal?
  bool clearPathToGoal (void);

  Vec3 steeringForSeeker (void);
  void updateState (const float currentTime);
  Vec3 steerToEvadeAllDefenders (void);
  Vec3 XXXsteerToEvadeAllDefenders (void);
  void adjustObstacleAvoidanceLookAhead (const bool clearPath);

  seekerState state;
  bool evading; // xxx store steer sub-state for anotation
  float lastRunningTime; // for auto-reset
};


class CtfEnemy : public CtfBase
{
public:

  // constructor
  CtfEnemy();
  void identify();
  // reset state
  void reset (void);
  // per frame simulation update
  void update (const float currentTime, const float elapsedTime);
  Vec3 steerToEvadeAllOtherEnemies (void);
};
