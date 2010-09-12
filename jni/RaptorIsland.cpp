//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

#include "RaptorIsland.h"

void RaptorIsland::build(int width, int height, GLuint *textures, foo *playerFoo) {
	
	//Screen
	screenWidth = width;
	screenHeight = height;

	//World
	myGravity = -3500.0; //-500
	mySimulationTime = 0.0;
	myGameStarted = false;

	myBuildSkyBoxDuration = 60.0;

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

	buildFont();
	buildSkyBox();
	
	myPlatformCount = 1;
	int i = 0;
	
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));

	myPlatforms[i].position = Vector3DMake(-50.0, 0.0, -50.0);
	myPlatforms[i].length = 100.0;
	myPlatforms[i].amplitude = 0.0;
	myPlatforms[i].step = 100.0;
	myPlatforms[i].angular_frequency = 0.0;
	myPlatforms[i].phase = 0.0;
	
	buildCamera();
	
	mySceneBuilt = true;
}

void RaptorIsland::render() {
	drawCamera();
	drawPlatform();
	drawSkyBox();
	drawFont();
}


int RaptorIsland::simulate() {
	
	tickPlatform();
	tickCamera();
	
	return 1;
	
}

void RaptorIsland::tickCamera() {
	//Vector3D cameraPosition;
	//Vector3D cameraTarget;
	
	//float limitP;
	//float limitT;

	//cameraPosition = Vector3DMake(0.0, 10.0, 0.0);
	//cameraTarget = Vector3DMake(0.0, 0.0, 0.0);
		

	//limitP = 1.07 * myDeltaTime;//0.0020;
	//limitT = 1.1 * myDeltaTime;//0.0020;
	
	Vector3D desiredPosition = Vector3DMake(0.0, 5.0, 0.0);
	Vector3D desiredTarget = Vector3DMake(1000.0, 1.0, 0.0);
	
	/*
	 Vector3DFlip(&desiredPosition);
	 Vector3D deltaP = Vector3DAdd(desiredPosition, myCameraPosition);
	 deltaP = Vector3DLimit(deltaP, myPlayerSpeed.x * limitP);
	 Vector3DFlip(&deltaP);
	 
	 myCameraPosition = Vector3DAdd(myCameraPosition, deltaP);
	 
	 
	 Vector3DFlip(&desiredTarget);
	 Vector3D deltaT = Vector3DAdd(desiredTarget, myCameraTarget);
	 deltaT = Vector3DLimit(deltaT, myPlayerSpeed.x * limitT);
	 Vector3DFlip(&deltaT);
	 
	 myCameraTarget = Vector3DAdd(myCameraTarget, deltaT);
	 */
	
	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;
	
}
