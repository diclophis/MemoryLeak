//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//




#define ctfEnemyCount 15


#include "CaptureTheFlag.h"
#include "RaptorIsland.h"


extern CtfEnemy* ctfEnemies[ctfEnemyCount];

// ----------------------------------------------------------------------------
// dynamic obstacle registry
//
// xxx need to combine guts of addOneObstacle and minDistanceToObstacle,
// xxx perhaps by having the former call the latter, or change the latter to
// xxx be "nearestObstacle": give it a position, it finds the nearest obstacle
// xxx (but remember: obstacles a not necessarilty spheres!)


SOG CtfBase::allObstacles;










RaptorIsland::~RaptorIsland() {
	LOGV("dealloc RaptorIsland\n");
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
	 
	myRaptorHeight = 4.5;	
	myRaptorManager.SetStagger(3.0);
	
	// create the seeker ("hero"/"attacker")
	ctfSeeker = new CtfSeeker;
	all.push_back(ctfSeeker);
	
	// create the specified number of enemies, 
	// storing pointers to them in an array.
	for (int i = 0; i<ctfEnemyCount; i++)
	{
		ctfEnemies[i] = new CtfEnemy;
		all.push_back (ctfEnemies[i]);
	}
	
	CtfBase::initializeObstacles();
	
	for (int i=0; i<ctfEnemyCount; i++) {
		Md2Instance *raptor = myRaptorManager.Load(models[0], 5, myTextures[0]);
		myRaptors.push_back(raptor);
		raptor->SwitchCycle(1, 0.0, true, -1, 0);
		raptor->SetPosition(-25.0, myRaptorHeight, (randf() * 50.0) - 25.0);
		raptor->SetScale(0.2, 0.2, 0.2);
	}
	
	myBarrelHeight = 0.0;
	
	
	for (int i=0; i<CtfBase::obstacleCount; i++) {
		Md2Instance *barrel;
		//if (randf() > 0.5) {
			barrel = myBarrelManager.Load(models[1], 1, myTextures[2]);
		//} else {
		//	barrel = myBarrelManager.Load(models[2], 0, myTextures[3]);
		//}
		
		myBarrels.push_back(barrel);
		//barrel->SetPosition(i * 150.0, 0.0, i * 10.0);
		barrel->SetScale(0.05, 0.05, 0.05);
		barrel->SetPosition(0.0, myBarrelHeight, 0.0);
		barrel->SetRotation(90.0);
	}
	 
	
	
	mySkyBoxHeight = 25.0;
	mySkyBox = mySkyBoxManager.Load(models[2], 1, myTextures[4]);
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
	mySkyBox->SetRotation(90.0);
	mySkyBox->SetScale(0.5, 0.5, 0.5);

	/*
	myPlayerHeight = 0.0;
	myPlayer = myPlayerManager.Load(models[2], 1, myTextures[3]);
	myPlayer->SetPosition(0.0, myPlayerHeight, 0.0);
	myPlayer->SetRotation(90.0);
	myPlayer->SetScale(0.1, 0.1, 0.1);
	*/
	
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
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	mySimulationTime = 0.0;
	
	tick();
	
	mySceneBuilt = true;
	
}

void RaptorIsland::render() {

	drawCamera();
	
	bindTexture(0);
	myRaptorManager.Render();
	myBarrelManager.Render();
	//myPlayerManager.Render();
	mySkyBoxManager.Render();
	unbindTexture(0);
		
	drawFont();
}


int RaptorIsland::simulate() {
	
	tickCamera();
	
	myRaptorManager.Update(myDeltaTime);
	myBarrelManager.Update(myDeltaTime);
	mySkyBoxManager.Update(myDeltaTime);
	//myPlayerManager.Update(myDeltaTime);

	
	/*
	for(std::vector<Md2Instance *>::const_iterator it = myRaptors.begin(); it != myRaptors.end(); ++it)
	{
		Md2Instance *raptor = (Md2Instance *)(*it);
		if (raptor->GetPosition()[2] < -150.0) {
			raptor->SetPosition(-25.0, myRaptorHeight, 25.0);
			//raptor->SetPosition(0.0, myRaptorHeight, 150.0);

		}

		raptor->Move(0.4, 0.0, 0.0);
		raptor->SetRotation(90.0 + (fastSinf(mySimulationTime * 30.0) * (20.0)));
	}
	 */
	
	/*
	for(std::vector<Md2Instance *>::const_iterator it = myBarrels.begin(); it != myBarrels.end(); ++it)
	{
		Md2Instance *barrel = (Md2Instance *)(*it);
		barrel->SetRotation(mySimulationTime * 50.0);
	}
	 */
	
	//tickSkyBox();
	
	OpenSteer::Vec3 pos1a, vel1a, pos2a, vel2a;
	
	float rot1a, rot2a;
	
	ctfSeeker->updateX(mySimulationTime, myDeltaTime, steeringFromInput);
	
	// update each enemy
	for (int i = 0; i < ctfEnemyCount; i++)
	{
		ctfEnemies[i]->update(mySimulationTime, myDeltaTime);
		
		pos1a = ctfEnemies[i]->position();
		vel1a = ctfEnemies[i]->velocity();
		//rot2a = ctfEnemies[i]->speed() / ctfEnemies[i]->maxSpeed();
		if (vel1a.x != 0.0) {
			rot1a = atan2(vel1a.z, vel1a.x);
		}
		
		myRaptors[i]->SetRotation(-RadiansToDegrees(rot1a));
		myRaptors[i]->SetPosition(pos1a.x, myRaptorHeight, pos1a.z);
		
		//i++;
	}
	
	int i=0;
	for (SOI so = CtfBase::allObstacles.begin(); so != CtfBase::allObstacles.end(); so++)
	{
		OpenSteer::Vec3 a = (**so).center;
		if (a.x > -50.0) {
			if (a.z > 25.0) {
				a.z = -25.0;
			} else {
				a.z += 0.1;
			}
		}
		(**so).setCenter(a);

		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		i++;
	}
	
	
	//pos1a = ctfSeeker->position();
	//vel1a = ctfSeeker->velocity();
	//myPlayer->SetPosition(pos1a.x, pos1a.y, pos1a.z);
	
	
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
	if (myCameraPosition.y < 8.0) {
		myCameraSpeed = Vector3DMake(0.0, 10.0, 0.0);

	} else {
		myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
		
		/*
		//desiredTarget = Vector3DMake(myRaptors[0]->GetPosition()[0], 0.0, myRaptors[0]->GetPosition()[2]);
		float totalZ = 0.0;
		for(std::vector<Md2Instance *>::const_iterator it = myRaptors.begin(); it != myRaptors.end(); ++it)
		{
			Md2Instance *raptor = (Md2Instance *)(*it);
			totalZ += raptor->GetPosition()[1];
		}
		float averageZ = totalZ / (float)myRaptors.size();
		desiredTarget = Vector3DMake(100.0, 0.0, averageZ);
		 */
	}
	
	//Vector3D desiredPosition = Vector3DAdd(myCameraPosition, Vector3DMake(myCameraSpeed.x * myDeltaTime, myCameraSpeed.y * myDeltaTime, myCameraSpeed.z * myDeltaTime));
	//desiredTarget = Vector3DMake(0.1, 0.1, fastSinf(mySimulationTime * 4.0) * 4.0);
	desiredTarget = Vector3DMake(1.0, 0.0, 0.0);

	//Vector3D desiredPosition = Vector3DMake(-49.0, 5.0, 0.0);
	//Vector3D desiredPosition = Vector3DMake(-100.0, 100.0, 0.0);
	Vector3D desiredPosition = Vector3DMake(-49.0, 10.0, 0.0);

	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;
	
}
