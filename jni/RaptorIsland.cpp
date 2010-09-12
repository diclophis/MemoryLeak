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
	buildPlatforms();
	buildCamera();
	
	//buildPlayer(playerFoo);
	//buildSpiral();
	//buildFountain();
	
	mySceneBuilt = true;
}

void RaptorIsland::render() {

	drawCamera();


	drawPlatform();
	drawSkyBox();
	drawFont();
	 
	//drawFountain();
	//drawPlayer();
}


int RaptorIsland::simulate() {
	
	tickPlatform();
	tickCamera();

/*
tickFont();
tickPlayer();
//tickSpiral();
tickFountain();
}

if (mySimulationTime > 2.0) {
myGameStarted = true;
}

if (myPlayerPosition.y < -1500.0) {
return 0;
} else {
return myGameSpeed;
}	
*/
	
	return 1;
	
}