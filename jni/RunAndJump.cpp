//
//  RunAndJump.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


#include "importgl.h"
#include "OpenGLCommon.h"

#include "Engine.h"
#include "RunAndJump.h"



//static GLuint myFontTexture;


void RunAndJump::hitTest(float x, float y) {

}




RunAndJump::~RunAndJump() {
	LOGV("dealloc GameController\n");
	myPlayerManager.Release();
	if (myPlatforms) {
		free(myPlatforms);
	}
	
	if (mySkyBoxTextures) {
		free(mySkyBoxTextures);
	}
}


void RunAndJump::playerStartedJumping() {
	if (myGameStarted) {
		
		
		if (myPlayerCanDoubleJump) {
			myGameSpeed += 1;
			myPlayerCanDoubleJump = false;
			myPlayerLastJump = mySimulationTime;
		} else {
			//myPlayerNeedsTransform = true;
		}
		
		if (myPlayerOnPlatform) {
			myPlayerLastJump = mySimulationTime;
		}
		
	}
	
	//myPlayerSpeed.x = 0;

}


void RunAndJump::playerStoppedJumping() {
	myPlayerLastEnd = mySimulationTime;
}


//void RunAndJump::build(int width, int height, GLuint *textures, foo *playerFoo) {
void RunAndJump::build() {
	mySkyBoxHeight = 12.5;
	mySkyBox = mySkyBoxManager.Load(models[3], 1, textures[4]);
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
	mySkyBox->SetRotation(90.0);
	mySkyBox->SetScale(0.5, 0.25, 0.5);
}


int RunAndJump::simulate() {
	tickCamera();
	mySkyBoxManager.Update(myDeltaTime);
	return 1;
}


void RunAndJump::render() {
  drawCamera();	
	
	glEnable(GL_TEXTURE_2D);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  mySkyBoxManager.Render();
  glDisable(GL_BLEND);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	drawFont();

	glDisable(GL_TEXTURE_2D);
	
	
	
	/*
	drawCamera();
	
	
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//mySkyBoxManager.Render();
	//glDisable(GL_BLEND);
	
	//glEnable(GL_DEPTH_TEST);
	//myRaptorManager.Render();
	//myBarrelManager.Render();
	//glDisable(GL_DEPTH_TEST);
	
	drawPlatform();

	
	//drawFont();
	
	//glEnable(GL_DEPTH_TEST);
	//myPlayerManager.Render();
	//glDisable(GL_DEPTH_TEST);	
	
	//glDisable(GL_TEXTURE_2D);
	
	
	*/
	
	/*
	if (mySceneBuilt) {
		
		prepareFrame(screenWidth, screenHeight);
		
		glPushMatrix();
		{			
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			//glEnable(GL_BLEND);
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glRotatef(rotation, 0.0, 0.0, 1.0);
			
			drawCamera();
			drawFountain();
			drawPlayer();
			drawPlatform();
			drawSkyBox();
			drawFont();
			
			//glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
		}
		glPopMatrix();
	}
	 */
}




void RunAndJump::buildCamera() {
	myCameraTarget = Vector3DMake(0.0, 0.0, 0.0);
	myCameraPosition = Vector3DMake(-10.0, 0.0, 0.0);
	myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
}


void RunAndJump::tickCamera() {
	Vector3D desiredTarget;
	Vector3D desiredPosition;
	desiredTarget = Vector3DMake(0.0, 0.0, 0.0);
	desiredPosition = Vector3DMake(-80.0, 12.0, 0.0);
	myCameraTarget = desiredTarget;
	myCameraPosition = desiredPosition;
}


void RunAndJump::tickPlayer() {
	myPlayerJumpCycle = (int)(randf() * 3.0) + 13;
	
	//float timeSinceStarted = -[myPlayerLastJump timeIntervalSinceNow];
	//float timeSinceEnded = -[myPlayerLastEnd timeIntervalSinceNow];
	
	float timeSinceStarted;
	float timeSinceEnded;
	
	timeSinceStarted = (mySimulationTime - myPlayerLastJump);
	timeSinceEnded = (mySimulationTime - myPlayerLastJump);
	
	//NSLog("%f %f \n", timeSinceStarted, timeSinceEnded);
	
	bool myPlayerFalling = true;

	if (myPlayerNeedsTransform) {
		myPlayerNeedsTransform = false;
		if (myPlayerIsTransformed) {
			myPlayerMd2->SwitchCycle(myPlayerTransformUpCycle, 0.1, false, -1, myPlayerRunCycle);
		} else {
			myPlayerMd2->SwitchCycle(myPlayerTransformDownCycle, 0.1, false, 1, myPlayerTransformedCycle);
		}
		myPlayerIsTransformed = !myPlayerIsTransformed;
	} else {
	}

	myPlayerManager.Update(myDeltaTime);
	
	if (timeSinceStarted < 0.02 && timeSinceStarted > 0.0) {
		//myMd2->SwitchCycle(6, 0.001, false);
		myPlayerMd2->SwitchCycle(myPlayerJumpCycle, 0.01, false, -1, myPlayerRunCycle);
		myPlayerFalling = false;
		if (myPlayerJumping) {

			if (!timeSinceEnded) {
				myPlayerAcceleration.y = 120.0; //60
			} else {
				//myMd2->SwitchCycle(1, 0.01, false);
				myPlayerFalling = true;
			}
		} else {
			myPlayerAcceleration.x = 0.0; //4000.0;
			myPlayerSpeed.y = 700.0;
			myPlayerJumping = true;
		}
	}
	
	if (myPlayerFalling) {
		
		myPlayerJumping = false;
		myPlayerAcceleration.x = 0.0;
		myPlayerAcceleration.y = myGravity;
		
		if (myPlayerOnPlatform) {
			//myMd2->SwitchCycle(1, 0.0001, false);
			myPlayerSpeed.y = myPlayerPlatformCorrection.y;
		}
		
		if (myPlayerLastJump) {
			//[myPlayerLastJump release];
			//myPlayerLastJump = 0;
		}
		
		if (myPlayerLastEnd) {
			//[myPlayerLastEnd release];
			//myPlayerLastEnd = 0;
		}
	}
	
	if (myPlayerOnPlatform) {
		//myMd2->SwitchCycle(myPlayerRunCycle, 0.1, false);

		myPlayerCanDoubleJump = true;
		
		if (!myGameStarted) {
			//myPlayerSpeed.x = 0.0;
		}
	}
	
	myPlayerSpeed = Vector3DAdd(myPlayerSpeed, Vector3DMake(myPlayerAcceleration.x * myDeltaTime, myPlayerAcceleration.y * myDeltaTime, myPlayerAcceleration.z * myDeltaTime));
	
	//NSLog("%f\n", myPlayerSpeed.y);
	if (myPlayerSpeed.y < -450.0) {
		myPlayerSpeed.y = -450.0;
	}
	
	Vector3D oldPosition = myPlayerPosition;
	
	myPlayerPosition = Vector3DAdd(myPlayerPosition, Vector3DMake(myPlayerSpeed.x * myDeltaTime, myPlayerSpeed.y * myDeltaTime, myPlayerSpeed.z * myDeltaTime));
	
	
	if (myPlayerPlatformIntersection.y - myPlayerPosition.y > 200.0) {
		myPlayerBelowPlatform = true;
		myPlayerNeedsTransform = true;
	} else {
		myPlayerBelowPlatform = false;
	}
	
	//if (!myPlayerOnPlatform) {
	//	float slope = (myPlayerPosition.y - oldPosition.y) / (myPlayerPosition.x - oldPosition.x);
	//	myPlayerRotation = slope * 20.0;
	//}
}


/*
void RunAndJump::drawPlayer() {
	glPushMatrix();
	{
		glTranslatef(myPlayerPosition.x, myPlayerPosition.y + 1.5, myPlayerPosition.z);
		glRotatef(myPlayerRotation, 0.0, 0.0, 1.0);
		float scale = 0.5;
		glScalef(scale, scale, scale);
		bindTexture(myPlayerTexture);
		//Md2Manager::Render();
		myPlayerManager.Render();
		unbindTexture(myPlayerTexture);
	}
	glPopMatrix();
}
 */


void RunAndJump::buildPlatforms() {	
	myPlatformCount = 100;
	Vector3D lastPlatformPosition = Vector3DMake(0.0, 0.0, 0.0);
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));
	int step;
	float randomY;
	float randomA;
	float randomL;
	int length;

	lastPlatformPosition.x = -250.0;
	
	for (int i=0; i<myPlatformCount; i++) {
		randomY = lastPlatformPosition.y;
		while (fabs(lastPlatformPosition.y - randomY) < 70.0) {
			randomY = ((random() / (float)RAND_MAX) * 150.0);
		}
		randomA = ((random() / (float)RAND_MAX) * 10.0);
		randomL = 40.0 - (i * randf());
		
		if (randomA > 5.0) {
			step = 5;
		} else if (randomA > 3.0) {
			step = 25;
		} else if (randomA > 1.0) {
			step = 30;
		} else if (randomA > 0.5) {
			step = 35;
		} else {
			step = 40;
		}
		
		length = step * randomL;
		
		myPlatforms[i].position = Vector3DMake(lastPlatformPosition.x, randomY, 0.0);
		myPlatforms[i].length = length;
		myPlatforms[i].amplitude = randomA;
		
		myPlatforms[i].step = step;
		myPlatforms[i].angular_frequency = 0.030; //1 / (i + 0.000001) + 1.0;
		myPlatforms[i].phase = 0.0;
		lastPlatformPosition = myPlatforms[i].position;
		//LOGV("%f\n", logf((float)i + 1.1) * 20.0);
		//lastPlatformPosition.x += length + (logf((float)i + 1.1) * 50.0);
		lastPlatformPosition.x += (0.5 * length) + randf() * 600.0;
	}
}


void RunAndJump::tickPlatform() {
	myPlayerOnPlatform = false;
	myPlayerPlatformCorrection = Vector3DMake(0.0, 0.0, 0.0);
	iteratePlatform(0);
}


void RunAndJump::drawPlatform() {
	glBindTexture(GL_TEXTURE_2D, myGroundTexture);
	iteratePlatform(1);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void RunAndJump::iteratePlatform(int operation) {
	for (int j=0; j<myPlatformCount; j++) {
		Platform platform = myPlatforms[j];
		//if ((platform.position.x > (myPlayerPosition.x - platform.length - 10.0)) && (platform.position.x < (myPlayerPosition.x + platform.length + 10.0))) {
			for (float i = platform.position.x; i < platform.position.x + platform.length; i += platform.step) {
				if ((i < (myPlayerPosition.x + 1200.0)) && (i > (myPlayerPosition.x - 150.0))) {
					float beginX = i;
					float endX = i + platform.step;
					
					float phaseX = platform.phase;
					float phaseY = phaseX;
					
					if (false) {
						phaseX = mySimulationTime * 1000.0;//(myPlayerPosition.x - beginX); //platform.phase + (mySimulationTime * );
						phaseY = -phaseX; //platform.phase + (mySimulationTime * 1000.0);
					}
					
					float beginY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (beginX + phaseX));
					float endY = platform.position.y + platform.amplitude * fastSinf(platform.angular_frequency * (endX + phaseY));
					
					platform.last_angular_frequency = platform.angular_frequency;
					switch (operation) {
						case 0:
							tickPlatformSegment(beginX, beginY, endX, endY);
							break;
						case 1:
							drawPlatformSegment(platform.position.y, beginX, beginY, endX, endY);
							break;
					}
				}
			}
		//}
	}
}


/*
const GLfloat RunAndJump::myPlatformTextureCoords[6] = {
	0.0, 0.0, // top-upper-right
	1.0, 0.0,
	1.0, 1.0,
	//1.0, 1.0, // top-lower-left
	//0.0, 1.0,
	//0.0, 0.0,
};
 */


void RunAndJump::drawPlatformSegment(float baseY, float x1, float y1, float x2, float y2) {
	float beginX; float beginY; float endX; float endY;
	
	baseY -= 0.0;
	
	float platformRadius = 10.0;

	int total = 10;

	float deep = -((platformRadius * (float)total) * 0.5);
	
	int number = 6 * 3 * total;
	
	GLfloat platformVertices[number];
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
	glDrawArrays(GL_TRIANGLES, 0, 6 * total);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void RunAndJump::tickPlatformSegment(float beginX, float beginY, float endX, float endY) {
	if (!myPlayerJumping && !(myPlayerSpeed.y > 250.0)) {
	Vector3D p1 = Vector3DMake(beginX, beginY, 0.0);
	Vector3D p2 = Vector3DMake(endX, endY, 0.0);
	
	Vector3D pp1 = Vector3DAdd(myPlayerPosition, Vector3DMake(-0.0, 0.0, 0.0));
	Vector3D pp2 = Vector3DAdd(myPlayerPosition, Vector3DMake(-0.0, -7.5, 0.0));
	
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
			myPlayerRotation = slope * 70.0;
		//} else {
		//	myPlayerRotation = slope;
		//}
		float distanceFromIntersection = myPlayerPosition.y - 1.25 - myPlayerPlatformIntersection.y;
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
