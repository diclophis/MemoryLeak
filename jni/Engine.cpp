//
//  Engine.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//


#include <sstream>

#include "Engine.h"

namespace OpenSteer {
	bool updatePhaseActive = false;
	bool drawPhaseActive = false;
	bool enableAnnotation = false;
}


inline std::string Engine::stringify(double x) {
	std::ostringstream o;
	if (!(o << x))
		throw 987;
	return o.str();
}


Engine::Engine() {
	LOGV("alloc/init GameController\n");
}


Engine::~Engine() {
	LOGV("dealloc GameController\n");
	myTextures.clear();
}


/*
void Engine::playerStartedJumping() {
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
*/

/*
void Engine::playerStoppedJumping() {
	myPlayerLastEnd = mySimulationTime;
}
*/




int Engine::tick() {
	
	int gameState;
	
	for (int i=0; i<=myGameSpeed; i++) {
		mySimulationTime += myDeltaTime;
		if (mySceneBuilt) {
			gameState = simulate();
		}
		if (gameState == 0) {
			break;
		}
	}
	
	return gameState;
	
}

/*
int Engine::simulate() {
	return 0;
}
 */


void Engine::gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
	GLfloat xmin, xmax, ymin, ymax;

	ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustumx(
		(GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
		(GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
		(GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536)
	);
}


void Engine::prepareFrame(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.5, 0.6, 0.85, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(0.0 + (mySimulationTime * 20.0), (float) width / (float) height, 0.1, 200.0);
	//gluPerspective(25.0, (float) width / (float) height, 0.1, 50.0);
	//gluPerspective(120.0, (float) width / (float) height, 21.0, 70.0);
	gluPerspective(90.0, (float) width / (float) height, 0.1, 200.0);

	//lower left corner at (left, bottom, -near) 
	//upper right corner at (right, top, -near).
	//glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far); 
	//glOrthof(-1.0, 1.0, 0.0, 1.0, -49.0, 49.0);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


void Engine::resizeScreen(int width, int height) {
	screenWidth = width;
	screenHeight = height;
}


void Engine::draw(float rotation) {

	if (mySceneBuilt) {

		prepareFrame(screenWidth, screenHeight);
		
		glPushMatrix();
		{			
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_DEPTH_TEST);
			glRotatef(rotation, 0.0, 0.0, 1.0);
			render();
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
		}
		glPopMatrix();
	}
}


void Engine::buildFont() {	
	
	m_charPixelWidth = m_animPixelWidth = CHAR_PIXEL_W;
	m_charPixelHeight = m_animPixelHeight = CHAR_PIXEL_H;
	
	//
	// Zero Z Coords in Geomtery Array.
	//
	//for(int i=0; i<(MAX_CHAR_BUFFER * ONE_CHAR_SIZE_V); i+=ONE_CHAR_SIZE_V)
	//{
	//	charGeomV[i+2] = charGeomV[i+5] = charGeomV[i+8] = charGeomV[i+11] = 0.0;
	//}
	
	for(int i=0; i<12; i++)
	{
		charGeomV[i] = 0.0;
	}
	
	// Pre-generate all possible texture coords
	for(int c=FONT_TEXTURE_ATLAS_WIDTH * FONT_TEXTURE_ATLAS_LINES; c>=0; c--)
	{
		int character = c * ONE_CHAR_SIZE_T;
		charTexCoords[character] = charTexCoords[character+4] = (c % FONT_TEXTURE_ATLAS_WIDTH) * CHAR_WIDTH;	
		charTexCoords[character+1] = charTexCoords[character+3] = 1.0 - (c / FONT_TEXTURE_ATLAS_WIDTH) * CHAR_HEIGHT;
		charTexCoords[character+2] = charTexCoords[character+6] = charTexCoords[character+0] + CHAR_WIDTH;
		charTexCoords[character+7] = charTexCoords[character+5] = 1.0 - (c / FONT_TEXTURE_ATLAS_WIDTH + 1) * CHAR_HEIGHT;
	}		
}


void Engine::tickFont() {
}


void Engine::drawFont() {
		
	float _scaleX = 1.0;
	float _scaleY = 1.0;

	if (m_animPixelWidth > m_charPixelWidth) {
		m_animPixelWidth /= 1.5;
	} else {
		m_animPixelWidth = m_charPixelWidth;
	}

	if (m_animPixelHeight > m_charPixelHeight) {
		m_animPixelHeight /= 1.5;
	} else {
		m_animPixelHeight = m_charPixelHeight;
	}

	m_ntextWidth = screenWidth / m_animPixelWidth;
	m_ntextHeight = screenHeight / m_animPixelHeight;
	m_fCharacterWidth = 1.0 / m_ntextWidth;
	m_fCharacterHeight = 1.0 / m_ntextHeight;


	bindTexture(myTextures[1]);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();

	glOrthof(0, _scaleX, 0, _scaleY, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();


	float x;
	std::string fps;

	if (myGameStarted) {
		x = 0.05;
		fps = "S:" + stringify((int)myPlayerSpeed.x) + " D:" + stringify((int)myPlayerPosition.x);
	} else {
		//x = 0.1 + m_fCharacterWidth * fastSinf(mySimulationTime);
		x = 0.0;
		fps = "Escape from Raptor Island";
	}
	 
	float y = 0.875;
	
	for (int i=0; i<fps.length(); i++) {
		
		int c = fps.at(i);

		if (c == ' ') {
			if (myGameStarted) {
				//x = -0.025;
				x = 0.0;
			} else {
				//x = -m_fCharacterWidth * fastSinf(mySimulationTime + (float)i);
				x = -m_fCharacterWidth;
				
			}
			y -= 0.055;
		}
		
		c -= ' ';

		m_nCurrentChar = 0;

		
		// TexCoords
		int offsetT = m_nCurrentChar - (m_nCurrentChar / 3);	// 12 / 3 = 4 So 12 - 4 = 8
		memcpy(&charGeomT[offsetT], &charTexCoords[c * ONE_CHAR_SIZE_T], ONE_CHAR_SIZE_T * sizeof(GLfloat));

		
		
		// Vertex Xs
		charGeomV[m_nCurrentChar + 0] = charGeomV[m_nCurrentChar + 6] = x;
		charGeomV[m_nCurrentChar + 3] = charGeomV[m_nCurrentChar + 9] = x + m_fCharacterWidth;

		// Vertex Ys
		charGeomV[m_nCurrentChar + 1] = charGeomV[m_nCurrentChar + 4] = y;
		charGeomV[m_nCurrentChar + 10] = charGeomV[m_nCurrentChar + 7] = y + m_fCharacterHeight;
		charGeomV[m_nCurrentChar + 2] = charGeomV[m_nCurrentChar + 5] = charGeomV[m_nCurrentChar + 8] = charGeomV[m_nCurrentChar + 11] = 0.0;
		
		/*
		// Vertex Xs
		charGeomV[m_nCurrentChar + 0] = charGeomV[m_nCurrentChar + 2] = x;
		charGeomV[m_nCurrentChar + 1] = charGeomV[m_nCurrentChar + 3] = x + m_fCharacterWidth;
		
		// Vertex Ys
		charGeomV[m_nCurrentChar + 4] = charGeomV[m_nCurrentChar + 6] = y;
		charGeomV[m_nCurrentChar + 5] = charGeomV[m_nCurrentChar + 7] = y + m_fCharacterHeight;
		*/
		
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT,0, &charGeomT);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &charGeomV);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		
		x += m_fCharacterWidth;
		
	}
	 


	
	unbindTexture(myTextures[1]);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();		
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}





void Engine::drawCamera() {	
	gluLookAt(
		myCameraPosition.x, myCameraPosition.y, myCameraPosition.z,
		myCameraTarget.x, myCameraTarget.y, myCameraTarget.z,
		0.0, 1.0, 0.0
	);
}


/*
void Engine::buildPlayer(foo *playerFoo) {
	//Player
	myPlayerAnimationIndex = 0;
	myPlayerAnimationDirection = 1;
	myPlayerMaxSpeed = 120.0;
	myPlayerJumpSpeed = 60.0;
	
	myPlayerPosition = Vector3DMake(0.0, 300.0, 0.0);
	myPlayerSpeed = Vector3DMake(400.0, 0.0, 0.0);
	myPlayerAcceleration = Vector3DMake(0.0, 0.0, 0.0);
	myPlayerAnimationIndex = 0;
	
	myPlayerRunCycle = 1;
	
	//15 //14 //13         //16 //17 //25

	myPlayerTransformedCycle = 17;
	myPlayerTransformUpCycle = 25;
	myPlayerTransformDownCycle = 16;
	myPlayerIsTransformed = false;
	myPlayerNeedsTransform = true;
	
	myPlayerManager = new Md2Manager();
	
	myPlayerMd2 = myPlayerManager->Load(playerFoo, 25);
	
	for (int cycle = 0; cycle < myPlayerMd2->GetNumCycles(); cycle++) {
		//LOGV("%d %d %s\n", myMd2->GetNumCycles(), cycle,myMd2->GetCycleName(cycle));
	}
	
	//myMd2->SwitchCycle(myPlayerRunCycle, 0.0, true, -1);
}
*/

/*
void Engine::tickPlayer() {
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

	myPlayerManager->Update(myDeltaTime);
	
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
*/


void Engine::drawPlayer() {
	glPushMatrix();
	{
		glTranslatef(myPlayerPosition.x, myPlayerPosition.y + 1.5, myPlayerPosition.z);
		glRotatef(myPlayerRotation, 0.0, 0.0, 1.0);
		float scale = 0.5;
		glScalef(scale, scale, scale);
		bindTexture(myPlayerTexture);
		myPlayerManager->Render();
		unbindTexture(myPlayerTexture);
	}
	glPopMatrix();
}


void Engine::buildPlatforms() {
	/*
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
	 */
}


void Engine::tickPlatform() {
	myPlayerOnPlatform = false;
	myPlayerPlatformCorrection = Vector3DMake(0.0, 0.0, 0.0);
	iteratePlatform(0);
}


void Engine::drawPlatform() {
	bindTexture(myGroundTexture);
	iteratePlatform(1);
	unbindTexture(myGroundTexture);
}


void Engine::iteratePlatform(int operation) {
	for (int j=0; j<myPlatformCount; j++) {
		Platform platform = myPlatforms[j];
		//if ((platform.position.x > (myPlayerPosition.x - platform.length - 10.0)) && (platform.position.x < (myPlayerPosition.x + platform.length + 10.0))) {
			for (float i = platform.position.x; i < platform.position.x + platform.length; i += platform.step) {
				//if ((i < (myPlayerPosition.x + 1200.0)) && (i > (myPlayerPosition.x - 150.0))) {
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
				//}
			}
		//}
	}
}


void Engine::drawPlatformSegment(float baseY, float x1, float y1, float x2, float y2) {
	float beginX; float beginY; float endX; float endY;
	
	//baseY -= 0.0;
	//baseY = fastSinf(baseY) * randf() * 1000.0;
	
	float platformRadius = 50.0;

	int total = 2.0; //randf() * 10.0;

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
	//glDrawArrays(GL_LINES, 0, 6 * total);
	glDrawArrays(GL_TRIANGLES, 0, 6 * total);
	glDisableClientState(GL_VERTEX_ARRAY);
}


void Engine::tickPlatformSegment(float beginX, float beginY, float endX, float endY) {
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


//returns a random float between 0 and 1
float Engine::randf() {
	//random hack since no floating point random function
	//optimize later
	return (lrand48() % 255) / 255.f;
}


void Engine::reset_vertex(int idx) {
	int i = idx * 3;
	vertices[i + 0] = generator[idx].x;
	vertices[i + 1] = generator[idx].y;
	vertices[i + 2] = generator[idx].z;
}


void Engine::random_velocity(int idx) {
	//velocity[idx].x = -0.35 + (randf() * 0.01);
	//velocity[idx].y = 0.25 - (randf() * 0.5);
	//velocity[idx].z = 0.25 - (randf() * 0.5);
	/*
	velocity[idx].x = -0.35 + (randf() * 0.01);
	velocity[idx].y = 0.25 - (randf() * 0.5);
	velocity[idx].z = 0.25 - (0.5) + (randf() * 0.002 * myPlayerSpeed.x);
	*/

	velocity[idx].x = 0.5 - randf();
	velocity[idx].y = -(randf() * 5.0);
	velocity[idx].z = 0.5 - randf();
}

void Engine::reset_particle(int idx) {

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

void Engine::update_vertex(int idx) {
	int i = idx * 3;
	vertices[i] += velocity[idx].x;
	vertices[i+1] += velocity[idx].y;
	vertices[i+2] += velocity[idx].z;
	//velocity[idx].y -= 0.0002 * fabs(myPlayerSpeed.y);
	//LOGV("%d %f %f %f\n", idx, vertices[idx * 3], life[idx], myPlayerPosition.x);
}


static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,1.0f,1.0f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},
	{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},
	{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f}
};


void Engine::update_color(int idx) {
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


void Engine::reset_life(int i) {
	life[i] = 1.0;
}


void Engine::buildFountain() {	
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


void Engine::tickFountain() {	
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


void Engine::drawFountain() {
	if (false) {
		//GLfloat points [ ] = { myPlayerPosition.x, myPlayerPosition.y, myPlayerPosition.z };
		bindTexture(myFountainTextures[0]);
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


const GLfloat Engine::mySkyBoxVertices[108] = {
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


const GLfloat Engine::cubeTextureCoords[72] = {
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


void Engine::buildSkyBox() {
	mySkyBoxRotation = 0.0;
}


void Engine::tickSkyBox() {
}


void Engine::bindTexture(GLuint texture) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
}


void Engine::unbindTexture(GLuint texture) {
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}


void Engine::drawSkyBox() {
	glPushMatrix();
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTranslatef(myPlayerPosition.x, 0.0, 0.0);
		mySkyBoxRotation += 0.1;
		glScalef(125.0, 125.0, 125.0);
		glRotatef(mySkyBoxRotation, 0.0, 1.0, 0.0);
		glVertexPointer(3, GL_FLOAT, 0, mySkyBoxVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, cubeTextureCoords);
		for (int i=0; i<6; i++) {
			glBindTexture(GL_TEXTURE_2D, myTextures[i] + 2);
			glDrawArrays(GL_TRIANGLES, i * 6, 6);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		
	}
	glPopMatrix();
}
