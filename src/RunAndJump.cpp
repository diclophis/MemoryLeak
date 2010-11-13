//
//  RunAndJump.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


#include "MemoryLeak.h"
#include "Model.h"
#include "Interpretator.h"
#include "Engine.h"
#include "MachineGun.h"
#include "RunAndJump.h"


RunAndJump::~RunAndJump() {
	LOGV("dealloc GameController\n");
}


void RunAndJump::hitTest(float x, float y, int hitState) {
	switch (hitState) {
		case 0:
			playerStartedJumping();
			break;
		case 2:
			playerStoppedJumping();
			break;
		default:
			break;
	}
}


void RunAndJump::tickCamera() {
	myCameraTarget = Vector3DMake(myPlayerPosition.x + 3.25, myPlayerPosition.y, myPlayerPosition.z);
	myCameraPosition = Vector3DMake(myPlayerPosition.x + 3.25, myPlayerPosition.y + 1.5, myPlayerPosition.z + 60.0);
}


void RunAndJump::build() {
	myPlayerSpeed = Vector3DMake(0.5, 0.0, 0.0);
	
	myGravity = 0.0;
	myPlayerJumpSpeed = 0.0;
	mySimulationTime = 0.0;
	myGameStarted = false;
	myGameSpeed = 1;
	myDeltaTime = 0.0;
	
	myPlayerAcceleration = Vector3DMake(0.0, 0.0, 0.0);
	myPlayerJumping = false;
	myPlayerLastJump = -1.0;
	myPlayerLastEnd =  -1.0;
	myPlayerOnPlatform = false;
	myPlayerCanDoubleJump = false;
	
	int m_PostProcessFlags =  aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_Triangulate;
	
	importer.ReadFile("0", m_PostProcessFlags);	
	foofoo *boxFoo = Model::GetFoo(importer.GetScene());
	importer.FreeScene();
	
	importer.ReadFile("1", m_PostProcessFlags);
	foofoo *ringFoo = Model::GetFoo(importer.GetScene());
	importer.FreeScene();
	
	importer.ReadFile("0", m_PostProcessFlags);	
	foofoo *playerFoo = Model::GetFoo(importer.GetScene());
	importer.FreeScene();
		
	m_Gun = new MachineGun(boxFoo);

	m_SkyBox = new Model(boxFoo);
	m_SkyBox->SetPosition(0.0, 0.0, 0.0);
	m_SkyBox->SetScale(300.0, 300.0, 300.0);
	
	myPlayerHeight = 1.0;
	m_Player = new Model(playerFoo);
	m_Player->SetScale(1.0, 1.0, 1.0);
	buildPlatforms();

	mySegmentCount = 16 * 15;
	for (unsigned int i=0; i<mySegmentCount; i++) {
		mySegments.push_back(new Model(boxFoo));
		mySegments[i]->SetScale(1.0, 1.0, 1.0);
		mySegments[i]->SetPosition(0.0, 0.0, 0.0);
		mySegments[i]->SetRotation(0.0, 0.0, 0.0);
	}
	
	myTerrainCount = 10;
	myTerrainHeight = 5.0;
	for (unsigned int i=0; i<myTerrainCount; i++) {
		myTerrains.push_back(new Model(ringFoo));
		myTerrains[i]->SetScale(1.5, 1.5, 1.5);
		myTerrains[i]->SetPosition(0.0, 0.0, 0.0);
		myTerrains[i]->SetRotation(0.0, 90.0, 0.0);
	}
	
	myPlayerPosition = Vector3DMake(2.0, myPlatforms[0].position.y + 5.0, 0.0);
	m_Gun->SetVelocity(myPlayerSpeed.x, myPlayerSpeed.y, myPlayerSpeed.z);
	myCameraTarget = Vector3DMake(myPlayerPosition.x + 3.0, myPlayerPosition.y, -20.0);
	myCameraPosition = Vector3DMake(myPlayerPosition.x + 3.0, myPlayerPosition.y, 20.0);
}


int RunAndJump::simulate() {
	tickCamera();
	m_SkyBox->SetPosition(myPlayerPosition.x, 0.0, 0.0);
	m_SkyBox->SetRotation(0.0, ((1.1 * mySimulationTime)), 0.0);
	tickPlatform();
	tickPlayer();
	for (unsigned int i=0; i<myTerrainCount; i++) {
		myTerrains[i]->SetRotation(0.0, mySimulationTime * 500.0, 0.0);
	}

  m_Gun->Tick(myDeltaTime);
	
	m_Gun->SetPosition(myPlayerPosition.x, myPlayerPosition.y + myPlayerHeight, myPlayerPosition.z);
	m_Gun->SetVelocity(myPlayerSpeed.x, myPlayerSpeed.y, myPlayerSpeed.z);

  if (myPlayerPosition.y < -2.0) {
    return 0;
  } else {
	  return 1;
  }
}


void RunAndJump::render() {	
	glBindTexture(GL_TEXTURE_2D, textures->at(2));
	m_SkyBox->render(0);
	
	glBindTexture(GL_TEXTURE_2D, textures->at(4));
	for (unsigned int i=0; i<myTerrainCount; i++) {
		myTerrains[i]->render(0);
	}
	
	glBindTexture(GL_TEXTURE_2D, textures->at(1));
	for (unsigned int i=mySegmentCount; i>0; i--) {
		mySegments[i - 1]->render(0);
	}	

	glBindTexture(GL_TEXTURE_2D, textures->at(3));
	m_Gun->render();
	
	glBindTexture(GL_TEXTURE_2D, textures->at(0));
	m_Player->render(0);
}


void RunAndJump::playerStartedJumping() {
  if (myPlayerCanDoubleJump) {
    myPlayerJumping = true;
    myPlayerLastJump = mySimulationTime;
  }
}


void RunAndJump::playerStoppedJumping() {
	myPlayerLastEnd = mySimulationTime;
}


void RunAndJump::tickPlayer() {

	float timeSinceStarted;
	float timeSinceEnded;
		
	Vector3D oldPosition = myPlayerPosition;
  
	timeSinceStarted = (mySimulationTime - myPlayerLastJump);
	timeSinceEnded = (mySimulationTime - myPlayerLastEnd);
	
	myGravity = -50.0 * myDeltaTime;
	myPlayerJumpSpeed = 12.0;

	myPlayerAcceleration.y = myGravity;
	if (myPlayerJumping && myPlayerLastEnd > myPlayerLastJump) {
		myPlayerJumping = false;
	} else if (myPlayerJumping && timeSinceStarted < 0.33) {
		if (myPlayerCanDoubleJump) {
			myPlayerCanDoubleJump = false;
			myPlayerSpeed.y = 4.0;
			myPlayerSpeed.x += 3.0;
		}
		
		myPlayerAcceleration.y = -(myGravity * 0.95);
	} else {
		myPlayerCanDoubleJump = true;
		myPlayerJumping = false;
	}
	
	myPlayerSpeed = Vector3DAdd(myPlayerSpeed, Vector3DMake(myPlayerAcceleration.x, myPlayerAcceleration.y, myPlayerAcceleration.z));

	if (myPlayerSpeed.x > 12.0) {
		myPlayerSpeed.x = 12.0;
	} else if (myPlayerSpeed.x < -12.0) {
	  myPlayerSpeed.x = -12.0;
	}
	
	myPlayerPosition = Vector3DAdd(myPlayerPosition, Vector3DMake(myPlayerSpeed.x * myDeltaTime, myPlayerSpeed.y * myDeltaTime, myPlayerSpeed.z * myDeltaTime));
	m_Player->SetPosition(myPlayerPosition.x, myPlayerPosition.y + myPlayerHeight, 0.0);
	m_Player->SetRotation(0.0, 90.0, 0.0);
}


void RunAndJump::buildPlatforms() {	
	myPlatformCount = 200;
	Vector3D lastPlatformPosition = Vector3DMake(0.0, 0.0, 0.0);

	int step;
	float randomY;
	float randomA;
	float randomL;
	int length;
	
	lastPlatformPosition.x = 0.0;
	lastPlatformPosition.y = 0.0;

	for (int i=0; i<myPlatformCount; i++) {

		randomY = 0.0;
		randomA = (randf() * 2.0) + 1.0;
		randomL = 25.0 + (10.0 * randf());
		
		length = randomL;
		step = 1.0;
		Platform f;
		f.position = Vector3DMake(lastPlatformPosition.x, randomY, 0.0);
		f.length = length;
		f.amplitude = randomA;
		
		f.step = step;
		f.angular_frequency = randf() * 0.15;
		f.phase = 0.0;
		lastPlatformPosition = f.position;
		lastPlatformPosition.x += length + ((randf() > 0.99) ? 5.0 : 0.0);
		myPlatforms.push_back(f);
	}
}


void RunAndJump::tickPlatform() {
	mySegmentIndex = 0;
	myTerrainIndex = 0;
	myPlayerOnPlatform = false;
	myPlayerPlatformCorrection = Vector3DMake(0.0, 0.0, 0.0);
	iteratePlatform(0);
}


void RunAndJump::iteratePlatform(int operation) {
  bool tickedWall = false;
  float lastHeight = 0.0;
	
	//std::vector<Platform>::iterator it;
	
	//for (it=myPlatforms.begin() ; it < myPlatforms.end(); it++ ) {
	for (unsigned int f=0; f<myPlatformCount; f++) {
		Platform platform = myPlatforms[f];

		float x = platform.position.x;
		if ((x < (myPlayerPosition.x + 50.0)) &&  (x > (myPlayerPosition.x - 50.0))) {
		
			for (int i = (int)platform.position.x; i < platform.position.x + platform.length; i += platform.step) {

				if ((i < (myPlayerPosition.x + 22.0)) && (i > (myPlayerPosition.x - 5.0))) {

					float beginX = i;
					float endX = i + platform.step;

					
					float phaseX = platform.phase;
					float phaseY = phaseX;
					
					float beginY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (beginX + phaseX));
					float endY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (endX + phaseY));
					
					float ringY = (0.33 * fastSinf(0.5 * beginX)); 
					
					platform.last_angular_frequency = platform.angular_frequency;

          if (!tickedWall) {
            if (platform.position.x > myPlayerPosition.x) {
              if (myPlayerPosition.x > (beginX - 0.25)) {
                if (myPlayerPosition.y < beginY) {
                  myPlayerSpeed.x = -(0.25 * myPlayerSpeed.x) - 2.0;
                }
              }
              tickedWall = true;
            }
          }
          if (mySegmentIndex < mySegmentCount) {
            for (unsigned int ijk=0; ijk < 15; ijk++) {
              mySegments[mySegmentIndex]->SetPosition(beginX + (platform.step * 0.5), beginY - ijk + myPlayerHeight, 0.0);
              mySegmentIndex++;
            }
          }
          
          if (myTerrainIndex < myTerrainCount) {
            if (beginX < (2.0 + platform.position.x) || beginX > (platform.position.x + platform.length - 2.0)) {
              myTerrains[myTerrainIndex]->SetPosition(beginX + (platform.step * 0.5), ringY + 6.0, 0.0);
              myTerrainIndex++;
            }
          }

          if (myPlayerSpeed.y < 0.0) {
          tickPlatformSegment(beginX, beginY, endX, endY);
          }
				}
			}
		}
    lastHeight = platform.position.y;
	}
}


void RunAndJump::tickPlatformSegment(float beginX, float beginY, float endX, float endY) {
	if (myPlayerPosition.x >= beginX && myPlayerPosition.x <= endX) {
		if (myPlayerPosition.y < beginY + myPlayerHeight || myPlayerPosition.y < endY + myPlayerHeight) {
			myPlayerOnPlatform = true;
			myPlayerPosition.y = beginY + myPlayerHeight;
			myPlayerSpeed.y = 0.0;
		}
	}
}
