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
	myGravity = -3500.0; //-500
	mySimulationTime = 0.0;
	myGameStarted = false;

	//myBuildSkyBoxDuration = 60.0;

	//myTextures = textures;
	
	myPlayerTexture = textures[0];
	myGroundTexture = textures[1];
	mySkyBoxTextures = (GLuint *) malloc(6 * sizeof(GLuint));
	/*
	mySkyBoxTextures[0] = textures[2];
	mySkyBoxTextures[1] = textures[3];
	mySkyBoxTextures[2] = textures[4];
	mySkyBoxTextures[3] = textures[5];
	mySkyBoxTextures[4] = textures[6];
	mySkyBoxTextures[5] = textures[7];
	 */
	
	myFontTexture = textures[8];
	myFountainTextures[0] = textures[9];

	buildFont();
	//buildSkyBox();
	
	myPlatformCount = 1;
	int i = 0;
	
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));

	myPlatforms[i].position = Vector3DMake(0.0, 0.0, 0.0);
	myPlatforms[i].length = 100.0;
	myPlatforms[i].amplitude = 0.0; //randf();
	myPlatforms[i].step = 1.0;
	myPlatforms[i].angular_frequency = 0.0;
	myPlatforms[i].phase = 0.0;
	
	buildCamera();
	
	
	
	
	
	
	
	
	
	
	
	myRaptorManager = new Md2Manager();
	
	myRaptors.push_back(myRaptorManager->Load((foo *)models.at(0), 25));
	
	for(std::vector<Md2Instance *>::const_iterator it = myRaptors.begin(); it != myRaptors.end(); ++it)
	{
		//cout << *it << " ";
		Md2Instance *raptor = (Md2Instance *)(*it);
		raptor->SetPosition(50.0, 0.0, 0.0);
		raptor->SwitchCycle(1);
		
		for (int cycle = 0; cycle < raptor->GetNumCycles(); cycle++) {
			LOGV("%d %d %s\n", raptor->GetNumCycles(), cycle, raptor->GetCycleName(cycle));
		}
		//delete raptor;
	}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	mySceneBuilt = true;
}

void RaptorIsland::render() {
	
	
	

	glPushMatrix();
	{
	//glTranslatef(0.0, 10.0, 0.0);

		float scale = 0.1;
		glScalef(scale, scale, scale);
		myRaptorManager->Render();
	}
	glPopMatrix();
	
	drawCamera();

	drawPlatform();
	//drawSkyBox();
	//drawFont();
	
	
	

	

	//LOGV("wtd");
	/*

		//glTranslatef(myPlayerPosition.x, myPlayerPosition.y + 1.5, myPlayerPosition.z);
		//glRotatef(myPlayerRotation, 0.0, 0.0, 1.0);
		//float scale = 1.0;
		//glScalef(scale, scale, scale);
		//bindTexture(myTextures[0]);
		//unbindTexture(myTextures[0]);

	 */
	
	
	
	
}


int RaptorIsland::simulate() {
	
	tickPlatform();
	tickCamera();
	
	myRaptorManager->Update(myDeltaTime);

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
	
	//Vector3D desiredPosition = Vector3DMake(-12.0, 10.0, 0.0);
	//Vector3D desiredTarget = Vector3DMake(20.0, 0.0, 0.0);
	
	Vector3D desiredPosition = Vector3DMake(-120.0, 100.0, 0.0);
	Vector3D desiredTarget = Vector3DMake(0.0, 0.0, 0.0);
	
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
