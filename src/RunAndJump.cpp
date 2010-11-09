//
//  RunAndJump.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


#include "MemoryLeak.h"
#include "Model.h"
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
  //75
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 15.0, myPlayerPosition.y - fastSinf(mySimulationTime * 0.01), -10.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 8.0, myCameraPosition.y + (0.01 * ((myPlayerPosition.y + 5.0) - myCameraPosition.y)), 10.0);

	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 30.0, 0.0, 0.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 2.0, myPlayerPosition.y + 1.5, 0.0);
  //

  //10
	
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 3.0 + (myPlayerSpeed.x * 2.0), myPlayerPosition.y, -20.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x + 0.0 - (myPlayerSpeed.x * 2.0), 4.0 + (myPlayerSpeed.x * 0.0), 15.0 + (myPlayerSpeed.x * 2.0));
	
	/*
	float lag = 1.0 + (myPlayerSpeed.x * 0.0001);
	myCameraSpeed = Vector3DMake(myPlayerSpeed.x * lag, myPlayerSpeed.y * lag, 0.0);
	myCameraTarget = Vector3DAdd(myCameraTarget, Vector3DMake(myCameraSpeed.x * myDeltaTime, myCameraSpeed.y * myDeltaTime, myCameraSpeed.z * myDeltaTime));
	myCameraTarget.y = myPlayerPosition.y - 10.0;
	myCameraPosition = Vector3DMake(myPlayerPosition.x - 3.0, myPlayerPosition.y + 40.0, 50.0);
	 */
	
	
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 2.0, myPlayerPosition.y, 0.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 2.0, myPlayerPosition.y, 50.0);
	
	//myCameraTarget = Vector3DMake(myPlayerPosition.x + 10.0, 0.0, -1.0);
	//myCameraPosition = Vector3DMake(myPlayerPosition.x - 10.0, 10.0, 2.0);
  //
	
	myCameraTarget = Vector3DMake(myPlayerPosition.x + 3.25, myPlayerPosition.y, myPlayerPosition.z);
	myCameraPosition = Vector3DMake(myPlayerPosition.x + 3.25, myPlayerPosition.y + 1.5, myPlayerPosition.z + 60.0);
}


void RunAndJump::build() {

  LOGV("fooo\n\n\n\n");

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

	mySegmentCount = 12 * 4;
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
	//m_Player->SetRotation(0.0, ((0.1 * mySimulationTime)), 0.0);
	tickPlatform();
	tickPlayer();
	for (unsigned int i=0; i<myTerrainCount; i++) {
	//	if (myTerrains[i]->GetPosition()[0] < myPlayerPosition.x - (20.0 * 2.0)) {
	//		myTerrains[i]->SetPosition(myTerrains[i]->GetPosition()[0] + (20.0 * myTerrainCount), myTerrainHeight, 0.0);
	//	}
		myTerrains[i]->SetRotation(0.0, mySimulationTime * 500.0, 0.0);
	}
	
	m_Gun->Tick(myDeltaTime);
	
	m_Gun->SetPosition(myPlayerPosition.x, myPlayerPosition.y + myPlayerHeight, myPlayerPosition.z);
	m_Gun->SetVelocity(myPlayerSpeed.x, myPlayerSpeed.y, myPlayerSpeed.z);

  if (myPlayerPosition.y < -5.0) {
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
	//if (myPlayerOnPlatform) {
		myPlayerOnPlatform = false;
		myPlayerJumping = true;
		myPlayerLastJump = mySimulationTime;
	//}	
}


void RunAndJump::playerStoppedJumping() {
	myPlayerLastEnd = mySimulationTime;
}


void RunAndJump::tickPlayer() {

	bool myPlayerFalling = true;
	float timeSinceStarted;
	float timeSinceEnded;
	
	//myPlayerAcceleration.y = myGravity;
	
	Vector3D oldPosition = myPlayerPosition;
  
	timeSinceStarted = (mySimulationTime - myPlayerLastJump);
	timeSinceEnded = (mySimulationTime - myPlayerLastEnd);
	
	myGravity = -20.0 * myDeltaTime;
	myPlayerJumpSpeed = 12.0;

  //if (timeSinceStarted < 100.0 || timeSinceEnded < 100.0) {
  //  LOGV("%f %f %f\n", (myPlayerSpeed.y), timeSinceStarted, timeSinceEnded);
  //}
	
	myPlayerAcceleration.y = myGravity;


	bool get = false;	
	if (myPlayerJumping && myPlayerLastJump >= 0.0 && timeSinceStarted < 0.01 && timeSinceStarted >= 0.0) {
		myPlayerSpeed.y = myPlayerJumpSpeed;
		myPlayerSpeed.x += 2.0;
		//LOGV("da %f\n", myPlayerAcceleration.y);

		myPlayerFalling = false;
		myPlayerJumping = false;
		
	/*
	if (myPlayerJumping && (timeSinceEnded > timeSinceStarted)) {
	LOGV("da\n");
	myPlayerFalling = false;
	myPlayerAcceleration.y = myPlayerJumpSpeed * myDeltaTime;
	myPlayerAcceleration.x = 0.05;
	get = true;
	} else if (myPlayerJumping) {
	LOGV("la %d\n", myPlayerOnPlatform);
	myPlayerFalling = true;
	myPlayerOnPlatform = false;
	get = true;
	} else {


	//LOGV("fo %f %d\n", timeSinceStarted, myPlayerOnPlatform);

	// myPlayerPosition.y += 2.5;
	//myPlayerSpeed.y = 10.0;
	myPlayerAcceleration.y = 50000.0;
	//myPlayerSpeed.x += 1.0;
	myPlayerAcceleration.x = 0.1;

	myPlayerJumping = true;
	myPlayerFalling = false;
	myPlayerOnPlatform = false;
	get = true;
	}
	 */
	}
	
	if (myPlayerFalling) {
		//myPlayerAcceleration.y = myGravity;
		myPlayerAcceleration.x = 0.00001;
		myPlayerLastEnd = -1.0;
		myPlayerLastJump = -1.0;
		myPlayerJumping = false;
		/*
		if (myPlayerOnPlatform) {
			//if (get) {
			//LOGV("fark\n");
			//}
			//myPlayerSpeed.y = 0.0;
			if (myPlayerSpeed.y < 0.0) {
				myPlayerSpeed.y = 0.0;
				float off = myPlayerPosition.y - myPlayerPlatformCorrection.y;
				
				//if (off < 0.0) {
				myPlayerPosition.y -= (off * 0.01);
				//}
			}

			//myPlayerPosition.y = myPlayerPlatformIntersection.y;
		}
		 */
	}
	
	myPlayerSpeed = Vector3DAdd(myPlayerSpeed, Vector3DMake(myPlayerAcceleration.x, myPlayerAcceleration.y, myPlayerAcceleration.z));


  if (myPlayerSpeed.x > 5.0) {
    myPlayerSpeed.x = 5.0;
  } else if (myPlayerSpeed.x < -5.0) {
    myPlayerSpeed.x = -5.0;
  }

//if (myPlayerSpeed.y > 0.5) {
//  myPlayerSpeed.y = 0.5;
//} else if (myPlayerSpeed.y < -0.5) {
//  myPlayerSpeed.y = -0.5;
//}
//if (get) {
//  LOGV("the fuck: %f %f\n", myPlayerPosition.y, myPlayerSpeed.y);
//}

	
	myPlayerPosition = Vector3DAdd(myPlayerPosition, Vector3DMake(myPlayerSpeed.x * myDeltaTime, myPlayerSpeed.y * myDeltaTime, myPlayerSpeed.z * myDeltaTime));

//if (get) {
//  LOGV("the fuck: %f %f\n", myPlayerPosition.y, myPlayerSpeed.y);
//}

//if ((myPlayerSpeed.y > 0.0) || ((int)(mySimulationTime * 10000.0) % 2000 < 1)) {
//LOGV("foo: %f %f\n", myPlayerSpeed.y, myPlayerAcceleration.y);
//LOGV("foo: %d %f %f %f %f %f %f %f\n", myPlayerOnPlatform, myPlayerPosition.y, myPlayerPlatformIntersection.y, myPlayerSpeed.y, myPlayerAcceleration.y, myDeltaTime, myGravity, myPlayerJumpSpeed);
//}


/*
if (myPlayerPosition.y < -5.0) {
for (unsigned int i=0; i<mySegmentCount; i++) {
mySegments[i]->SetPosition(mySegments[i]->GetPosition()[0] + randf(), mySegments[i]->GetPosition()[1] + randf(), + randf());
}
myPlayerSpeed.x = 0.0;
}
 */
 
	


	
	m_Player->SetPosition(myPlayerPosition.x, myPlayerPosition.y + myPlayerHeight, 0.0);
	m_Player->SetRotation(0.0, 0.0, 0);
}


void RunAndJump::buildPlatforms() {	
	myPlatformCount = 100;
	Vector3D lastPlatformPosition = Vector3DMake(0.0, 0.0, 0.0);

	int step;
	float randomY;
	float randomA;
	float randomL;
	int length;
	
	lastPlatformPosition.x = 0.0;
	lastPlatformPosition.y = 0.0;

	for (int i=0; i<myPlatformCount; i++) {

		randomY = 0.0; //((random() / (float)RAND_MAX) * 4.0) - 0.25;
		randomA = (randf() * 1.5) + 1.0;
		randomL = ((randf() * 25.0) + 10.0);
		
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
		lastPlatformPosition.x += length + randf() + 4.0;
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
	
	std::vector<Platform>::iterator it;
	
	for (it=myPlatforms.begin() ; it < myPlatforms.end(); it++ ) {
		Platform platform = *it;

		float x = platform.position.x;
		if ((x < (myPlayerPosition.x + 200.0)) &&  (x > (myPlayerPosition.x - 200.0))) {
		
			for (int i = (int)platform.position.x; i < platform.position.x + platform.length; i += platform.step) {

				if ((i < (myPlayerPosition.x + 9.0)) && (i > (myPlayerPosition.x - 3.0))) {

					float beginX = i;
					float endX = i + platform.step;

					
					float phaseX = platform.phase;
					float phaseY = phaseX;
					
					float beginY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (beginX + phaseX));
					float endY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (endX + phaseY));
					
					float ringY = (1.5 * fastSinf(0.5 * beginX)); 
					
					platform.last_angular_frequency = platform.angular_frequency;

          if (!tickedWall) {
            if (platform.position.x > myPlayerPosition.x) {
              if (myPlayerPosition.x > (beginX - 0.25)) {
                if (myPlayerPosition.y < platform.position.y) {
                  myPlayerSpeed.x = 0.0;
                }
              }
              tickedWall = true;
            }
          }
					//LOGV("the fuck: %d", mySegments.size());
					switch (operation) {
						case 0:
							if (mySegmentIndex < mySegmentCount) {
								for (unsigned int ijk=0; ijk < 4; ijk++) {
									//mySegments[mySegmentIndex]->SetPosition(beginX + (platform.step * 0.5), beginY - 5.0 + myPlayerHeight, 0.0);
									mySegments[mySegmentIndex]->SetPosition(beginX + (platform.step * 0.5), beginY - ijk + myPlayerHeight, 0.0);

									mySegmentIndex++;
								}
							}
							
							if (myTerrainIndex < myTerrainCount) {
								myTerrains[myTerrainIndex]->SetPosition(beginX + (platform.step * 0.5), beginY + ringY + 4.0, 0.0);
								myTerrainIndex++;
							}

							if (myPlayerSpeed.y < 0.0) {
              tickPlatformSegment(beginX, beginY, endX, endY);
							}
							break;
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
	
	/*
		Vector3D p1 = Vector3DMake(beginX, beginY, 0.0);
		Vector3D p2 = Vector3DMake(endX, endY, 0.0);
		
		Vector3D pp1 = Vector3DAdd(myPlayerPosition, Vector3DMake(-0.5, 10.0, 0.0));
		Vector3D pp2 = Vector3DAdd(myPlayerPosition, Vector3DMake(-0.5, -10.0, 0.0));
		
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
			float distanceFromIntersection = myPlayerPosition.y - myPlayerPlatformIntersection.y;
			if (distanceFromIntersection < 0.05) {
				myPlayerPlatformCorrection.y = beginY + myPlayerHeight;
				if (myPlayerOnPlatform) {
				} else {
					LOGV("foo\n");
				}
				myPlayerOnPlatform = true;
			} else {
				//LOGV("%f\n", distanceFromIntersection);
			}
		}
	 */
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
