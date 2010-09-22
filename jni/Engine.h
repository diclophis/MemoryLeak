//
//  Engine.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//

#include "importgl.h"
#include "OpenGLCommon.h"
#include "MD2_Model.h"
#include "MD2_Manager.h"
//#include "Player.h"


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

class Engine {
	
public:
	
	Engine(const Engine&);
	Engine& operator=(const Engine&);
	
	// Fountain Engine
	#define NUM_PARTICLES 10
	GLfloat vertices[NUM_PARTICLES * 3];
	GLfloat colors[NUM_PARTICLES * 4];
	GLushort elements[NUM_PARTICLES];
	Vector3D generator[NUM_PARTICLES]; //keep track of generator (origin) for each particle
	Vector3D velocity[NUM_PARTICLES]; //keep track of velocity vector for each particle
	float alpha[NUM_PARTICLES]; //keep track of alpha for display
	float life[NUM_PARTICLES]; //keep track of life of particle
	GLuint myFountainTextures[1];
	void buildFountain();
	void tickFountain();
	void drawFountain();
	void reset_life(int idx);
	float randf();
	void reset_vertex(int idx);
	void random_velocity(int idx);
	void reset_particle(int idx);
	void update_vertex(int idx);
	void update_color(int idx);

	// Font Engine
	float m_nScalerX;		// To scale texture values based on imported pngs.
	float m_nScalerY;		// To scale texture values based on imported pngs.
	float m_charPixelWidth;
	float m_charPixelHeight;
	float m_animPixelWidth;
	float m_animPixelHeight;	
	int m_ntextWidth, m_ntextHeight;
	GLfloat m_fCharacterWidth, m_fCharacterHeight;
	GLfloat		charTexCoords[FONT_TEXTURE_ATLAS_WIDTH*FONT_TEXTURE_ATLAS_LINES*8]; // Predefined texture coords
	int m_nCurrentChar;					// Position in current line.
	GLfloat charGeomV[12];
	//GLfloat charGeomT[(MAX_CHAR_BUFFER * ONE_CHAR_SIZE_T)];
	GLfloat charGeomT[8];
	GLuint myFontTexture;
	//GLint viewport[4];	
	void buildFont();
	void tickFont();
	void drawFont();
	
// Something Engine	
//GLfloat *mySomethingVertices;
//int mySomethingArrays;
//void buildSomething();
//void tickSomething();
//void drawSomething();

// GarbageCollector Engine
//GLfloat *myGarbageCollectorVertices;
//Vector3D myGarbageCollectorPosition;
//int myGarbageCollectorArrays;
//void buildGarbageCollector();
//void tickGarbageCollector();
//void drawGarbageCollector();

//Spiral Engine
//GLfloat *mySpiralVertices;
//int mySpiralArrays;
//void buildSpiral();
//void tickSpiral();
//void drawSpiral();


	// Platform Engine
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
	Platform *myPlatforms;
	int myPlatformCount;
	GLuint myGroundTexture;
	//static const GLfloat myPlatformTextureCoords[];
	void buildPlatforms();
	void tickPlatform();
	void drawPlatform();
	void iteratePlatform(int operation);
	void tickPlatformSegment(float beginX, float beginY, float endX, float endY);
	void drawPlatformSegment(float baseY, float beginX, float beginY, float endX, float endY);
	
	// Player Engine
	GLuint myPlayerTexture;
	Vector3D myPlayerPlatformIntersection;
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
	float myPlayerLastJump;
	float myPlayerLastEnd;
	Vector3D myPlayerPlatformCorrection;
	Md2Instance *myPlayerMd2;
	Md2Manager *myPlayerManager;
	int myPlayerRunCycle;
	int myPlayerJumpCycle;
	int myPlayerTransformedCycle;
	int myPlayerTransformUpCycle;
	int myPlayerTransformDownCycle;
	int myPlayerIsTransformed;
	int myPlayerNeedsTransform;
	//PlayerState *myPlayerStates;
	//int myPlayerStatesCount;
	//int myState;
	//int myStatesToShow;
	//NSTimeInterval myTimeSinceLastStatePush;
	//int myLastStateAvailable;
	//typedef struct {
	//	Vector3D position;
	//	GLfloat rotation;
	//} PlayerState;
	//void buildPlayer(foo *playerFoo);
	void tickPlayer();
	void drawPlayer();
	void playerStartedJumping();
	void playerStoppedJumping();

	// World Engine
	bool mySceneBuilt;
	bool myViewportSet;

	float myGravity;
	float mySimulationTime;
	float myDeltaTime;
	bool myGameStarted;
	int myGameSpeed;
	int screenWidth;
	int screenHeight;
	std::vector<GLuint> myTextures;
	Engine();
	virtual ~Engine();
	virtual void build(int width, int height, std::vector<GLuint> textures, std::vector<foo*> models) = 0;
	int tick();
	virtual int simulate() = 0;
	virtual void render() = 0;
	void draw(float rotation);
	void bindTexture(GLuint texture);
	void unbindTexture(GLuint texture);
	void resizeScreen(int width, int height);
	
	void prepareFrame(int width, int height);
	inline std::string stringify(double x);
	
	void go();
	
	pthread_mutex_t m_mutex;
	pthread_t m_thread;
	static void *start_thread(void *);
	bool mNeedsTick;
	
	
	// Camera Engine
	Vector3D myCameraPosition;
	Vector3D myCameraSpeed;
	Vector3D myCameraTarget;
	virtual void buildCamera() = 0;
	virtual void tickCamera() = 0;
	void drawCamera();
	
	// SkyBox Engine	
	static const GLfloat mySkyBoxVertices[];
	static const GLfloat cubeTextureCoords[];
	GLuint mySkyBoxTexture[1];
	float myBuildSkyBoxDuration;
	float myBuildPlatformDuration;
	GLuint *mySkyBoxTextures;
	GLfloat mySkyBoxRotation;
	void buildSkyBox();
	void tickSkyBox();
	void drawSkyBox();

	//void camera_directions(float * out_rgt, float * out_up , float * out_look);
	
private:

	
};
