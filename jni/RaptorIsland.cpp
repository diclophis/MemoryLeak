//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//


#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "CaptureTheFlag.h"
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


void RaptorIsland::build(int width, int height, std::vector<GLuint> textures, std::vector<foo*> models) {
	//Screen
	screenWidth = width;
	screenHeight = height;

	//World
	myGravity = 0.0;
	mySimulationTime = 0.0;
	myGameStarted = false;
	myGameSpeed = 1;
	myDeltaTime = 1.0 / 60.0;

	myTextures = textures;


	buildCamera();


	ctfSeeker = new CtfSeeker;
	all.push_back(ctfSeeker);


	for (int i = 0; i<15; i++) {
		CtfEnemy *enemy = new CtfEnemy;
		ctfEnemies.push_back(enemy);
		all.push_back (ctfEnemies[i]);
	}

	
	CtfBase::initializeObstacles();

	myRaptorHeight = 2.5;	
	myRaptorManager.SetStagger(2.0);
	for (unsigned int i=0; i<ctfEnemies.size(); i++) {
		myRaptors.push_back(myRaptorManager.Load(models[0], 30, myTextures[0]));
		myRaptors[i]->SetCycle(1);
		myRaptors[i]->SetPosition(-25.0, myRaptorHeight, (randf() * 50.0) - 25.0);
		myRaptors[i]->SetScale(0.1, 0.1, 0.1);
	}


	for (unsigned int cycle = 0; cycle < myRaptors[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myRaptors[0]->GetNumCycles(), cycle, myRaptors[0]->GetCycleName(cycle));
	}


	myBarrelHeight = 0.0;
	for (int i=0; i<CtfBase::obstacleCount; i++) {
		Md2Instance *barrel;
		barrel = myBarrelManager.Load(models[1], 1, myTextures[2]);
		myBarrels.push_back(barrel);
		barrel->SetScale(0.05, 0.05, 0.05);
		barrel->SetPosition(0.0, myBarrelHeight, 0.0);
		barrel->SetRotation(90.0);
	}
	 
	
	
	mySkyBoxHeight = 12.5;
	mySkyBox = mySkyBoxManager.Load(models[2], 1, myTextures[4]);
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
	mySkyBox->SetRotation(90.0);
	mySkyBox->SetScale(0.5, 0.25, 0.5);

	
	myPlayerHeight = 0.0;
	myPlayer = myPlayerManager.Load(models[2], 1, myTextures[3]);
	myPlayer->SetPosition(0.0, myPlayerHeight, 0.0);
	myPlayer->SetRotation(90.0);
	myPlayer->SetScale(0.1, 0.1, 0.1);
	
	
	/*
	for (int cycle = 0; cycle < myBarrels[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myBarrels[0]->GetNumCycles(), cycle, myBarrels[0]->GetCycleName(cycle));
	}
	 */
	
	
	buildFont();
	
	/*
	myPlatformCount = 1;
	int i = 0;
	
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));

	myPlatforms[i].position = Vector3DMake(0.0, 0.0, 0.0);
	myPlatforms[i].length = 100.0;
	myPlatforms[i].amplitude = 0.0; //randf();
	myPlatforms[i].step = 100.0;
	myPlatforms[i].angular_frequency = 0.0;
	myPlatforms[i].phase = 0.0;
	
	buildSkyBox();
	*/
	
	
	

	
	
	
	
	
	
	
	
	
	
	int i=0;
	for (SOI so = CtfBase::allObstacles.begin(); so != CtfBase::allObstacles.end(); so++)
	{
		OpenSteer::Vec3 a = (**so).center;
		
		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		
		
		i++;
		
		//Ogre::Vector3 b = Vector3(a.x, a.y, a.z);
		
		/*
		if (frandom2(1.0, 2.0) >= 1.5) {
			obstacle = m_pSceneMgr->createEntity(gen->generate(), "rock.05.mesh");
			scale = Vector3(0.1, 0.1, 0.1);
		} else {
			obstacle = m_pSceneMgr->createEntity(gen->generate(), "tree.05.mesh");
			scale = Vector3(1.0, 1.0, 1.0);
		}
		 */
		
		/*
		node1 = m_pSceneMgr->getRootSceneNode()->createChildSceneNode(gen->generate());
		node1->attachObject(obstacle);
		node1->setScale(scale);
		node1->setPosition(b);
		*/
		/*
		 CONICAL = 0, ///< Conical shape
		 SPHERICAL, ///< Spherical shape
		 HEMISPHERICAL, ///< Hemispherical shape
		 CYLINDRICAL, ///< Cylindrical shape
		 TAPERED_CYLINDRICAL, ///< Tapered cylindrical shape
		 FLAME, ///< Flame shape
		 INVERSE_CONICAL, ///< Inverse conical shape
		 TEND_FLAME, ///< Tend flame shape
		 */
		
		/*
		 obstacle = m_pSceneMgr->createEntity(gen->generate(), "TreeMesh");
		 treeNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode();
		 treeNode->attachObject(obstacle);
		 treeNode->setPosition(b);
		 */
	}
	
	
	
	
	
	
	
	
	
	buildFountain();	
	
	
	
	
	
	
	
	
	mySimulationTime = 0.0;
		
	mySceneBuilt = true;
	
	simulate();
	go();
	
}

void RaptorIsland::render() {
	drawCamera();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	myRaptorManager.Render();
	myBarrelManager.Render();
	myPlayerManager.Render();
	mySkyBoxManager.Render();
	drawFountain();
	drawFont();
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
		bool hit = false;
		pos1a = ctfEnemies[i]->position();
		vel1a = ctfEnemies[i]->velocity();
		if (vel1a.x != 0.0) {
			rot1a = atan2(vel1a.z, vel1a.x);
		}
		
		myRaptors[i]->SetRotation(-RadiansToDegrees(rot1a));
		myRaptors[i]->SetPosition(pos1a.x, myRaptorHeight, pos1a.z);
		
		if (hit) {
			myRaptors[i]->SwitchCycle(6, 0.0, true, -1, 0);
		}
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
				a.z -= 0.1;
			}
		} else if (a.x == -15.0) {
			if (a.z > 60.0) {
				a.z = -60.0;
			} else {
				a.z += 0.1;
			}
		}
		
		(**so).setCenter(a);

		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		i++;
	}
	
	
	pos1a = ctfSeeker->position();
	vel1a = ctfSeeker->velocity();
	myPlayer->SetPosition(pos1a.x, pos1a.y, pos1a.z);
	

	tickFountain();

	
	//if (mySimulationTime > 30.0) {
	//	return 0;
	//} else {
		return 1;
	//}
}


void RaptorIsland::buildCamera() {
	//Camera
	myCameraTarget = Vector3DMake(0.0, 0.0, 0.0);
	myCameraPosition = Vector3DMake(-10.0, 0.0, 0.0);
	myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
}

void RaptorIsland::tickCamera() {
	Vector3D desiredTarget;
	Vector3D desiredPosition;

	desiredTarget = Vector3DMake(1.0, 5.0, 0.0);
	//Vector3D desiredPosition = Vector3DMake(-49.0, 5.0, 0.0);
	//desiredPosition = Vector3DMake(-75.0, 75.0, 0.0);
	desiredPosition = Vector3DMake(-49.0, 5.0, 0.0);

	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;
}
