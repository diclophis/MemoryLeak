/*
 *  BasicVehicle.h
 *  WangChung
 *
 *  Created by Jon Bardin on 4/13/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef __cplusplus
#import "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Clock.h"
#include "OpenSteer/PlugIn.h"
#include "OpenSteer/Camera.h"
#include "OpenSteer/Utilities.h"
#include "OpenSteer/Color.h"
#endif

using namespace OpenSteer;

#define testOneObstacleOverlap(radius, center)               \
{                                                            \
float d = Vec3::distance (c, center);                    \
float clearance = d - (r + (radius));                    \
if (minClearance > clearance) minClearance = clearance;  \
}

typedef std::vector<SphereObstacle*> SOG;  // SphereObstacle group
typedef SOG::const_iterator SOI;           // SphereObstacle iterator

//static int obstacleCount = -1;
//static const int maxObstacleCount = 25;

const Vec3 gHomeBaseCenter(0, 0, 0);
const float gHomeBaseRadius = 0.1;

const float gMinStartRadius = 100;
const float gMaxStartRadius = 110;

const float gBrakingRate = 0.75;

const Color evadeColor(0.6f, 0.6f, 0.3f);
const Color seekColor(0.3f, 0.6f, 0.6f);
const Color clearPathColor(0.3f, 0.6f, 0.3f);

const float gAvoidancePredictTimeMin = 0.1f;
const float gAvoidancePredictTimeMax = 5.0;
float gAvoidancePredictTime = gAvoidancePredictTimeMin;

bool enableAttackSeek  = true;
bool enableAttackEvade = true;

int resetCount = 0;

const int enemyCount = 1;

static SOG allObstacles;
enum seekerState {running, tagged, atGoal};


class BasicVehicle : public SimpleVehicle {
public:
	BasicVehicle(){}
	void update(const float currentTime, const float elapsedTime){};
	
};


std::vector<BasicVehicle*> allEnemies;
//const AVGroup& getAllEnemies (void) {return (const AVGroup&) allEnemies;}
