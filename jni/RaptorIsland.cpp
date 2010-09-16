//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

#include "RaptorIsland.h"

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

	myTextures = textures;
		
	buildCamera();
	 
	myRaptorHeight = 0.0;	
	myRaptorManager.SetStagger(3.0);
	
	for (int i=0; i<5; i++) {
		Md2Instance *raptor = myRaptorManager.Load(models[0], 24, myTextures[0]);
		myRaptors.push_back(raptor);
		raptor->SwitchCycle(1, 0.0, true, -1, 0);
		//raptor->SetPosition(randf() * 300.0, myRaptorHeight, );
		raptor->SetPosition(-25.0, myRaptorHeight, (randf() * 50.0) - 25.0);
		raptor->SetScale(0.2, 0.2, 0.2);
		raptor->SetRotation(90.0);
	}
	
	myBarrelHeight = 0.0;
	
	for (int i=0; i<0; i++) {
		Md2Instance *barrel;
		if (randf() > 0.5) {
			barrel = myBarrelManager.Load(models[1], 0, myTextures[2]);
		} else {
			barrel = myBarrelManager.Load(models[2], 0, myTextures[3]);
		}
		
		myBarrels.push_back(barrel);
		//barrel->SetPosition(i * 150.0, 0.0, i * 10.0);
		barrel->SetPosition(0.0, myBarrelHeight, 0.0);
		barrel->SetRotation(90.0);
	}
	
	
	mySkyBoxHeight = 10.0;
	mySkyBox = mySkyBoxManager.Load(models[2], 0, myTextures[4]);
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
	mySkyBox->SetRotation(90.0);
	mySkyBox->SetScale(0.5, 0.25, 0.5);

	
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
	
	mySimulationTime = 0.0;
	
	tick();
	
	mySceneBuilt = true;
	
}

void RaptorIsland::render() {

	drawCamera();
	
	bindTexture(0);
	myRaptorManager.Render();
	//myBarrelManager.Render();
	mySkyBoxManager.Render();
	unbindTexture(0);
		
	drawFont();
}


int RaptorIsland::simulate() {
	
	tickCamera();
	
	myRaptorManager.Update(myDeltaTime);
	//myBarrelManager.Update(myDeltaTime);
	mySkyBoxManager.Update(myDeltaTime);

	
	for(std::vector<Md2Instance *>::const_iterator it = myRaptors.begin(); it != myRaptors.end(); ++it)
	{
		Md2Instance *raptor = (Md2Instance *)(*it);
		if (raptor->GetPosition()[2] < -150.0) {
			raptor->SetPosition(-25.0, myRaptorHeight, 25.0);
			//raptor->SetPosition(0.0, myRaptorHeight, 150.0);

		}

		raptor->Move(0.9, 0.0, 0.0);
		raptor->SetRotation(90.0 + (fastSinf(mySimulationTime * 50.0) * (10.0)));
	}
	
	/*
	for(std::vector<Md2Instance *>::const_iterator it = myBarrels.begin(); it != myBarrels.end(); ++it)
	{
		Md2Instance *barrel = (Md2Instance *)(*it);
		barrel->SetRotation(mySimulationTime * 50.0);
	}
	 */
	
	//tickSkyBox();
	
	if (mySimulationTime > 30.0) {
		return 0;
	} else {
		return 1;
	}
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
	desiredTarget = Vector3DMake(0.1, 1.0, fastSinf(mySimulationTime * 4.0) * 4.0);
	//Vector3D desiredPosition = Vector3DMake(-49.0, 5.0, 0.0);
	Vector3D desiredPosition = Vector3DMake(-49.0, 1.0, 0.0);

	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;
	
}
