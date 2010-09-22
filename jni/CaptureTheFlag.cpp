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



#include <iomanip>
#include <sstream>
#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "Globals.h"

#include "CaptureTheFlag.h"

extern CtfSeeker* gSeeker;
extern std::vector<CtfEnemy*> ctfEnemies;

using namespace OpenSteer;

int CtfBase::obstacleCount = 0; // this value means "uninitialized"

// ----------------------------------------------------------------------------
// reset state

CtfBase::CtfBase() {
	reset();
}

CtfEnemy::CtfEnemy() {
	reset();
}


CtfSeeker::CtfSeeker() {
	reset();
}

void CtfBase::reset (void)
{
	SimpleVehicle::reset ();  // reset the vehicle 
	
	setSpeed(0);             // speed along Forward direction.
	setMaxForce(0.0);        // steering force is clipped to this magnitude
	setMaxSpeed(0.0);        // velocity is clipped to this magnitude
	
	avoiding = false;         // not actively avoiding
	
	clearTrailHistory ();     // prevent long streaks due to teleportation
}


void CtfSeeker::reset (void)
{
	CtfBase::reset();
	setPosition(gHomeBaseCenter);
	//setPosition(Vec3(0.0, 0.0, 0.0));
	setRadius(20.0);
	
	setSpeed(1);             // speed along Forward direction.
	setMaxSpeed(1.0);        // velocity is clipped to this magnitude
	setMaxForce(1000.0);        // steering force is clipped to this magnitude
	
	gSeeker = this;
	state = running;
	evading = false;
}


void CtfEnemy::reset (void)
{
	CtfBase::reset();
	//randomizeStartingPositionAndHeading();
	//setPosition(0.0, 0.0, 0.0);
	
	//printf("hit");
	float rz = (lrand48() % 255) / 255.f;
	float rx = (lrand48() % 255) / 255.f;
	rz = (rz * 20.0) - 10.0;
	//rz = 0.0;
	rx = (rx * 200.0) + 50;

	setPosition(rx, 0.0, rz);
	setRadius(2.0);
	setSpeed(25.0);
	setMaxSpeed(30.0);
	setMaxForce(100.0);
}


// ----------------------------------------------------------------------------
// draw this character/vehicle into the scene
void CtfBase::initializeObstacles (void)
{
	Vec3 c;
	float r = gObstacleRadius;
	
	/*
	for (int z=-60; z<=60; z+=4) {
		if (z > 20 || z < -20) {
			c = Vec3(-50.0 + (z * z * 0.015), 0, z);
			allObstacles.push_back (new SphereObstacle (r * 2.1, c));
			obstacleCount++;
		}
	}
	 */
	
	
	/*
	for (int i=0; i<14; i++) {
		c = Vec3(-30.0 + (i * 4), 0, -26.0 - i);
		allObstacles.push_back (new SphereObstacle (r * 1.5, c));
		obstacleCount++;
	}
	
	for (int i=0; i<14; i++) {
		c = Vec3(-30.0 + (i * 4), 0, 26.0 + i);
		allObstacles.push_back (new SphereObstacle (r * 1.5, c));
		obstacleCount++;
	}
	 */
	
	
	//allObstacles.push_back (new PlaneObstacle());

	
	for (int z=-60; z<=60; z+=20) {
		float rx = (lrand48() % 255) / 255.f;
		c = Vec3((rx > 0.5) ? -25.0 : -15, 0, z);
		allObstacles.push_back (new SphereObstacle (r, c));
		obstacleCount++;
	}
	
	
	//c = Vec3(-25.0, 0.0, 0.0);
	//allObstacles.push_back (new SphereObstacle (r, c));
	//obstacleCount++;
	
	//c = Vec3(-25.0, 0.0, -10.0);
	//allObstacles.push_back (new SphereObstacle (r, c));
	//obstacleCount++;

	/*
	
	// start with 40% of possible obstacles
	if (obstacleCount == -1)
	{
		int x = 0;
		int z = -20;
		int ii = 0;
		obstacleCount = 0;
		for (int i = 0; i < (maxObstacleCount); i++) {
			printf("obst");			
			
			float r = gObstacleRadius;
			Vec3 c = Vec3(x, 0, z + (ii * 15.0));
			
			if (((i + 1) % 3) == 0) {
				z = -25;
				x -= 15;
				ii = 0;
			}
			
			ii++;
			

			
			// add new non-overlapping obstacle to registry
			allObstacles.push_back (new SphereObstacle (r, c));
			obstacleCount++;
		}
	}
	 */
}


// ----------------------------------------------------------------------------


void CtfBase::randomizeStartingPositionAndHeading (void)
{
	// randomize position on a ring between inner and outer radii
	// centered around the home base
	const float rRadius = frandom2 (gMinStartRadius, gMaxStartRadius);
	const Vec3 randomOnRing = RandomUnitVectorOnXZPlane () * rRadius;
	setPosition (gHomeBaseCenter + randomOnRing);
	
	// are we are too close to an obstacle?
	if (minDistanceToObstacle (position()) < radius()*5)
	{
		// if so, retry the randomization (this recursive call may not return
		// if there is too little free space)
		randomizeStartingPositionAndHeading ();
	}
	else
	{
		// otherwise, if the position is OK, randomize 2D heading
		randomizeHeadingOnXZPlane ();
	}
}


// ----------------------------------------------------------------------------


void CtfEnemy::update (const float currentTime, const float elapsedTime)
{
	// determine upper bound for pursuit prediction time
	//const float seekerToGoalDist = Vec3::distance (gHomeBaseCenter, gSeeker->position());
	//const float adjustedDistance = seekerToGoalDist - radius() - gHomeBaseRadius;
	//const float seekerToGoalTime = ((adjustedDistance < 0 ) ? 0 : (adjustedDistance/gSeeker->speed()));
	const float maxPredictionTime = 0.5; //seekerToGoalTime * 0.9f;
	
	// determine steering (pursuit, obstacle avoidance, or braking)
	Vec3 steer (0, 0, 0);
	if (gSeeker->state == running)
	{
		Vec3 avoidance = steerToAvoidObstacles(gAvoidancePredictTimeMin, (ObstacleGroup&) allObstacles);

		// saved for annotation
		//avoiding = (avoidance == Vec3::zero);
		
		//if (avoiding) {
		//	steer += steerForPursuit(*gSeeker, maxPredictionTime);
		//} else {
		//	steer = avoidance;
		//}
		steer = avoidance + steerForPursuit(*gSeeker, maxPredictionTime);
	}
	else
	{
		applyBrakingForce (gBrakingRate, elapsedTime);
	}
	applySteeringForce (steer, elapsedTime);
	
	// detect and record interceptions ("tags") of seeker
	const float seekerToMeDist = Vec3::distance (position(), 
												 gSeeker->position());
	const float sumOfRadii = radius() + gSeeker->radius();
	
	if (seekerToMeDist < sumOfRadii)
	{
		if (gSeeker->state == running) {
			//gSeeker->state = tagged;
			//printf("hit");
			reset();
		}
	}
}

Vec3 CtfEnemy::steerToEvadeAllOtherEnemies (void)
{
	// sum up weighted evasion
	Vec3 evade (0, 0, 0);
	for (int i = 0; i < ctfEnemies.size(); i++)
	{
		const CtfEnemy& e = *ctfEnemies[i];
		if (position() != e.position()) {
			
			const Vec3 eOffset = e.position() - position();
			const float eDistance = eOffset.length();
			
			// xxx maybe this should take into account e's heading? xxx
			const float timeEstimate = 0.5f * eDistance / e.speed(); //xxx
			const Vec3 eFuture = e.predictFuturePosition (timeEstimate);
			
			// steering to flee from eFuture (enemy's future position)
			const Vec3 flee = xxxsteerForFlee (eFuture);
			
			const float eForwardDistance = forward().dot (eOffset);
			const float behindThreshold = radius() * -2;
			
			const float distanceWeight = 4 / eDistance;
			const float forwardWeight = ((eForwardDistance > behindThreshold) ?
										 1.0f : 0.5f);
			
			const Vec3 adjustedFlee = flee * distanceWeight * forwardWeight;
			
			evade += adjustedFlee;
		}
	}
	return evade;
}


// ----------------------------------------------------------------------------
// are there any enemies along the corridor between us and the goal?


bool CtfSeeker::clearPathToGoal (void)
{
	const float sideThreshold = radius() * 8.0f;
	const float behindThreshold = radius() * 2.0f;
	
	const Vec3 goalOffset = gHomeBaseCenter - position();
	const float goalDistance = goalOffset.length ();
	const Vec3 goalDirection = goalOffset / goalDistance;
	
	const bool goalIsAside = isAside (gHomeBaseCenter, 0.5);
	
	// for annotation: loop over all and save result, instead of early return 
	bool xxxReturn = true;
	
	// loop over enemies
	for (int i = 0; i < ctfEnemies.size(); i++)
	{
		// short name for this enemy
		const CtfEnemy& e = *ctfEnemies[i];
		const float eDistance = Vec3::distance (position(), e.position());
		const float timeEstimate = 0.3f * eDistance / e.speed(); //xxx
		const Vec3 eFuture = e.predictFuturePosition (timeEstimate);
		const Vec3 eOffset = eFuture - position();
		const float alongCorridor = goalDirection.dot (eOffset);
		const bool inCorridor = ((alongCorridor > -behindThreshold) && 
								 (alongCorridor < goalDistance));
		const float eForwardDistance = forward().dot (eOffset);
		
		// xxx temp move this up before the conditionals
		//annotationXZCircle (e.radius(), eFuture, clearPathColor, 20); //xxx
		
		// consider as potential blocker if within the corridor
		if (inCorridor)
		{
			const Vec3 perp = eOffset - (goalDirection * alongCorridor);
			const float acrossCorridor = perp.length();
			if (acrossCorridor < sideThreshold)
			{
				// not a blocker if behind us and we are perp to corridor
				const float eFront = eForwardDistance + e.radius ();
				
				//annotationLine (position, forward*eFront, gGreen); // xxx
				//annotationLine (e.position, forward*eFront, gGreen); // xxx
				
				// xxx
				// std::ostringstream message;
				// message << "eFront = " << std::setprecision(2)
				//         << std::setiosflags(std::ios::fixed) << eFront << std::ends;
				// draw2dTextAt3dLocation (*message.str(), eFuture, gWhite);
				
				const bool eIsBehind = eFront < -behindThreshold;
				const bool eIsWayBehind = eFront < (-2 * behindThreshold);
				const bool safeToTurnTowardsGoal =
				((eIsBehind && goalIsAside) || eIsWayBehind);
				
				if (! safeToTurnTowardsGoal)
				{
					// this enemy blocks the path to the goal, so return false
					//annotationLine (position(), e.position(), clearPathColor);
					// return false;
					xxxReturn = false;
				}
			}
		}
	}
	
	// no enemies found along path, return true to indicate path is clear
	// clearPathAnnotation (sideThreshold, behindThreshold, goalDirection);
	// return true;
	//if (xxxReturn)
	//clearPathAnnotation (sideThreshold, behindThreshold, goalDirection);
	return xxxReturn;
}


// ----------------------------------------------------------------------------








// ----------------------------------------------------------------------------


Vec3 CtfSeeker::steerToEvadeAllDefenders (void)
{
	Vec3 evade (0, 0, 0);
	const float goalDistance = Vec3::distance (gHomeBaseCenter, position());
	
	// sum up weighted evasion
	for (int i = 0; i < ctfEnemies.size(); i++)
	{
		const CtfEnemy& e = *ctfEnemies[i];
		const Vec3 eOffset = e.position() - position();
		const float eDistance = eOffset.length();
		
		const float eForwardDistance = forward().dot (eOffset);
		const float behindThreshold = radius() * 2;
		const bool behind = eForwardDistance < behindThreshold;
		if ((!behind) || (eDistance < 5))
		{
			if (eDistance < (goalDistance * 1.2)) //xxx
			{
				// const float timeEstimate = 0.5f * eDistance / e.speed;//xxx
				const float timeEstimate = 0.15f * eDistance / e.speed();//xxx
				const Vec3 future =
				e.predictFuturePosition (timeEstimate);
				
				//annotationXZCircle (e.radius(), future, evadeColor, 20); // xxx
				
				const Vec3 offset = future - position();
				const Vec3 lateral = offset.perpendicularComponent (forward());
				const float d = lateral.length();
				const float weight = -1000 / (d * d);
				evade += (lateral / d) * weight;
			}
		}
	}
	return evade;
}


Vec3 CtfSeeker::XXXsteerToEvadeAllDefenders (void)
{
	// sum up weighted evasion
	Vec3 evade (0, 0, 0);
	for (int i = 0; i < ctfEnemies.size(); i++)
	{
		const CtfEnemy& e = *ctfEnemies[i];
		const Vec3 eOffset = e.position() - position();
		const float eDistance = eOffset.length();
		
		// xxx maybe this should take into account e's heading? xxx
		const float timeEstimate = 0.5f * eDistance / e.speed(); //xxx
		const Vec3 eFuture = e.predictFuturePosition (timeEstimate);
		
		// steering to flee from eFuture (enemy's future position)
		const Vec3 flee = xxxsteerForFlee (eFuture);
		
		const float eForwardDistance = forward().dot (eOffset);
		const float behindThreshold = radius() * -2;
		
		const float distanceWeight = 4 / eDistance;
		const float forwardWeight = ((eForwardDistance > behindThreshold) ?
									 1.0f : 0.5f);
		
		const Vec3 adjustedFlee = flee * distanceWeight * forwardWeight;
		
		evade += adjustedFlee;
	}
	return evade;
}


// ----------------------------------------------------------------------------


Vec3 CtfSeeker::steeringForSeeker (void)
{
	// determine if obstacle avodiance is needed
	const bool clearPath = clearPathToGoal();
	adjustObstacleAvoidanceLookAhead (clearPath);
	const Vec3 obstacleAvoidance = steerToAvoidObstacles(gAvoidancePredictTime, (ObstacleGroup&) allObstacles);
	
	// saved for annotation
	avoiding = (obstacleAvoidance != Vec3::zero);
	
	if (avoiding)
	{
		// use pure obstacle avoidance if needed
		return obstacleAvoidance;
	}
	else
	{
		// otherwise seek home base and perhaps evade defenders
		const Vec3 seek = xxxsteerForSeek (gHomeBaseCenter);
		if (clearPath)
		{
			Vec3 s = limitMaxDeviationAngle(seek, 0.707f, forward());
			
			//annotationLine (position(), position() + (s * 0.2f), seekColor);
			return s;
		}
		else
		{
			if (0) // xxx testing new evade code xxx
			{
				// combine seek and (forward facing portion of) evasion
				const Vec3 evade = steerToEvadeAllDefenders ();
				const Vec3 steer = seek + limitMaxDeviationAngle (evade, 0.5f, forward());
				
				// annotation: show evasion steering force
				//annotationLine (position(),position()+(steer*0.2f),evadeColor);
				return steer;
			}
			else
				
			{
				const Vec3 evade = XXXsteerToEvadeAllDefenders ();
				const Vec3 steer = limitMaxDeviationAngle (seek + evade,
														   0.707f, forward());
				
				//annotationLine (position(),position()+seek, gRed);
				//annotationLine (position(),position()+evade, gGreen);
				
				// annotation: show evasion steering force
				//annotationLine (position(),position()+(steer*0.2f),evadeColor);
				return steer;
			}
		}
	}
}


// ----------------------------------------------------------------------------
// adjust obstacle avoidance look ahead time: make it large when we are far
// from the goal and heading directly towards it, make it small otherwise.


void CtfSeeker::adjustObstacleAvoidanceLookAhead (const bool clearPath)
{
	if (clearPath)
	{
		evading = false;
		const float goalDistance = Vec3::distance (gHomeBaseCenter,position());
		const bool headingTowardGoal = isAhead (gHomeBaseCenter, 0.98f);
		const bool isNear = (goalDistance/speed()) < gAvoidancePredictTimeMax;
		const bool useMax = headingTowardGoal && !isNear;
		gAvoidancePredictTime = (useMax ? gAvoidancePredictTimeMax : gAvoidancePredictTimeMin);
	}
	else
	{
		evading = true;
		gAvoidancePredictTime = gAvoidancePredictTimeMin;
	}
}


// ----------------------------------------------------------------------------


void CtfSeeker::updateState (const float currentTime)
{
	// if we reach the goal before being tagged, switch to atGoal state
	if (state == running)
	{
		const float baseDistance = Vec3::distance (position(),gHomeBaseCenter);
		if (baseDistance < (radius() + gHomeBaseRadius)) {
			//state = atGoal;
		}
	}
	
	// update lastRunningTime (holds off reset time)
	if (state == running)
	{
		lastRunningTime = currentTime;
	}
	else
	{
		const float resetDelay = 4;
		const float resetTime = lastRunningTime + resetDelay;
		if (currentTime > resetTime) 
		{
			// xxx a royal hack (should do this internal to CTF):
			//OpenSteerDemo::queueDelayedResetPlugInXXX ();
		}
	}
}



// ----------------------------------------------------------------------------
// update method for goal seeker
void CtfSeeker::update (const float currentTime, const float elapsedTime)
{
	updateX(currentTime, elapsedTime, Vec3::zero);
}

void CtfSeeker::updateX (const float currentTime, const float elapsedTime, Vec3 inputSteering)
{
	// do behavioral state transitions, as needed
	updateState (currentTime);
	
	// determine and apply steering/braking forces
	Vec3 steer (0, 0, 0);
	if (state == running) {
		steer = steeringForSeeker();
	} else {
		applyBrakingForce (gBrakingRate, elapsedTime);
	}
	applySteeringForce (steer, elapsedTime);
}


void CtfBase::addOneObstacle (void)
{
	/*
	if (obstacleCount < maxObstacleCount)
	{
		// pick a random center and radius,
		// loop until no overlap with other obstacles and the home base
		float r;
		Vec3 c;
		float minClearance;
		const float requiredClearance = gSeeker->radius() * 4; // 2 x diameter
		do
		{
			r = gObstacleRadius;
			c = randomVectorOnUnitRadiusXZDisk() * gMaxStartRadius * 1.1f;
			minClearance = FLT_MAX;
			
			for (SOI so = allObstacles.begin(); so != allObstacles.end(); so++)
			{
				testOneObstacleOverlap ((**so).radius, (**so).center);
			}
			
			testOneObstacleOverlap (gHomeBaseRadius - requiredClearance, gHomeBaseCenter);
		}
		while (minClearance < requiredClearance);
		
		// add new non-overlapping obstacle to registry
		allObstacles.push_back (new SphereObstacle (r, c));
		obstacleCount++;
	}
	 */
}


float CtfBase::minDistanceToObstacle (const Vec3 point)
{
	//float r = 0;
	Vec3 c = point;
	float minClearance = FLT_MAX;
	for (SOI so = allObstacles.begin(); so != allObstacles.end(); so++)
	{
		//testOneObstacleOverlap ((**so).radius, (**so).center);
	}
	return minClearance;
}


void CtfBase::removeOneObstacle (void)
{
	/*
	if (obstacleCount > 0)
	{
		obstacleCount--;
		allObstacles.pop_back();
	}
	 */
}

