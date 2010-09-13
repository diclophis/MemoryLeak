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
	
	myGroundTexture = textures[1];
	
	buildCamera();
	 
	myRaptorHeight = 22.0;	
	myRaptorManager.SetStagger(2.0);
	
	for (int i=0; i<20; i++) {
		Md2Instance *raptor = myRaptorManager.Load(models[0], 24);
		myRaptors.push_back(raptor);
		raptor->SwitchCycle(1, 0.0, true, -1, 0);
		raptor->SetPosition(randf() * 300.0, myRaptorHeight, (randf() * 500.0) - 250.0);
		//raptor->SetPosition(0.0, myRaptorHeight, (randf() * 1000.0) - 500.0);
		raptor->SetRotation(90.0);
	}
	
	myBarrelHeight = 0.0;
	
	for (int i=0; i<1; i++) {
		Md2Instance *barrel = myBarrelManager.Load(models[1], 0);
		myBarrels.push_back(barrel);
		barrel->SwitchCycle(0, 0.0, true, -1, 0);
		barrel->SetPosition(0.0, 0.0, 0.0);
		//barrel->SetRotation(90.0);
	}
	
	/*
	for (int cycle = 0; cycle < myBarrels[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myBarrels[0]->GetNumCycles(), cycle, myBarrels[0]->GetCycleName(cycle));
	}
	 */
	
	
	buildFont();
	
	
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
	
	mySceneBuilt = true;
	
}

void RaptorIsland::render() {

	drawCamera();
	
	glPushMatrix();
	{
		bindTexture(myTextures[0]);
		float scale = 0.20;
		glScalef(scale, scale, scale);
		myRaptorManager.Render();
		unbindTexture(myTextures[0]);
	}
	glPopMatrix();
	
	glPushMatrix();
	{
		bindTexture(myTextures[12]);
		float scale = 0.05;
		glScalef(scale, scale, scale);
		myBarrelManager.Render();
		unbindTexture(myTextures[12]);
	}
	glPopMatrix();
	
	drawPlatform();
	
	drawFont();
	
	drawSkyBox();
}


int RaptorIsland::simulate() {
	
	tickCamera();
	
	myRaptorManager.Update(myDeltaTime);
	myBarrelManager.Update(myDeltaTime);

	
	for(std::vector<Md2Instance *>::const_iterator it = myRaptors.begin(); it != myRaptors.end(); ++it)
	{
		Md2Instance *raptor = (Md2Instance *)(*it);
		if (raptor->GetPosition()[2] < -150.0) {
			raptor->SetPosition(randf() * 300.0, myRaptorHeight, 200.0);
			//raptor->SetPosition(0.0, myRaptorHeight, 150.0);

		}

		raptor->Move(0.9, 0.0, 0.0);
		raptor->SetRotation(90.0 + (fastSinf(mySimulationTime * 50.0) * (10.0)));
	}
	
	for(std::vector<Md2Instance *>::const_iterator it = myBarrels.begin(); it != myBarrels.end(); ++it)
	{
		Md2Instance *barrel = (Md2Instance *)(*it);
		barrel->SetRotation(mySimulationTime * 50.0);
	}
	
	tickSkyBox();
	
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
		desiredTarget = Vector3DMake(150.0, 0.0, 0.0);

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
		desiredTarget = Vector3DMake(200.0, 0.0, 0.0);
	}
	
	Vector3D desiredPosition = Vector3DAdd(myCameraPosition, Vector3DMake(myCameraSpeed.x * myDeltaTime, myCameraSpeed.y * myDeltaTime, myCameraSpeed.z * myDeltaTime));

	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;
	
}
