//
//  GLViewController.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//
/*
  class generic_category : public boost::system::error_category
  {
  public:
    generic_category();
    const char *name() { return "wtf"; }
    std::string message(int ev) const;
  };

  class system_category : public boost::system::error_category
  {
  public:
    system_category();
    const char *name() { return "wtf"; }
    std::string message(int ev) const;
  };
  */


#include <pthread.h>

#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// a single header file is required
//#include <ev.c>

#include <stdio.h> // for puts


#include "importgl.h"
#include "Engine.h"


class BadConversion : public std::runtime_error {
public:
	BadConversion(std::string const& s)
	: std::runtime_error(s)
	{ }
};

inline std::string stringify(double x)
{
	std::ostringstream o;
	if (!(o << x))
		throw BadConversion("stringify(double)");
	return o.str();
}


static GLuint myFontTexture;





GLViewController::GLViewController() {
	Md2Manager::Release();
}


GLViewController::~GLViewController() {
	LOGV("dealloc GameController\n");
	Md2Manager::Release();
	if (myPlatforms) {
		free(myPlatforms);
	}
}


void GLViewController::playerStartedJumping() {
	if (myPlayerCanDoubleJump) {
		myPlayerCanDoubleJump = false;
		myPlayerLastJump = mySimulationTime;
	} else {
		//myPlayerNeedsTransform = true;
	}
	
	if (myPlayerOnPlatform) {
		myPlayerLastJump = mySimulationTime;
	}
	
	//myPlayerSpeed.x = 0;

}


void GLViewController::playerStoppedJumping() {
	myPlayerLastEnd = mySimulationTime;
}


void GLViewController::build(int width, int height, GLuint *textures, FILE *playerFilename, unsigned int off, unsigned int len) {
	//Screen
	screenWidth = width;
	screenHeight = height;
	
	//World
	myGravity = -4000.0; //-500
	myState = 0;
	mySimulationTime = 0.0;
	
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
	
	buildFont();
	buildSkyBox();
	buildPlayer(playerFilename, off, len);
	buildPlatforms();
	buildCamera();
	buildSpiral();
	buildFountain();
	mySceneBuilt = true;
}

int GLViewController::tick(float delta) {
	mySimulationTime += (myDeltaTime = delta);
	
	if (mySceneBuilt) {
		//tickFont();
		tickPlatform();
		tickPlayer();
		tickSpiral();
		tickFountain();
		tickCamera();
	}
	
	if (myPlayerPosition.y < -100.0) {
		return 0;
	} else {
		return 1;
	}	
}





void GLViewController::gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    GLfloat xmin, xmax, ymin, ymax;
	
    ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;
	
    glFrustumx((GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
               (GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
               (GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536));
}


void GLViewController::prepareFrame(int width, int height) {
    glViewport(0, 0, width, height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float) width / (float) height, 10.0f, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


void GLViewController::resizeScreen(int width, int height) {
	screenWidth = width;
	screenHeight = height;
}


void GLViewController::draw(float rotation) {
	
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
			drawCamera();
			drawPlayer();
			drawPlatform();
			drawFountain();
			//drawSkyBox();
			drawSpiral();
			drawFont();
			glDisable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
		}
		glPopMatrix();
	}
}


void GLViewController::buildFont() {	
	
	m_charPixelWidth = m_animPixelWidth = CHAR_PIXEL_W;
	m_charPixelHeight = m_animPixelWidth = CHAR_PIXEL_H;
	
	//
	// Zero Z Coords in Geomtery Array.
	//
	for(int i=0; i<(MAX_CHAR_BUFFER * ONE_CHAR_SIZE_V); i+=ONE_CHAR_SIZE_V)
	{
		charGeomV[i+2] = charGeomV[i+5] = charGeomV[i+8] = charGeomV[i+11] = 0.0;
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


void GLViewController::tickFont() {
}


void GLViewController::drawFont() {
	std::string fps = stringify(myPlayerSpeed.x);
		
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


	bindTexture(myFontTexture);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();

	glOrthof(0, _scaleX, 0, _scaleY, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float x = 0.0;

	for (int i=0; i<fps.length(); i++) {

		
		int c = fps.at(i) - ' ';


		float y = 0.5;


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

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT,0, &charGeomT);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &charGeomV);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		x += m_fCharacterWidth;
	}
	 
	unbindTexture(myFontTexture);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();		
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void GLViewController::buildCamera() {
	//Camera
	myCameraTarget = Vector3DMake(35.0, 0.0, 0.0);
	myCameraPosition = Vector3DMake(-45.0, 20.0, 80.0);
	myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
}


void GLViewController::tickCamera() {
	Vector3D cameraPosition;
	Vector3D cameraTarget;
	float limitP;
	float limitT;
	/*
	if (mySimulationTime < 0.5) {
		//close up sideways
		cameraPosition = Vector3DMake(20.0, 0.0, 100.0);
		cameraTarget = Vector3DMake(20.0, 0.0, 0.0);
		limit = 0.02;
		
	} else if (mySimulationTime < 1.0) {
		//far out sideways
		cameraPosition = Vector3DMake(50.0, 0.0, 300.0);
		cameraTarget = Vector3DMake(50.0, 0.0, 0.0);
		limit = 0.02;

	} else 
		*/
	//if (mySimulationTime < 2.5) {
	if (!myPlayerOnPlatform) {
		//on top of
		//cameraPosition = Vector3DMake(-20.0, 15.0, 0.0);
		//cameraTarget = Vector3DMake(80.0, 8.0, 0.0);
		
		//mid out sideways
		cameraPosition = Vector3DMake(-80.0, 15.0, 30.0);
		cameraTarget = Vector3DMake(90.0, 8.0, 0.0);
		

	} else {
		cameraPosition = Vector3DMake(-60.0, 15.0, 30.0);
		cameraTarget = Vector3DMake(190.0, 8.0, 0.0);

		//far out sideways
		//cameraPosition = Vector3DMake(50.0, 0.0, 250.0);
		//cameraTarget = Vector3DMake(50.0, 0.0, 0.0);
		//limit = 0.02;
	}
	
	limitP = 0.0070;
	limitT = 0.0070;
	
	Vector3D desiredPosition = Vector3DAdd(myPlayerPosition, cameraPosition);
	
	Vector3DFlip(&desiredPosition);
	Vector3D deltaP = Vector3DAdd(desiredPosition, myCameraPosition);
	deltaP = Vector3DLimit(deltaP, myPlayerSpeed.x * limitP);
	Vector3DFlip(&deltaP);
	
	myCameraPosition = Vector3DAdd(myCameraPosition, deltaP);
	
	Vector3D desiredTarget = Vector3DAdd(myPlayerPosition, cameraTarget);
	Vector3DFlip(&desiredTarget);
	Vector3D deltaT = Vector3DAdd(desiredTarget, myCameraTarget);
	deltaT = Vector3DLimit(deltaT, myPlayerSpeed.x * limitT);
	Vector3DFlip(&deltaT);
	
	myCameraTarget = Vector3DAdd(myCameraTarget, deltaT);
}


void GLViewController::drawCamera() {	
	gluLookAt(myCameraPosition.x, myCameraPosition.y, myCameraPosition.z,
			  myCameraTarget.x, myCameraTarget.y, myCameraTarget.z,
			  0.0, 1.0, 0.0);
}


void GLViewController::buildPlayer(FILE *playerFilename, unsigned int off, unsigned int len) {
	//Player
	myPlayerAnimationIndex = 0;
	myPlayerAnimationDirection = 1;
	myPlayerMaxSpeed = 120.0;
	myPlayerJumpSpeed = 60.0;
	
	myPlayerPosition = Vector3DMake(0.0, 100.0, 0.0);
	myPlayerSpeed = Vector3DMake(300.0, 0.0, 0.0);
	myPlayerAcceleration = Vector3DMake(0.0, 0.0, 0.0);
	myPlayerAnimationIndex = 0;
	
	myPlayerRunCycle = 1;
	myPlayerJumpCycle = 6;
	myPlayerTransformedCycle = 7;
	myPlayerTransformUpCycle = 9;
	myPlayerTransformDownCycle = 11;
	myPlayerIsTransformed = false;
	myPlayerNeedsTransform = false;
	
	myMd2 = Md2Manager::Load(playerFilename, 15, off, len);
	
	//for (int cycle = 0; cycle < myMd2->GetNumCycles(); cycle++) {
	//	LOGV("%d %d %s\n", myMd2->GetNumCycles(), cycle,myMd2->GetCycleName(cycle));
	//}
	
	myMd2->SwitchCycle(myPlayerRunCycle, 0.0, true, -1);
}


void GLViewController::tickPlayer() {
	
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
			myMd2->SwitchCycle(myPlayerTransformUpCycle, 0.1, false, -1, myPlayerRunCycle);
		} else {
			myMd2->SwitchCycle(myPlayerTransformDownCycle, 0.1, false, 1, myPlayerTransformedCycle);
		}
		myPlayerIsTransformed = !myPlayerIsTransformed;
	} else {
	}

	Md2Manager::Update(myDeltaTime);
	
	if (timeSinceStarted < 0.02 && timeSinceStarted > 0.0) {
		//myMd2->SwitchCycle(6, 0.001, false);
		//myMd2->SwitchCycle(myPlayerJumpCycle, 0.1, false);
		myPlayerFalling = false;
		if (myPlayerJumping) {

			if (!timeSinceEnded) {
				myPlayerAcceleration.y = 120.0; //60
			} else {
				//myMd2->SwitchCycle(1, 0.01, false);
				myPlayerFalling = true;
			}
		} else {
			myPlayerAcceleration.x = 1000.0;
			myPlayerSpeed.y = 600.0;
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
	}
	
	myPlayerSpeed = Vector3DAdd(myPlayerSpeed, Vector3DMake(myPlayerAcceleration.x * myDeltaTime, myPlayerAcceleration.y * myDeltaTime, myPlayerAcceleration.z * myDeltaTime));
	
	//NSLog("%f\n", myPlayerSpeed.y);
	if (myPlayerSpeed.y < -300.0) {
		myPlayerSpeed.y = -300.0;
	}
	
	Vector3D oldPosition = myPlayerPosition;
	
	myPlayerPosition = Vector3DAdd(myPlayerPosition, Vector3DMake(myPlayerSpeed.x * myDeltaTime, myPlayerSpeed.y * myDeltaTime, myPlayerSpeed.z * myDeltaTime));
	
	if (!myPlayerOnPlatform) {
		float slope = (myPlayerPosition.y - oldPosition.y) / (myPlayerPosition.x - oldPosition.x);
		myPlayerRotation = slope * 30.0;
	}
}

void GLViewController::drawPlayer() {
	
	
	glPushMatrix();
	{
		//horse
		//glTranslatef(myPlayerPosition.x, myPlayerPosition.y - 1.75, myPlayerPosition.z);
		//glRotatef(myPlayerRotation, 0.0, 0.0, 1.0);
		//float scale = 0.15;
		
		glTranslatef(myPlayerPosition.x, myPlayerPosition.y + 10, myPlayerPosition.z);
		glRotatef(myPlayerRotation, 0.0, 0.0, 1.0);
		float scale = 0.5;
		
		glScalef(scale, scale, scale);
		bindTexture(myPlayerTexture);
		Md2Manager::Render();
		unbindTexture(myPlayerTexture);

	}
	glPopMatrix();
}


void GLViewController::buildPlatforms() {	
	myPlatformCount = 100;
	Vector3D lastPlatformPosition = Vector3DMake(0.0, 0.0, 0.0);
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));
	int step;
	float randomY;
	float randomA;
	float randomL;
	int length;

	lastPlatformPosition.x = 40.0;
	
	for (int i=0; i<myPlatformCount; i++) {
		randomY = lastPlatformPosition.y;
		while (fabs(lastPlatformPosition.y - randomY) < 10.0) {
			randomY = ((random() / (float)RAND_MAX) * 50.0);
		}
		
		randomA = ((random() / (float)RAND_MAX) * 5.0);
		randomL = 40.0; //50.0 - (i * randf());
		
		if (randomA > 5.0) {
			step = 28;
		} else if (randomA > 3.0) {
			step = 29;
		} else if (randomA > 1.0) {
			step = 30;
		} else if (randomA > 0.5) {
			step = 31;
		} else {
			step = 32;
		}
		
		length = step * randomL;
		
		myPlatforms[i].position = Vector3DMake(lastPlatformPosition.x, randomY, 0.0);
		myPlatforms[i].length = length;
		myPlatforms[i].amplitude = randomA;
		
		myPlatforms[i].step = step;
		myPlatforms[i].angular_frequency = 0.030; //1 / (i + 0.000001) + 1.0;
		myPlatforms[i].phase = 0.0;
		lastPlatformPosition = myPlatforms[i].position;
		LOGV("%f\n", logf((float)i + 1.1) * 20.0);
		lastPlatformPosition.x += length + (logf((float)i + 1.1) * 50.0);
	}
}


void GLViewController::tickPlatform() {
	myPlayerOnPlatform = false;
	myPlayerPlatformCorrection = Vector3DMake(0.0, 0.0, 0.0);
	iteratePlatform(0);
}


void GLViewController::drawPlatform() {
	bindTexture(myGroundTexture);
	iteratePlatform(1);
	unbindTexture(myGroundTexture);
}


void GLViewController::iteratePlatform(int operation) {
	for (int j=0; j<myPlatformCount; j++) {
		Platform platform = myPlatforms[j];
		//if ((platform.position.x > (myPlayerPosition.x - platform.length - 10.0)) && (platform.position.x < (myPlayerPosition.x + platform.length + 10.0))) {
			for (float i = platform.position.x; i < platform.position.x + platform.length; i += platform.step) {
				if ((i < (myPlayerPosition.x + 400.0)) && (i > (myPlayerPosition.x - 100.0))) {
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
							drawPlatformSegment(beginX, beginY, endX, endY);
							break;
					}
				}
			}
		//}
	}
}


const GLfloat GLViewController::myPlatformTextureCoords[12] = {
	0.0, 0.0, // top-upper-right
	1.0, 0.0,
	1.0, 1.0,
	1.0, 1.0, // top-lower-left
	0.0, 1.0,
	0.0, 0.0,
};


void GLViewController::drawPlatformSegment(float beginX, float beginY, float endX, float endY) {
	glPushMatrix();
	{		
		float platformRadius = 10.0;
		
		GLfloat platformVertices[18] = {
			beginX, beginY, -platformRadius, // top-upper-right
			beginX, beginY, platformRadius,
			endX, endY, platformRadius,
			
			endX, endY, platformRadius, // top-lower-left
			endX, endY, -platformRadius,
			beginX, beginY, -platformRadius
		};
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, platformVertices);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, myPlatformTextureCoords);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableClientState(GL_VERTEX_ARRAY);
	}
	glPopMatrix();
}


void GLViewController::tickPlatformSegment(float beginX, float beginY, float endX, float endY) {
	if (!myPlayerJumping) {
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
			myPlayerRotation = slope * 45.0;
		//} else {
		//	myPlayerRotation = slope;
		//}
		float distanceFromIntersection = myPlayerPosition.y - 1.25 - myPlayerPlatformIntersection.y;
		myPlayerPlatformCorrection.y = -(distanceFromIntersection / myDeltaTime) - (myPlayerAcceleration.y * myDeltaTime);
		myPlayerOnPlatform = true;
	}
	}
}


void GLViewController::tickSpiral() {
	myGarbageCollectorPosition = myPlayerPosition;
}


void GLViewController::buildSpiral() {
	int dots = 4;
	int lines_from_dot = 0;
	
	mySpiralArrays = (dots * 3) + ((dots * lines_from_dot) * 3);
	mySpiralVertices = (GLfloat *)malloc(mySpiralArrays * sizeof(GLfloat));
	
    GLfloat x,y,z,angle,incline,interval;
    int c = 0;
	float r = 4.0;
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


void GLViewController::drawSpiral() {
	glPushMatrix();
	{
		float rotation = mySimulationTime * 360.0;
		glTranslatef(myGarbageCollectorPosition.x, myGarbageCollectorPosition.y, myGarbageCollectorPosition.z);	
		glRotatef(90.0, 0.0f, 1.0f, 0.0f);
		//glScalef(1.0, fastSinf(mySimulationTime) + 1.0, fastSinf(mySimulationTime) + 1.0);
		glRotatef(rotation, 1.0, -fastSinf(mySimulationTime), fastSinf(mySimulationTime));
		/*
		 for (int i=0; i<mySpiralArrays; i+=6) {
		 glEnableClientState(GL_VERTEX_ARRAY);
		 glVertexPointer(3, GL_FLOAT, 0, mySpiralVertices);
		 glDrawArrays(GL_LINE_STRIP, i, 3);
		 glDrawArrays(GL_POINTS, i, 1);
		 glDisableClientState(GL_VERTEX_ARRAY);
		 }
		 */
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, mySpiralVertices);
		glDrawArrays(GL_POINTS, 0, mySpiralArrays / 3);
		glDisableClientState(GL_VERTEX_ARRAY);
		
	}
	glPopMatrix();
}


void GLViewController::tickGarbageCollector() {
	//float percentOfMaxSpeed = myPlayerSpeed.x / myPlayerMaxSpeed;
	//float distanceFromPlayer = percentOfMaxSpeed * 20.0;
	//myGarbageCollectorPosition = Vector3DMake(myPlayerPosition.x - distanceFromPlayer, 0.0, 0.0);	 
	//myGarbageCollectorPosition = myPlayerPosition;
}


void GLViewController::buildGarbageCollector() {
	/*
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
	 */
}


void GLViewController::drawGarbageCollector() {	
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


void GLViewController::tickSomething() {
}


void GLViewController::buildSomething() {
	/*
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
	 */
}


void GLViewController::drawSomething() {
	/*
	 glPushMatrix();
	 {
	 glRotatef(90.0, 0.0, 1.0, 0.0);
	 glEnableClientState(GL_VERTEX_ARRAY);
	 glVertexPointer (2, GL_FLOAT , 0, mySomethingVertices);
	 glDrawArrays (GL_LINE_LOOP, 0, mySomethingArrays);
	 }
	 glPopMatrix();
	 */
}


//returns a random float between 0 and 1
float GLViewController::randf()
{
    //random hack since no floating point random function
    //optimize later
    return (lrand48() % 255) / 255.f;
}


void GLViewController::reset_vertex(int idx) {
    int i = idx * 3;
    vertices[i + 0] = generator[idx].x;
    vertices[i + 1] = generator[idx].y;
    vertices[i + 2] = generator[idx].z;
}


void GLViewController::random_velocity(int idx) {
    //velocity[idx].x = -0.35 + (randf() * 0.01);
	//velocity[idx].y = 0.25 - (randf() * 0.5);
    //velocity[idx].z = 0.25 - (randf() * 0.5);
	
	velocity[idx].x = -0.35 + (0.01);
	velocity[idx].y = 0.25 - (0.5);
    velocity[idx].z = 0.25 - (0.5);
}

void GLViewController::reset_particle(int idx) {

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
	
	//LOGV("RESET: %d %f %f\n", idx, vertices[idx * 3], life[idx]);

}

void GLViewController::update_vertex(int idx) {
	
    int i = idx * 3;
    vertices[i] += velocity[idx].x;
    vertices[i+1] += velocity[idx].y;
    vertices[i+2] += velocity[idx].z;
	//LOGV("%d %f %f %f\n", idx, vertices[idx * 3], life[idx], myPlayerPosition.x);
}

static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,0.5f,0.5f},{1.0f,0.75f,0.5f},{1.0f,1.0f,0.5f},{0.75f,1.0f,0.5f},
	{0.5f,1.0f,0.5f},{0.5f,1.0f,0.75f},{0.5f,1.0f,1.0f},{0.5f,0.75f,1.0f},
	{0.5f,0.5f,1.0f},{0.75f,0.5f,1.0f},{1.0f,0.5f,1.0f},{1.0f,0.5f,0.75f}
};


void GLViewController::update_color(int idx) {
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
	colors[i+3] = 1.0;	
}

void GLViewController::reset_life(int i) {
	life[i] = 1.0;
}

void GLViewController::buildFountain() {	
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


void GLViewController::tickFountain() {	
	int i = 0; //particle index
    for(i=0;i<NUM_PARTICLES;i++) {
        life[i] -= 0.075;
        if(life[i] <= 0.0) {
            reset_particle(i);
        } else {
			update_color(i);
			update_vertex(i);
		}
    }
}


void GLViewController::drawFountain() {
	if (!myPlayerOnPlatform) {
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//glEnable(GL_BLEND);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_FLOAT, 0, colors);
		glPointSize(7.0);
		glDrawElements(GL_POINTS, NUM_PARTICLES, GL_UNSIGNED_SHORT, elements);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		//glDisable(GL_BLEND);
	}
}


const GLfloat GLViewController::mySkyBoxVertices[108] = {
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


const GLfloat GLViewController::cubeTextureCoords[72] = {
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


void GLViewController::buildSkyBox() {
	mySkyBoxRotation = 0.0;
}


void GLViewController::tickSkyBox() {
}


void GLViewController::bindTexture(GLuint texture) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texture);
}


void GLViewController::unbindTexture(GLuint texture) {
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}


void GLViewController::drawSkyBox() {
	glPushMatrix();
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mySkyBoxTexture[0]);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		
		glTranslatef(myPlayerPosition.x, 0.0, 0.0);
				
		mySkyBoxRotation += 1.0;
		
		glScalef(400.0, 400.0, 400.0);
		glRotatef(mySkyBoxRotation, 0.0, 1.0, 0.0);
				
		glVertexPointer(3, GL_FLOAT, 0, mySkyBoxVertices);
		glTexCoordPointer(2, GL_FLOAT, 0, cubeTextureCoords);
		for (int i=0; i<6; i++) {
			bindTexture(mySkyBoxTextures[i]);
			glDrawArrays(GL_TRIANGLES, i * 6, 6);	
			unbindTexture(mySkyBoxTextures[i]);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		
	}
	glPopMatrix();
}
