//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//


#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "CaptureTheFlag.h"

#include "importgl.h"
#include "OpenGLCommon.h"

#include "Engine.h"
#include "MachineGun.h"
#include "RaptorIsland.h"


CtfSeeker* gSeeker = NULL;
std::vector<CtfEnemy*> ctfEnemies;
extern std::vector<CtfEnemy*> ctfEnemies;
SOG CtfBase::allObstacles;


RaptorIsland::~RaptorIsland() {
	LOGV("dealloc RaptorIsland!!@!@!\n");
	myRaptors.clear();
	myRaptorManager.Release();
	
	myBarrels.clear();
	myBarrelManager.Release();
}


//RaptorIsland::RaptorIsland(int width, int height, std::vector<GLuint> x_textures, std::vector<foo*> x_models) {
//}


void RaptorIsland::build() {

	ctfSeeker = new CtfSeeker;
	all.push_back(ctfSeeker);

	for (int i = 0; i<10; i++) {
		CtfEnemy *enemy = new CtfEnemy;
		ctfEnemies.push_back(enemy);
		all.push_back (ctfEnemies[i]);
	}

	
	CtfBase::initializeObstacles();

	myRaptorHeight = 5.0;	
	for (unsigned int i=0; i<ctfEnemies.size(); i++) {
		myRaptors.push_back(myRaptorManager.Load(models[0], 30, textures[0]));
		myRaptors[i]->SwitchCycle(1, 0.0, false, -1, 1);
		myRaptors[i]->SetPosition(-25.0, myRaptorHeight, (randf() * 50.0) - 25.0);
		myRaptors[i]->SetScale(0.2, 0.2, 0.2);
	}


	for (unsigned int cycle = 0; cycle < myRaptors[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myRaptors[0]->GetNumCycles(), cycle, myRaptors[0]->GetCycleName(cycle));
	}


	myBarrelHeight = 0.0;
	for (int i=0; i<CtfBase::obstacleCount; i++) {
		Md2Instance *barrel;
		barrel = myBarrelManager.Load(models[1], 1, textures[2]);
		myBarrels.push_back(barrel);
		barrel->SetScale(0.05, 0.05, 0.05);
		barrel->SetPosition(0.0, myBarrelHeight, 0.0);
		barrel->SetRotation(90.0, 0.0);
	}

	
	mySkyBoxHeight = 12.5;
	mySkyBox = mySkyBoxManager.Load(models[3], 1, textures[4]);
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
	mySkyBox->SetRotation(90.0, 0.0);
	mySkyBox->SetScale(0.5, 0.25, 0.5);

	
	myPlayerHeight = 0.0;
	myPlayer = myPlayerManager.Load(models[2], 1, textures[3]);
	myPlayer->SetPosition(0.0, myPlayerHeight, 0.0);
	myPlayer->SetScale(0.15, 0.15, 0.15);
	
	/*
	for (int cycle = 0; cycle < myBarrels[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myBarrels[0]->GetNumCycles(), cycle, myBarrels[0]->GetCycleName(cycle));
	}
	 */
	
	int i=0;
	for (SOI so = CtfBase::allObstacles.begin(); so != CtfBase::allObstacles.end(); so++) {
		OpenSteer::Vec3 a = (**so).center;
		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		i++;
	}
	
	buildFountain();
	
	
	myLineVertices[0] = 0.0;
	myLineVertices[1] = 0.0;
	myLineVertices[2] = 0.0;
	myLineVertices[3] = 0.0;
	myLineVertices[4] = 0.0;
	myLineVertices[5] = 0.0;
	
	
	m_Gun = MachineGun(textures[5]);
	//m_Gun.buildFountain();
}

void RaptorIsland::render() {
	
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	mySkyBoxManager.Render();
	glDisable(GL_BLEND);

  /*

	glEnable(GL_DEPTH_TEST);
	myRaptorManager.Render();
	myBarrelManager.Render();
	glDisable(GL_DEPTH_TEST);
	*/
	
	drawFont();
	m_Gun.drawFountain();
	
	/*
	glEnable(GL_DEPTH_TEST);
	myPlayerManager.Render();
	glDisable(GL_DEPTH_TEST);	
	*/
	
	glDisable(GL_TEXTURE_2D);
	 
}


int RaptorIsland::simulate() {
	
	tickCamera();

	
	myRaptorManager.Update(myDeltaTime);
	myBarrelManager.Update(myDeltaTime);
	mySkyBoxManager.Update(myDeltaTime);
	myPlayerManager.Update(myDeltaTime);

	OpenSteer::Vec3 pos1a, vel1a, pos2a, vel2a;
	
	float rot1a;
	
	ctfSeeker->updateX(mySimulationTime, myDeltaTime, steeringFromInput);
	
	// update each enemy
	for (unsigned int i = 0; i < ctfEnemies.size(); i++) {
		ctfEnemies[i]->update(mySimulationTime, myDeltaTime);
		pos1a = ctfEnemies[i]->position();
		vel1a = ctfEnemies[i]->velocity();
		if (vel1a.x != 0.0) {
			rot1a = atan2(vel1a.z, vel1a.x);
		}
		
		myRaptors[i]->SetRotation(-RadiansToDegrees(rot1a), 0.0);
		myRaptors[i]->SetPosition(pos1a.x, myRaptorHeight, pos1a.z);
	}
	
	int i=0;
	for (SOI so = CtfBase::allObstacles.begin(); so != CtfBase::allObstacles.end(); so++)
	{
		OpenSteer::Vec3 a = (**so).center;
		
		if (randf() < 0.1) {
			myFountainPosition = Vector3DMake(a.x, a.y, a.z);
		}
		
		if (a.x == -25.0) {
			if (a.z < -60.0) {
				a.z = 60.0;
			} else {
				a.z -= 0.2;
			}
		} else if (a.x == -15.0) {
			if (a.z > 60.0) {
				a.z = -60.0;
			} else {
				a.z += 0.2;
			}
		}
		
		(**so).setCenter(a);

		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		i++;
	}
	
	
	pos1a = ctfSeeker->position();
	vel1a = ctfSeeker->velocity();
	myPlayer->SetPosition(pos1a.x, pos1a.y, pos1a.z);
	

	myLineVertices[3] = pos1a.x;
	myLineVertices[4] = pos1a.y + 2.0;
	myLineVertices[5] = pos1a.z;

	GLfloat m_GunHit[9];

	m_GunHit[0] = myLineVertices[0];
	m_GunHit[1] = myLineVertices[1];
	m_GunHit[2] = myLineVertices[2];

	m_GunHit[3] = pos1a.x;
	m_GunHit[4] = pos1a.y + 2.0;
	m_GunHit[5] = pos1a.z;

	Vec3 a,b,c;
	a.x = myLineVertices[0];
	a.y = myLineVertices[1];
	a.z = myLineVertices[2];
	b.x = myLineVertices[3];
	b.y = myLineVertices[4];
	b.z = myLineVertices[5];

	bool hit = false;
	m_LastCollide = Vec3(0.0, 0.0, 0.0);

	for (i = 0; i < ctfEnemies.size(); i++) {
		c = ctfEnemies[i]->position();
		if (c.x < 0.0) {
			hit = IntersectCircleSegment(c, 7.0, a, b);
			if (hit) {
				//myRaptors[i]->SwitchCycle(3 + (int)(randf() * 3.0), 0.02, false, 1, 1);
				myRaptors[i]->SwitchCycle(21, 0.02, false, 1, 1);
				m_LastCollide = c;
			}
		}
	}


    m_GunHit[6] = m_LastCollide.x;
    m_GunHit[7] = m_LastCollide.y + 2.0;
    m_GunHit[8] = m_LastCollide.z;


    m_Gun.SetVertices(m_GunHit);

	m_Gun.tickFountain();

	tickFountain();

	

	return 1;
}


void RaptorIsland::tickCamera() {
	Vector3D desiredTarget;
	Vector3D desiredPosition;

	//desiredTarget = Vector3DMake(1000.0, 8.0, 0.0);
	desiredTarget = Vector3DMake(0.0, 0.0, 0.0);
	//Vector3D desiredPosition = Vector3DMake(-49.0, 5.0, 0.0);
#ifdef DESKTOP
	//desiredPosition = Vector3DMake(-75.0, 75.0, 0.0);
	//desiredPosition = Vector3DMake(-49.0, 8.0, 0.0);
	//desiredPosition = Vector3DMake(-11.0, 50.0, 0.0);
	//desiredPosition = Vector3DMake(-49.0, 50.0 - (mySimulationTime * 10.0), 0.0);
	//desiredPosition = Vector3DMake(-49.0, 10.0, 0.0);
	//desiredPosition = Vector3DMake(-55.0 - (mySimulationTime * 8.0), 10.0 + (mySimulationTime * 7.0), mySimulationTime);
	desiredPosition = Vector3DMake(-80.0, 12.0, 0.0);
#else
	desiredPosition = Vector3DMake(-80.0, 12.0, 0.0);
	//desiredPosition = Vector3DMake(-49.0, 10.0, 0.0);

#endif
	
	myCameraTarget = desiredTarget;

	if (desiredPosition.y > 5.0) {
		myCameraPosition = desiredPosition;
	}
}


bool RaptorIsland::IntersectCircleSegment(const Vec3& c, float r, const Vec3& p1, const Vec3& p2) {
    Vec3 dir = p2 - p1;
    Vec3 diff = c - p1;
    float t = diff.dot(dir) / dir.dot(dir);
    if (t < 0.0f) {
        t = 0.0f;
	}
    if (t > 1.0f) {
        t = 1.0f;
	}
    Vec3 closest = p1 + t * dir;
    Vec3 d = c - closest;
    float distsqr = d.dot(d);
    return distsqr <= r * r;
}


void RaptorIsland::hitTest(float x, float y) {
	float zzz = x - (screenWidth / 2);
	float p = zzz / screenWidth;
	myLineVertices[0] = -25.0;
	myLineVertices[1] = 0.0;
	myLineVertices[2] = p * 20.0;
}
