//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

#include "RaptorIsland.h"

void RaptorIsland::build(int width, int height, GLuint *textures, std::vector<foo*> models) {
	
	
	//Screen
	screenWidth = width;
	screenHeight = height;

	//World
	myGravity = 0.0;
	mySimulationTime = 0.0;
	myGameStarted = false;

	myTextures = textures;
	
	/*
	myPlayerTexture = textures[0];
	myGroundTexture = textures[1];
	mySkyBoxTextures = (GLuint *) malloc(6 * sizeof(GLuint));
	 
	 mySkyBoxTextures[0] = textures[2];
	 mySkyBoxTextures[1] = textures[3];
	 mySkyBoxTextures[2] = textures[4];
	 mySkyBoxTextures[3] = textures[5];
	 mySkyBoxTextures[4] = textures[6];
	 mySkyBoxTextures[5] = textures[7];
	 
	
	myFontTexture = textures[8];
	myFountainTextures[0] = textures[9];
	*/
	
	buildFont();
	buildCamera();
	
	
	
	myRaptorHeight = 22.0;	
	myRaptorManager = new Md2Manager();
	myRaptorManager->SetStagger(2.0);
	
	for (int i=0; i<1; i++) {
		Md2Instance *raptor = myRaptorManager->Load(models[0], 29);
		myRaptors.push_back(raptor);
		raptor->SwitchCycle(1, 0.0, true, -1, 0);
		raptor->SetPosition(randf() * 1000.0, myRaptorHeight, (randf() * 1000.0) - 500.0);
		raptor->SetRotation(90.0);
	}
	
	
	
	
	myBarrelHeight = 42.0;
	myBarrelManager = new Md2Manager();
	
	for (int i=0; i<1; i++) {
		myBarrels.push_back(myBarrelManager->Load(models[1], 24));
	}
	
	for(std::vector<Md2Instance *>::const_iterator it = myBarrels.begin(); it != myBarrels.end(); ++it)
	{
		Md2Instance *barrel = (Md2Instance *)(*it);
		barrel->SetCycle(0);
		barrel->SetPosition(500.0, myBarrelHeight, 0.0);
		//barrel->SetPosition(randf() * 1000.0, myRaptorHeight, (randf() * 1000.0) - 500.0);
		//barrel->SetRotation(randf() * 100.0);
	}
	
	for (int cycle = 0; cycle < myBarrels[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myBarrels[0]->GetNumCycles(), cycle, myBarrels[0]->GetCycleName(cycle));
	}
	
	
	
	
	
	myPlatformCount = 1;
	int i = 0;
	
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));

	myPlatforms[i].position = Vector3DMake(0.0, 0.0, 0.0);
	myPlatforms[i].length = 100.0;
	myPlatforms[i].amplitude = 0.0; //randf();
	myPlatforms[i].step = 1.0;
	myPlatforms[i].angular_frequency = 0.0;
	myPlatforms[i].phase = 0.0;
	
	mySceneBuilt = true;
	
}

void RaptorIsland::render() {

	drawCamera();
	
	
	glPushMatrix();
	{
		bindTexture(myTextures[0]);
		float scale = 0.1;
		glScalef(scale, scale, scale);
		myRaptorManager->Render();
		unbindTexture(myTextures[0]);
	}
	glPopMatrix();
	
	 
	 
	 
	glPushMatrix();
	{
		bindTexture(myTextures[12]);
		float scale = 0.0125;
		glScalef(scale, scale, scale);
		myBarrelManager->Render();
		unbindTexture(myTextures[12]);
	}
	glPopMatrix();
	
	drawPlatform();

	
	drawFont();
}


int RaptorIsland::simulate() {
	tickCamera();
	
	
	myRaptorManager->Update(myDeltaTime);
	myBarrelManager->Update(myDeltaTime);

	
	for(std::vector<Md2Instance *>::const_iterator it = myRaptors.begin(); it != myRaptors.end(); ++it)
	{
		Md2Instance *raptor = (Md2Instance *)(*it);
		if (raptor->GetPosition()[2] < -150.0) {
			raptor->SetPosition(randf() * 1000.0, myRaptorHeight, 150.0);
		}

		raptor->Move(0.9, 0.0, 0.0);
		raptor->SetRotation(90.0 + (fastSinf(mySimulationTime * 50.0) * 2.0));
	}
	
	/*
	if (randf() > 0.90) {
		Md2Instance *raptor = myRaptorManager->Load(models[0], 29);
		myRaptors.push_back(raptor);
		raptor->SwitchCycle(1, 0.0, true, -1, 0);
		raptor->SetPosition(randf() * 1000.0, myRaptorHeight, (randf() * 1000.0) - 500.0);
		raptor->SetRotation(90.0);
	}
	*/
	
	/*
	if (randf() > 0.25) {
		myRaptors[0]->SetCycle(3);
	} else {
		myRaptors[0]->SetCycle(1);
	}
	 */
	
	return 1;
}

void RaptorIsland::tickCamera() {
	
	Vector3D desiredPosition = Vector3DMake(-20.0, 5.0, 0.0);
	Vector3D desiredTarget = Vector3DMake(100.0, 0.0, 0.0);

	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;

}
