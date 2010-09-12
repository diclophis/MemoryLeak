//
//  GLViewController.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

#include "OpenGLCommon.h"
#include "MD2_Model.h"
#include "MD2_Manager.h"
#include "Player.h"


// For FastFontDefinition
//#define CHAR_WIDTH 0.0234375			/* ogl tex coords :: Based on not quite 42 chars per line! */
//#define CHAR_HEIGHT 0.203125			/* ogl tex coords */

#define CHAR_WIDTH 0.1			/* ogl tex coords :: Based on not quite 42 chars per line! */
#define CHAR_HEIGHT 0.1

// Initial setup only.
#define CHAR_PIXEL_W 20 // 8
#define CHAR_PIXEL_H 23 // 9

#define MAX_CHAR_BUFFER 256

#define ONE_CHAR_SIZE_V 12				// Floats making up one TRIANGLE_STRIP Vertices
#define ONE_CHAR_SIZE_T 8				// Floats making up one TRIANGLE_STRIP TexCoords

#define FONT_TEXTURE_ATLAS_WIDTH 10		// Characters per line in Atlas
#define FONT_TEXTURE_ATLAS_LINES 10		// Lines of characters in the Atlas

#ifndef LOGV
#define LOGV printf
#endif


class GLViewController {
	
public:

	
	typedef struct {
		GLfloat step;
		GLfloat amplitude;
		GLfloat angular_frequency;
		GLfloat last_angular_frequency;
		GLfloat phase;
		GLfloat length;
		Vector3D position;
		Vector3D speed;
		Vector3D acceleration;
	} Platform;
	
	typedef struct {
		Vector3D position;
		GLfloat rotation;
	} PlayerState;
	
	#define NUM_PARTICLES 10
	
	GLfloat vertices[NUM_PARTICLES * 3];
	GLfloat colors[NUM_PARTICLES * 4];
	GLushort elements[NUM_PARTICLES];
	Vector3D generator[NUM_PARTICLES]; //keep track of generator (origin) for each particle
	Vector3D velocity[NUM_PARTICLES]; //keep track of velocity vector for each particle
	float alpha[NUM_PARTICLES]; //keep track of alpha for display
	float life[NUM_PARTICLES]; //keep track of life of particle
	
	
	float		m_nScalerX;		// To scale texture values based on imported pngs.
	float		m_nScalerY;		// To scale texture values based on imported pngs.
	
	float		m_charPixelWidth;
	float		m_charPixelHeight;
	float		m_animPixelWidth;
	float		m_animPixelHeight;	
	
	int			m_ntextWidth, m_ntextHeight;
	GLfloat		m_fCharacterWidth, m_fCharacterHeight;
	
	GLuint myPlayerTexture;
	
	// Predefined texture coords
	GLfloat		charTexCoords[FONT_TEXTURE_ATLAS_WIDTH*FONT_TEXTURE_ATLAS_LINES*8];
	
	// Working space for OpenGL
	int			m_nCurrentChar;					// Position in current line.
	GLfloat		charGeomV[(MAX_CHAR_BUFFER * ONE_CHAR_SIZE_V)];
	GLfloat		charGeomT[(MAX_CHAR_BUFFER * ONE_CHAR_SIZE_T)];
	GLint		viewport[4];	
	
	GLfloat *mySomethingVertices;
	GLfloat *myGarbageCollectorVertices;
	GLfloat *mySpiralVertices;
	
	
	Platform *myPlatforms;
	//static const GLfloat myPlatformTextureCoords[];
	
	
	PlayerState *myPlayerStates;
	int myPlayerStatesCount;
	
	bool mySceneBuilt;

	
	float myGravity;
	float mySimulationTime;
	float myDeltaTime;
	Vector3D myPlayerPlatformIntersection;
	
	
	Vector3D myCameraPosition;
	Vector3D myCameraSpeed;
	Vector3D myCameraTarget;
	
	
	Vector3D myPlayerPosition;
	Vector3D myPlayerSpeed;
	Vector3D myPlayerAcceleration;
	Vector3D myPlayerJumpStartPosition;
	float myPlayerMaxSpeed;
	float myPlayerJumpSpeed;
	
	GLfloat myPlayerRotation;
	
	int myPlayerAnimationIndex;
	int myPlayerAnimationDirection;
	
	bool myPlayerJumping;
	bool myPlayerCanDoubleJump;
	bool myPlayerOnPlatform;
	bool myPlayerBelowPlatform;
	
	int myState;
	int myStatesToShow;
	//NSTimeInterval myTimeSinceLastStatePush;
	int myLastStateAvailable;
	Vector3D myGarbageCollectorPosition;
	
	int mySpiralArrays;
	int myGarbageCollectorArrays;
	int mySomethingArrays;
	
	int myPlatformCount;
	
	//NSDate *myPlayerLastJump;
	//NSDate *myPlayerLastEnd;
	
	float myPlayerLastJump;
	float myPlayerLastEnd;
	
	Vector3D myPlayerPlatformCorrection;
	
	
	static const GLfloat mySkyBoxVertices[];
	static const GLfloat cubeTextureCoords[];
	GLuint mySkyBoxTexture[1];
		
	float myBuildSkyBoxDuration;
	float myBuildPlatformDuration;
	
	GLuint myGroundTexture;
	
	Md2Instance *myPlayerMd2;
	Md2Manager *myPlayerManager;
	
	
	GLViewController();
	~GLViewController();
	
	void build(int width, int height, GLuint *textures, foo *playerFoo);
	int tick(float delta);
	void draw(float rotation);

	void buildCamera();
	void tickCamera();
	void drawCamera();


	void buildPlayer(foo *playerFoo);

	void tickPlayer();
	void drawPlayer();


	void buildPlatforms();
	void tickPlatform();
	void drawPlatform();


	void iteratePlatform(int operation);
	void tickPlatformSegment(float beginX, float beginY, float endX, float endY);
	void drawPlatformSegment(float baseY, float beginX, float beginY, float endX, float endY);


	void buildSpiral();
	void tickSpiral();
	void drawSpiral();
	
	
	
	void buildGarbageCollector();
	void tickGarbageCollector();
	void drawGarbageCollector();
	
	void buildSomething();
	void tickSomething();
	void drawSomething();


	void buildFont();
	void tickFont();
	void drawFont();

	void buildStates();
	void pushState(bool shift);
	void setState(int n);

	void playerStartedJumping();
	void playerStoppedJumping();

	void buildFountain();
	void tickFountain();
	void drawFountain();
	void reset_life(int idx);


	void buildSkyBox();
	void tickSkyBox();
	void drawSkyBox();
	GLuint *mySkyBoxTextures;
	GLfloat mySkyBoxRotation;
	
	float randf();
	void reset_vertex(int idx);
	void random_velocity(int idx);
	void reset_particle(int idx);
	void update_vertex(int idx);
	void update_color(int idx);
	void bindTexture(GLuint texture);
	void unbindTexture(GLuint texture);
	
	int screenWidth;
	int screenHeight;
	
	void resizeScreen(int width, int height);
	
	void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
	void prepareFrame(int width, int height);
	
	int myPlayerRunCycle;
	int myPlayerJumpCycle;
	int myPlayerTransformedCycle;
	int myPlayerTransformUpCycle;
	int myPlayerTransformDownCycle;
	int myPlayerIsTransformed;
	int myPlayerNeedsTransform;
	
	inline std::string stringify(double x);
	
	bool myGameStarted;
	int myGameSpeed;
	
	GLuint myTreeTextures[1];
	
	void camera_directions(float * out_rgt, float * out_up , float * out_look);
	
};
