//
//  RunAndJump.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


#include "importgl.h"
#include "OpenGLCommon.h"

#include "assimp.hpp"
#include "aiPostProcess.h"
#include "aiScene.h"

#include "Engine.h"
#include "RunAndJump.h"


RunAndJump::~RunAndJump() {
	LOGV("dealloc GameController\n");
	myPlayerManager.Release();
	//if (myPlatforms) {
	//	free(myPlatforms);
	//}
}


void RunAndJump::hitTest(float x, float y) {
	playerStartedJumping();
}


void RunAndJump::tickCamera() {
	//const float *playerRenderPosition = myPlayer->GetPosition();
	//myCameraTarget = Vector3DMake(playerRenderPosition[0] + 200.0, 0.0, 0.0);
	//myCameraPosition = Vector3DMake(playerRenderPosition[0] - 300.0, 100.0, 100.0);

  //from begin, persp
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 300.0, myPlayerPosition.y, 0.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x + 10.0, myPlayerPosition.y + 30.0, 0.0);

  //top down from side, persp
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 300.0, 20.0 + myPlayerPosition.y, -300.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 25.0, 325.0, 500.0);

  //top down from side, persp
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 1000.0, myPlayerPosition.y - 50.0, -500.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 400.0, 300.0, 450.0);

	//foo bar
	myCameraTarget = Vector3DMake(myPlayerPosition.x, myPlayerPosition.y, myPlayerPosition.z);
	myCameraPosition = Vector3DMake(myPlayerPosition.x - 1000.0, 1000.0, 1000.0);
	
  //top down from side, ortho
	//myCameraTarget = Vector3DMake(playerRenderPosition[0], playerRenderPosition[1] - 200.0, playerRenderPosition[2] - 100.0);
	//myCameraPosition = Vector3DMake(playerRenderPosition[0] - 100.0, playerRenderPosition[1] - 100.0, playerRenderPosition[2] + 50.0);

  //sideview, ortho
	//myCameraTarget = Vector3DMake(myPlayerPosition.x - 100.0, myPlayerPosition.y - 200.0, myPlayerPosition.z - 100.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 100.0, myPlayerPosition.y - 200.0, myPlayerPosition.z + 50.0);

  //
	//myCameraTarget = Vector3DMake(playerRenderPosition[0] - 100.0, playerRenderPosition[1] - 200.0, playerRenderPosition[2] - 100.0);
	//myCameraPosition = Vector3DMake(playerRenderPosition[0] - 100.0, playerRenderPosition[1] - 200.0, playerRenderPosition[2] + 50.0);
}


void RunAndJump::build() {

	myPlayerPosition = Vector3DMake(0.0, 250.0, 0.0);
	myPlayerSpeed = Vector3DMake(0.0, 0.0, 0.0);
	
	myGravity = -0.05;
	mySimulationTime = 0.0;
	myGameStarted = false;
	myGameSpeed = 1;
	myDeltaTime = 0.0;
	
	myPlayerAcceleration = Vector3DMake(0.0, 0.0, 0.0);
	myPlayerJumping = false;
	myPlayerLastJump = -1.0;
	myPlayerOnPlatform = false;

	LOGV("A %d %d\n", models->size(), textures->size());

	mySkyBoxHeight = 0.0;
	mySkyBox = mySkyBoxManager.Load(models->at(0), 1, textures->at(1));
  LOGV("really??? 1\n");
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
  LOGV("really??? 1\n");
	mySkyBox->SetScale(80.0, 80.0, 80.0);
  LOGV("really??? 1\n");
	mySkyBoxManager.Update(myDeltaTime);
	//mySkyBox->SetRotation(150.0, 0.0);
  LOGV("B\n");

  LOGV("C\n");

	//myPlayerHeight = 25.0;
	//myPlayer = myPlayerManager.Load(models[1], 30, textures[4]);
	//myPlayer->SetScale(1.4, 1.4, 1.4);

	/*
	myPlayerHeight = 0.0;
	myPlayer = myPlayerManager.Load(models->at(1), 30, textures->at(2));
	myPlayer->SetScale(1.0, 1.0, 1.0);

	myPlayer->SetPosition(myPlayerPosition.x, myPlayerPosition.y + myPlayerHeight, myPlayerPosition.z);
	myPlayer->SwitchCycle(1, 0.1, false, -1, 1);
  
	for (unsigned int cycle = 0; cycle < myPlayer->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myPlayer->GetNumCycles(), cycle, myPlayer->GetCycleName(cycle));
	}
	 */
	
	myPlayerScene = importer.ReadFile("0", 
									  aiProcess_Triangulate |
									  aiProcess_OptimizeMeshes |
									  aiProcess_OptimizeGraph |
									  aiProcess_JoinIdenticalVertices |
									  aiProcess_SortByPType
									  );
	/*
aiProcess_OptimizeGraph |
aiProcess_TransformUVCoords |
aiProcess_GenUVCoords |
aiProcess_CalcTangentSpace |
aiProcess_GenNormals |
aiProcess_GenSmoothNormals |
aiProcess_SplitLargeMeshes |
aiProcess_ImproveCacheLocality |
aiProcess_FixInfacingNormals |
	 */
	LOGV("%s\n", importer.GetErrorString());


	LOGV("A %d %d\n", models->size(), textures->size());

  LOGV("D\n");
	
	buildPlatforms();

	mySegmentCount = 0;
	for (unsigned int i=0; i<mySegmentCount; i++) {
		mySegments.push_back(mySegmentManager.Load(models->at(0), 1, textures->at(0)));
		mySegments[i]->SetScale(10.0, 2.0, 3.0);
		mySegments[i]->SetPosition(0.0, 0.0, 0.0);
		mySegments[i]->SetRotation(0.0, 0.0);
	}

  LOGV("E\n");

	myTerrainCount = 0;
	for (unsigned int i=0; i<myTerrainCount; i++) {
		myTerrains.push_back(myTerrainManager.Load(models->at(2), 1, textures->at(3)));
		myTerrains[i]->SetScale(3.7, 2.1, 3.7);
		myTerrains[i]->SetPosition(i * 8000.0, 0.0, 0.0);
		myTerrains[i]->SetRotation(i * 90.0, 0.0);
	}

  LOGV("F\n");
	myTerrainManager.Update(myDeltaTime);
	myPlayerManager.Update(myDeltaTime);
	mySegmentManager.Update(myDeltaTime);
  LOGV("G\n");
}


int RunAndJump::simulate() {
	tickCamera();
	mySkyBox->SetPosition(myPlayerPosition.x, mySkyBoxHeight, 0.0);
	tickPlatform();
	tickPlayer();
  for (unsigned int i=0; i<myTerrainCount; i++) {
    if (myTerrains[i]->GetPosition()[0] < myPlayerPosition.x - (2700.0 * 2)) {
      myTerrains[i]->SetPosition(myPlayerPosition.x + (3 * 2700.0), 0.0, 0.0);
		  myTerrains[i]->SetRotation(((int)randf() * 4.0) * 90.0 + (randf() * 10.0), 0.0);
    }
  }
	return 1;
}


void RunAndJump::render() {	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	//glDepthFunc(GL_LESS);
	//glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//mySkyBoxManager.Render();
	//myPlayerManager.Render();
	//mySegmentManager.Render();
	//myTerrainManager.Render();
	

	glPushMatrix();
	glTranslatef(myPlayerPosition.x ,myPlayerPosition.y, myPlayerPosition.z);
	//glRotatef(m_RotateY,0,1,0);
	//glRotatef(m_RotateZ,1,0,0);
	glScalef(1.5, 1.5, 1.5);
	//LOGV("A %d %d\n", models->size(), textures->size());

	glBindTexture(GL_TEXTURE_2D, textures->at(0));
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMALIZE);

	//float scale = 3.0;
	

	//struct aiMatrix4x4 m = myPlayerScene->mRootNode->mTransformation;
	//m.Scaling(aiVector3D(scale, scale, scale), m);
	// update transform
	//m.Transpose();
	//glPushMatrix();
	//glMultMatrixf((float*)&m);
	

	int cnt = myPlayerScene->mNumMeshes;
	for (unsigned int mm=0; mm<cnt; mm++) {
		const aiMesh& aimesh = *myPlayerScene->mMeshes[mm];
		
		
		if (aimesh.HasBones()) {
			LOGV("woo");
		}
		
		if (aimesh.HasTextureCoords(0)) {
			glTexCoordPointer(3, GL_FLOAT, 0, aimesh.mTextureCoords[0]);
		}
		glVertexPointer(3, GL_FLOAT, 0, myPlayerScene->mMeshes[mm]->mVertices);
		glNormalPointer(GL_FLOAT, 0, myPlayerScene->mMeshes[mm]->mNormals);
		unsigned short* indices = new unsigned short[myPlayerScene->mMeshes[mm]->mNumFaces * 3];
		for(unsigned int i=0,j=0; i<myPlayerScene->mMeshes[mm]->mNumFaces; ++i,j+=3)
		{
			indices[j]   = myPlayerScene->mMeshes[mm]->mFaces[i].mIndices[0];
			indices[j+1] = myPlayerScene->mMeshes[mm]->mFaces[i].mIndices[1];
			indices[j+2] = myPlayerScene->mMeshes[mm]->mFaces[i].mIndices[2];
		}
		glDrawElements(GL_TRIANGLES,3 * myPlayerScene->mMeshes[mm]->mNumFaces, GL_UNSIGNED_SHORT, indices);
		delete indices;
	}
	glDisable(GL_NORMALIZE);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);
	glPopMatrix();
	
	
	
	
	
	
	
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}


void RunAndJump::playerStartedJumping() {
	if (myPlayerCanDoubleJump) {
		myPlayerCanDoubleJump = false;
		myPlayerLastJump = mySimulationTime;
	}
	
	if (myPlayerOnPlatform) {
		myPlayerLastJump = mySimulationTime;
	}	
}


void RunAndJump::playerStoppedJumping() {
	myPlayerLastEnd = mySimulationTime;
}


void RunAndJump::tickPlayer() {
	bool myPlayerFalling = true;
	float timeSinceStarted;

	myPlayerAcceleration.y = myGravity;
	
	Vector3D oldPosition = myPlayerPosition;
			
	timeSinceStarted = (mySimulationTime - myPlayerLastJump);

 // LOGV("thefuck: %f\n", timeSinceStarted);
	
	if (myPlayerLastJump > 0.0 && timeSinceStarted < (40.0) && timeSinceStarted > 0.0) {
		//13 is shrink
		//14 is stay shrunk
		//15 is grow
		//19 is side
		//
		//myPlayer->SwitchCycle(2, 0.2, false, -1, 1);
		myPlayerFalling = false;
		if (myPlayerJumping) {
			myPlayerAcceleration.x = 0.07;
			myPlayerAcceleration.y = 0.0;
      //LOGV(".");
		} else {
			myPlayerAcceleration.y = 0.66;
			myPlayerJumping = true;
      //LOGV("boom\n");
		}
	}
	
	if (myPlayerFalling) {		
		myPlayerJumping = false;
		if (myPlayerOnPlatform) {
		  myPlayerSpeed.y = 0.0;
			myPlayerAcceleration.y = 0.0;
		}
		if (myPlayerLastJump > 0) {
			myPlayerLastJump = -1.0;
		}
	}
	
	if (myPlayerOnPlatform) {
		myPlayerCanDoubleJump = true;
	}

	
	myPlayerSpeed = Vector3DAdd(myPlayerSpeed, Vector3DMake(myPlayerAcceleration.x * myDeltaTime, myPlayerAcceleration.y * myDeltaTime, myPlayerAcceleration.z * myDeltaTime));

  float myPlayerDec = 0.0002 * myDeltaTime;

  if (myPlayerSpeed.x > 10.0) {
    myPlayerSpeed.x = 10.0;
  } else if (myPlayerSpeed.x < -8.0) {
    myPlayerSpeed.x = -8.0;
  } else if (fabs(myPlayerSpeed.x) > 1.1) {
    if (myPlayerSpeed.x > 0.0) {
      myPlayerAcceleration.x -= myPlayerDec;
    } else {
      myPlayerAcceleration.x += myPlayerDec;
    }
  } else if (!myPlayerJumping) {
    myPlayerSpeed.x = 0.0;
    myPlayerAcceleration.x = 0.0;
  }
  

  if (myPlayerSpeed.y > 0.1 || myPlayerSpeed.y < -0.1) {
    //LOGV("myPlayerSpeed.y %d %f %f\n", myPlayerOnPlatform, myPlayerSpeed.y, myDeltaTime);
  }

	myPlayerPosition = Vector3DAdd(myPlayerPosition, Vector3DMake(myPlayerSpeed.x * myDeltaTime, myPlayerSpeed.y * myDeltaTime, myPlayerSpeed.z * myDeltaTime));

	//myPlayer->SetPosition(myPlayerPosition.x, myPlayerPosition.y + myPlayerHeight, 0.0);
	//myPlayer->SetRotation(90.0, -myPlayerRotation);
}


void RunAndJump::buildPlatforms() {	
	myPlatformCount = 100;
	Vector3D lastPlatformPosition = Vector3DMake(0.0, 0.0, 0.0);
	//myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));
	int step;
	float randomY;
	float randomA;
	float randomL;
	int length;
	
	lastPlatformPosition.x = 0.0;
	lastPlatformPosition.y = 0.0;

	for (int i=0; i<myPlatformCount; i++) {
		randomY = lastPlatformPosition.y;
		while (fabs(lastPlatformPosition.y - randomY) < 100.0) {
			randomY = ((random() / (float)RAND_MAX) * 101.0);
		}

		//randomA = ((random() / (float)RAND_MAX) * 12.0);
		//randomL = 40.0 - (i * randf());
    //randomY = 0.0;
    randomA = 0.0;
    randomL = ((randf() * 5.0) + 1.0) * 2000.0; //2000.0
		
		length = randomL;
		//step = 50.0;
		step = 1000.0; //randomL * 0.5; //100.0;
		Platform f;
		f.position = Vector3DMake(lastPlatformPosition.x, randomY, 0.0);
		f.length = length;
		f.amplitude = randomA;
		
		f.step = step;
		f.angular_frequency = 0.030;
		f.phase = 0.0;
		lastPlatformPosition = f.position;
		lastPlatformPosition.x += length; // + 100.0;
		myPlatforms.push_back(f);
	}
}


void RunAndJump::tickPlatform() {
	mySegmentIndex = 0;
	myPlayerOnPlatform = false;
	myPlayerPlatformCorrection = Vector3DMake(0.0, 0.0, 0.0);
	iteratePlatform(0);
}


void RunAndJump::drawPlatform() {
	//mySegmentIndex = 0;
	//glBindTexture(GL_TEXTURE_2D, textures[5]);
	//iteratePlatform(1);
}


void RunAndJump::iteratePlatform(int operation) {
  bool tickedWall = false;
  float lastHeight = 0.0;
	
	std::vector<Platform>::iterator it;
	
	
	for (it=myPlatforms.begin() ; it < myPlatforms.end(); it++ ) {
		Platform platform = *it;

		float foo = platform.position.x;
		
		//Vector3D pos = platform.position;
		
	//for (int j=0; j<myPlatformCount; j++) {
	//	Platform platform = myPlatforms.at(j);
		//if (&platform) {
		//if ((platform.position.x > (myPlayerPosition.x - platform.length - 10.0)) && (platform.position.x < (myPlayerPosition.x + platform.length + 10.0))) {
			for (int i = (int)platform.position.x; i < platform.position.x + platform.length; i += platform.step) {


				if ((i < (myPlayerPosition.x + 4000.0)) && (i > (myPlayerPosition.x - 1000.0))) {

					float beginX = i;
					float endX = i + platform.step;
					
					float phaseX = platform.phase;
					float phaseY = phaseX;
					
					float beginY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (beginX + phaseX));
					float endY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (endX + phaseY));
					
					platform.last_angular_frequency = platform.angular_frequency;

          if (!tickedWall) {
            if (platform.position.x > myPlayerPosition.x) {
              //first platform after player
              float heightDiff = platform.position.y - lastHeight;
              //LOGV("heightDiff %f\n", heightDiff);
              if (myPlayerPosition.x > (beginX - (platform.step * 0.2)) && myPlayerPosition.x < (beginX)) {
                if (myPlayerPosition.y < platform.position.y) {
                  myPlayerSpeed.x = -(myPlayerSpeed.x);
                  myPlayerAcceleration.x = 0.2;
                }
              }
              tickedWall = true;
            }
          }
					
					switch (operation) {
						case 0:
							if (mySegmentIndex < mySegmentCount) {
								//mySegments[mySegmentIndex]->SetPosition(beginX, beginY - 50.0, 25.0 * fastSinf(0.01 * (beginX + phaseX)));

								mySegments[mySegmentIndex]->SetPosition(beginX + (platform.step * 0.5), beginY - 100.0, 0.0);
								mySegmentIndex++;
							}
							tickPlatformSegment(beginX, beginY, endX, endY);
							break;
					}
				}
			}
		//}
    lastHeight = platform.position.y;
	}
}


void RunAndJump::drawPlatformSegment(float baseY, float x1, float y1, float x2, float y2) {


  int i = 0;
  float tex = 1.0;
  int index = 0;
  int tindex = 0;
  int number = 6 * 3 * 1;

  GLfloat platformRadius = 50.0;
  GLfloat deep = -25.0;

	GLfloat *platformVertices = (GLfloat *)malloc(number * sizeof(GLfloat));
	//GLfloat platformVertices[number];
	//GLfloat myPlatformTextureCoords[6 * 2 * 1];
	//LOGV("wtf: %d\n", index++);
	GLfloat *myPlatformTextureCoords = (GLfloat *)malloc(6 * 2 * 1 * sizeof(GLfloat));

		//1
		platformVertices[index++] = x1;
		platformVertices[index++] = y1;
		platformVertices[index++] = (deep);
		
		//2
		platformVertices[index++] = x1;
		platformVertices[index++] = y1;//beginY + lift;
		platformVertices[index++] = (deep + platformRadius); //0 - 10
		
		//3
		platformVertices[index++] = x2;
		platformVertices[index++] = y2; //endY + lift;
		platformVertices[index++] = (deep + platformRadius);
		
		//4
		platformVertices[index++] = x2;
		platformVertices[index++] = y2; //endY + lift;
		platformVertices[index++] = (deep + platformRadius);
		
		//5
		platformVertices[index++] = x2;
		platformVertices[index++] = y2;
		platformVertices[index++] = (deep);
		
		//6
		platformVertices[index++] = x1;
		platformVertices[index++] = y1;
		platformVertices[index++] = (deep);


		myPlatformTextureCoords[tindex++] = (i) * tex;
		myPlatformTextureCoords[tindex++] = 0.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex + tex;
		myPlatformTextureCoords[tindex++] = 0.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex + tex;
		myPlatformTextureCoords[tindex++] = 1.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex + tex;
		myPlatformTextureCoords[tindex++] = 1.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex;
		myPlatformTextureCoords[tindex++] = 1.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex;
		myPlatformTextureCoords[tindex++] = 0.0;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, platformVertices);
	glTexCoordPointer(2, GL_FLOAT, 0, myPlatformTextureCoords);
	
	
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	

	free(platformVertices);
	free(myPlatformTextureCoords);
	
  /*
	float beginX; float beginY; float endX; float endY;
	
	baseY -= 0.0;
	
	float platformRadius = 10.0;

	int total = 4;

	float deep = -((platformRadius * (float)total) * 0.5);
	
	int number = 6 * 3 * total;
	
	//GLfloat platformVertices[number];
	GLfloat *platformVertices = (GLfloat *)malloc(number * sizeof(GLfloat));
  for (int i=0; i<number; i++) {
    platformVertices[i] = 0.0;
  }

	GLfloat myPlatformTextureCoords[2 * 3 * 2 * total];
	
	int index = 0;
	int tindex = 0;

	float lift = 0.0;
		
	int middle = (total / 2);
	
	float y3;
	float y4;
	
	float tex = 1.0 / (float)total;
	
	for (int i=0; i<total; i++ ) {
		platformRadius += 0.0;
		if (i == middle) {
			lift = 0.0;
			beginY = y1;
			endY = y2;
			y3 = beginY;
			y4 = endY;
		} else if (i < middle) {
			beginY = baseY + (float)i / (float)middle * (y1 - baseY);
			endY = baseY + (float)i / (float)middle * (y2 - baseY);
			y3 = baseY + (float)(i + 1) / (float)middle * (y1 - baseY);
			y4 = baseY + (float)(i + 1) / (float)middle * (y2 - baseY);
		} else {
			beginY = y1 - (float)(i - (float)middle - 1) / (float)middle * (y1 - baseY);
			endY = y2 - (float)(i - (float)middle - 1) / (float)middle * (y2 - baseY);
			y3 = y1 - (float)((i - (float)middle)) / (float)middle * (y1 - baseY);
			y4 = y2 - (float)((i - (float)middle)) / (float)middle * (y2 - baseY);
		}
		
		
		beginX = x1;
		endX = x2;
		
		//1
		platformVertices[index++] = beginX;
		platformVertices[index++] = beginY;
		platformVertices[index++] = (deep); //-10 - 0
		
		//2
		platformVertices[index++] = beginX;
		platformVertices[index++] = y3;//beginY + lift;
		platformVertices[index++] = (deep + platformRadius); //0 - 10
		
		//3
		platformVertices[index++] = endX;
		platformVertices[index++] = y4; //endY + lift;
		platformVertices[index++] = (deep + platformRadius);
		
		//4
		platformVertices[index++] = endX;
		platformVertices[index++] = y4; //endY + lift;
		platformVertices[index++] = (deep + platformRadius);
		
		//5
		platformVertices[index++] = endX;
		platformVertices[index++] = endY;
		platformVertices[index++] = (deep);
		
		//6
		platformVertices[index++] = beginX;
		platformVertices[index++] = beginY;
		platformVertices[index++] = (deep);
				
		
		myPlatformTextureCoords[tindex++] = (i) * tex;
		myPlatformTextureCoords[tindex++] = 0.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex + tex;
		myPlatformTextureCoords[tindex++] = 0.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex + tex;
		myPlatformTextureCoords[tindex++] = 1.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex + tex;
		myPlatformTextureCoords[tindex++] = 1.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex;
		myPlatformTextureCoords[tindex++] = 1.0;
		
		myPlatformTextureCoords[tindex++] = (i) * tex;
		myPlatformTextureCoords[tindex++] = 0.0;
		
		deep += platformRadius;
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, platformVertices);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, myPlatformTextureCoords);
	glDrawArrays(GL_TRIANGLES, 0, number);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);

  */

  //free(platformVertices);
}


void RunAndJump::tickPlatformSegment(float beginX, float beginY, float endX, float endY) {
	if (!myPlayerJumping && !(myPlayerSpeed.y > 250.0)) {
		Vector3D p1 = Vector3DMake(beginX, beginY, 0.0);
		Vector3D p2 = Vector3DMake(endX, endY, 0.0);
		
		Vector3D pp1 = Vector3DAdd(myPlayerPosition, Vector3DMake(0.0, 30.0, 0.0));
		Vector3D pp2 = Vector3DAdd(myPlayerPosition, Vector3DMake(0.0, -30.0, 0.0));
		
		Vector3D p = p1;
		
		Vector3DFlip(&p1);
		Vector3D r = Vector3DAdd(p2, p1);
		Vector3DFlip(&p1);
		
		Vector3D q = pp1;
		
		Vector3DFlip(&pp1);
		Vector3D s = Vector3DAdd(pp2, pp1);
		Vector3DFlip(&pp1);
		
		float rCrossS = Vector3DCrossProduct(r, s).z;
		
		Vector3DFlip(&p);
		float t = Vector3DCrossProduct(Vector3DAdd(q, p), s).z / rCrossS;
		Vector3DFlip(&p);
		
		Vector3DFlip(&p);
		float u = Vector3DCrossProduct(Vector3DAdd(q, p), r).z / rCrossS;
		Vector3DFlip(&p);
		
		if (0 <= u && u <= 1 && 0 <= t && t <= 1) {
			Vector3D timesT = Vector3DMake(r.x * t, r.y * t, r.z * t);
			myPlayerPlatformIntersection = Vector3DAdd(p, timesT);
			float slope = (endY - beginY) / (endX - beginX);
			//if (slope > 0) {
				myPlayerRotation = slope * 40.0;
			//} else {
			//	myPlayerRotation = slope;
			//}
			float distanceFromIntersection = myPlayerPosition.y - myPlayerPlatformIntersection.y;
			myPlayerPlatformCorrection.y = -(distanceFromIntersection / myDeltaTime) - (myPlayerAcceleration.y * myDeltaTime);
			myPlayerOnPlatform = true;
			myGameSpeed = 1;
		}
	}
}

/*
void RunAndJump::tickSpiral() {
	myGarbageCollectorPosition = myPlayerPosition;
}
 */


/*
void RunAndJump::buildSpiral() {
	int dots = 0;
	int lines_from_dot = 0;
	
	mySpiralArrays = (dots * 3) + ((dots * lines_from_dot) * 3);
	mySpiralVertices = (GLfloat *)malloc(mySpiralArrays * sizeof(GLfloat));
	
    GLfloat x,y,z,angle,incline,interval;
    int c = 0;
	float r = 10.0;
    z = 0;
	interval = 1.0 / 4.0 * (2 * M_PI);
	incline = -0.0;
	angle = 0.0;
	while (c < mySpiralArrays) {
		angle += interval;
		x = r * sin(angle);
		y = r * cos(angle);
		mySpiralVertices[c++] = x;
		mySpiralVertices[c++] = y;
		mySpiralVertices[c++] = z; 
		z += incline;
	}
}
 */

/*
void RunAndJump::drawSpiral() {
	glPushMatrix();
	{
		float rotation = mySimulationTime * 360.0;
		glTranslatef(myGarbageCollectorPosition.x, myGarbageCollectorPosition.y, myGarbageCollectorPosition.z);	
		glRotatef(90.0, 0.0f, 1.0f, 0.0f);
		//glScalef(1.0, fastSinf(mySimulationTime) + 1.0, fastSinf(mySimulationTime) + 1.0);
		glRotatef(rotation, 1.0, -fastSinf(mySimulationTime), fastSinf(mySimulationTime));

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, mySpiralVertices);
		glDrawArrays(GL_POINTS, 0, mySpiralArrays / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		
	}
	glPopMatrix();
}
 */

/*
void RunAndJump::tickGarbageCollector() {
	//float percentOfMaxSpeed = myPlayerSpeed.x / myPlayerMaxSpeed;
	//float distanceFromPlayer = percentOfMaxSpeed * 20.0;
	//myGarbageCollectorPosition = Vector3DMake(myPlayerPosition.x - distanceFromPlayer, 0.0, 0.0);	 
	//myGarbageCollectorPosition = myPlayerPosition;
}
 */

/*
void RunAndJump::buildGarbageCollector() {
	
	 float linesPerHalf = 12.0f;
	 
	 myGarbageCollectorArrays = linesPerHalf * 2;
	 
	 myGarbageCollectorVertices = malloc(myGarbageCollectorArrays * sizeof(GLfloat) * 3);
	 
	 GLfloat x,y,z,angle,r;
	 int c;
	 z = 0.0f;
	 c = 0;
	 r = 5.0;
	 
	 while (c < (myGarbageCollectorArrays)) {
	 angle += (M_PI / linesPerHalf);
	 x = r * sin(angle);
	 y = r * cos(angle);
	 myGarbageCollectorVertices[c++] = x;
	 myGarbageCollectorVertices[c++] = y;
	 myGarbageCollectorVertices[c++] = z;
	 x = r * sin(angle + M_PI);
	 y = r * cos(angle + M_PI);		
	 myGarbageCollectorVertices[c++] = x;
	 myGarbageCollectorVertices[c++] = y;
	 myGarbageCollectorVertices[c++] = z;
	 }
	 
}
 */

/*
void RunAndJump::drawGarbageCollector() {	
	glPushMatrix();
	{
		glTranslatef(myGarbageCollectorPosition.x, myGarbageCollectorPosition.y, myGarbageCollectorPosition.z);
		//glRotatef(90.0, 0.0f, 1.0f, 0.0f);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, myGarbageCollectorVertices);
		glDrawArrays(GL_LINES, 0, myGarbageCollectorArrays);
		glDisableClientState(GL_VERTEX_ARRAY);
		
	}
	glPopMatrix();
}
*/

/*
void RunAndJump::tickSomething() {
}
*/

/*
void RunAndJump::buildSomething() {
	
	 int segments = 6;
	 GLfloat width = 10;
	 GLfloat height = 10;
	 
	 mySomethingArrays = segments;
	 mySomethingVertices = malloc(mySomethingArrays * sizeof(GLfloat) * 2);
	 
	 int count=0;
	 for (GLfloat i = 0; i < 360.0f; i+=(360.0f/segments))
	 {
	 mySomethingVertices[count++] = (cos(DEGREES_TO_RADIANS(i))*width);
	 mySomethingVertices[count++] = (sin(DEGREES_TO_RADIANS(i))*height);
	 }
	 
}
 */

/*
void RunAndJump::drawSomething() {
	
	 glPushMatrix();
	 {
	 glRotatef(90.0, 0.0, 1.0, 0.0);
	 glEnableClientState(GL_VERTEX_ARRAY);
	 glVertexPointer (2, GL_FLOAT , 0, mySomethingVertices);
	 glDrawArrays (GL_LINE_LOOP, 0, mySomethingArrays);
	 }
	 glPopMatrix();
	 
}
 */


/*
//returns a random float between 0 and 1
float RunAndJump::randf()
{
    //random hack since no floating point random function
    //optimize later
    return (lrand48() % 255) / 255.f;
}
*/

/*
void RunAndJump::reset_vertex(int idx) {
    int i = idx * 3;
    vertices[i + 0] = generator[idx].x;
    vertices[i + 1] = generator[idx].y;
    vertices[i + 2] = generator[idx].z;
}


void RunAndJump::random_velocity(int idx) {

	velocity[idx].x = 0.5 - randf();
	velocity[idx].y = -(randf() * 5.0);
    velocity[idx].z = 0.5 - randf();
}

void RunAndJump::reset_particle(int idx) {

	if (false) {
		generator[idx].x = myPlayerPlatformIntersection.x;
		generator[idx].y = myPlayerPlatformIntersection.y;
		generator[idx].z = myPlayerPlatformIntersection.z;
	} else {
		
		generator[idx].x = myPlayerPosition.x;
		generator[idx].y = myPlayerPosition.y;
		generator[idx].z = myPlayerPosition.z;
		
		//generator[idx].x = myPlayerPosition.x + (randf() * 4.0) - 2.0;
		//generator[idx].y = myPlayerPosition.y + (randf() * 4.0) - 2.0;
		//generator[idx].z = myPlayerPosition.z + (randf() * 4.0) - 2.0;
		
		//LOGV("RES 1: %f %f %f\n", mySpiralVertices[0], mySpiralVertices[1], mySpiralVertices[2]);

	}
	
    reset_vertex(idx);
    random_velocity(idx);
	reset_life(idx);
	
	//LOGV("RESET: %d %f %f\n", idx, vertices[idx * 3], myPlayerPosition.x);

}

void RunAndJump::update_vertex(int idx) {
	
    int i = idx * 3;
    vertices[i] += velocity[idx].x;
    vertices[i+1] += velocity[idx].y;
    vertices[i+2] += velocity[idx].z;
	//velocity[idx].y -= 0.0002 * fabs(myPlayerSpeed.y);
	//LOGV("%d %f %f %f\n", idx, vertices[idx * 3], life[idx], myPlayerPosition.x);
}
 */

/*
static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};
 */

/*
static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,1.0f,1.0f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},
	{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},
	{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f}
};

void RunAndJump::update_color(int idx) {
    int i = idx * 4;
	
	float distanceFromPlayer = myPlayerPosition.x - vertices[idx * 3];
	float percentOf = (distanceFromPlayer) / 40.0;
	int ii = (int)(percentOf * 12);
	if (ii > 11) {
		ii = 11;
	}
	colors[i+0] = ccolors[ii][0];
    colors[i+1] = ccolors[ii][1];
    colors[i+2] = ccolors[ii][2];
	colors[i+3] = 1.0; //i / (float)11.0;	
}

void RunAndJump::reset_life(int i) {
	life[i] = 1.0;
}

void RunAndJump::buildFountain() {	
	srand48(time(NULL));
	
	int i = 0;
    for(i=0;i<NUM_PARTICLES;i++) {
        elements[i] = i;
    }
	
	for(i=0;i<NUM_PARTICLES;i++) {
		reset_particle(i);
		life[i] -= (float)i / (float)NUM_PARTICLES;
	}	
}


void RunAndJump::tickFountain() {	
	int i = 0; //particle index
    for(i=0;i<NUM_PARTICLES;i++) {
        life[i] -= 0.1;
        if(life[i] <= 0.0) {
            reset_particle(i);
        } else {
			update_color(i);
			update_vertex(i);
		}
    }
}
*/

/*
void RunAndJump::drawFountain() {

	if (false) {
	
		//GLfloat points [ ] = { myPlayerPosition.x, myPlayerPosition.y, myPlayerPosition.z };

		bindTexture(myTreeTextures[0]);
		glEnable(GL_POINT_SPRITE_OES);
		glPointSize(100.0);

		glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
		//glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_FALSE);

		glEnableClientState(GL_VERTEX_ARRAY); 
		glVertexPointer(3, GL_FLOAT, 0, vertices); 
		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

		//glColor4f(1.0, 1.0, 1.0, 1.0);

		glDisable(GL_POINT_SPRITE_OES);
		unbindTexture(myGroundTexture);
	 
	} else {
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_FLOAT, 0, colors);
		glPointSize(7.0);
		glDrawElements(GL_POINTS, NUM_PARTICLES, GL_UNSIGNED_SHORT, elements);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
}
 */

/*
const GLfloat RunAndJump::mySkyBoxVertices[108] = {
	-1.0, 1.0, -1.0, // top-upper-right
	1.0, 1.0, 1.0,
	1.0, 1.0, -1.0,
	
	-1.0, 1.0, -1.0, // top-lower-left
	-1.0, 1.0, 1.0,
	1.0, 1.0, 1.0,
	
	-1.0, 1.0, 1.0, // front-upper-right
	1.0, -1.0, 1.0,
	1.0, 1.0, 1.0,
	
	-1.0, 1.0, 1.0, // front-lower-left
	-1.0, -1.0, 1.0,
	1.0, -1.0, 1.0,
	
	-1.0, -1.0, 1.0, // bottom-upper-right
	1.0, -1.0, -1.0,
	1.0, -1.0, 1.0,
	
	-1.0, -1.0, 1.0, // bottom-lower-left
	-1.0, -1.0, -1.0,
	1.0, -1.0, -1.0,
	
	-1.0, -1.0, -1.0, // back-upper-right
	1.0, 1.0, -1.0,
	1.0, -1.0, -1.0,
	
	-1.0, -1.0, -1.0, // back-lower-left
	-1.0, 1.0, -1.0,
	1.0, 1.0, -1.0,
	
	-1.0, 1.0, -1.0, // left-upper-right
	-1.0, -1.0, 1.0,
	-1.0, 1.0, 1.0,
	
	-1.0, 1.0, -1.0, // left-lower-left
	-1.0, -1.0, -1.0,
	-1.0, -1.0, 1.0,
	
	1.0, 1.0, 1.0, // right-upper-right
	1.0, -1.0, -1.0,
	1.0, 1.0, -1.0,
	
	1.0, 1.0, 1.0, // right-lower-left
	1.0, -1.0, 1.0,
	1.0, -1.0, -1.0,
};


const GLfloat RunAndJump::cubeTextureCoords[72] = {
	0.0f, 1.0, // top-upper-right
	1.0, 0.0f,
	0.0f, 0.0f,
	
	0.0f, 1.0, // top-lower-left
	1.0, 1.0,
	1.0, 0.0f,
	
	0.0f, 0.0f, // front-upper-right
	1.0, 1.0,
	1.0, 0.0f,
	
	0.0f, 0.0f, // front-lower-left
	0.0f, 1.0,
	1.0, 1.0,
	
	0.0f, 0.0f, // bottom-upper-right
	1.0, 1.0,
	1.0, 0.0f,
	
	0.0f, 0.0f, // bottom-lower-left
	0.0f, 1.0,
	1.0, 1.0,
	
	0.0f, 1.0, // back-upper-right
	1.0, 0.0f,
	0.0f, 0.0f,
	
	0.0f, 1.0, // back-lower-left
	1.0, 1.0,
	1.0, 0.0f,
	
	0.0f, 0.0f, // left-upper-right
	1.0, 1.0,
	1.0, 0.0f,
	
	0.0f, 0.0f, // left-lower-left
	0.0f, 1.0,
	1.0, 1.0,
	
	1.0, 0.0f, // right-upper-right
	0.0f, 1.0,
	1.0, 1.0,
	
	1.0, 0.0f, // right-lower-left
	0.0f, 0.0f,
	0.0f, 1.0,
};


void RunAndJump::buildSkyBox() {
	mySkyBoxRotation = 0.0;
}


void RunAndJump::tickSkyBox() {
}


void RunAndJump::bindTexture(GLuint texture) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
}


void RunAndJump::unbindTexture(GLuint texture) {
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}


void RunAndJump::drawSkyBox() {
	glPushMatrix();
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTranslatef(myPlayerPosition.x, 0.0, 0.0);
		mySkyBoxRotation += 0.11;
		glScalef(4000.0, 4000.0, 4000.0);
		glRotatef(mySkyBoxRotation, 0.0, 1.0, 0.0);
		glVertexPointer(3, GL_FLOAT, 0, mySkyBoxVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, cubeTextureCoords);
		for (int i=0; i<6; i++) {
			glBindTexture(GL_TEXTURE_2D, mySkyBoxTextures[i]);
			glDrawArrays(GL_TRIANGLES, i * 6, 6);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		
	}
	glPopMatrix();
}

*/
