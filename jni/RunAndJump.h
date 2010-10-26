//
//  GLViewController.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//


//#include "Engine.h"


class RunAndJump : public Engine {
	
public:

  RunAndJump(int width, int height, std::vector<GLuint> x_textures, std::vector<foo*> x_models) : Engine(width, height, x_textures, x_models) {
  };

	~RunAndJump();
	void hitTest(float x, float y);
	
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
	

	
	GLfloat *mySomethingVertices;
	GLfloat *myGarbageCollectorVertices;
	GLfloat *mySpiralVertices;
	
	Platform *myPlatforms;

  int mySegmentIndex;
  int mySegmentCount;
	std::vector<Md2Instance *> mySegments;
	Md2Manager mySegmentManager;
	
	PlayerState *myPlayerStates;
	int myPlayerStatesCount;
	
	Vector3D myPlayerPlatformIntersection;
	
	

	
	int myState;
	int myStatesToShow;
	int myLastStateAvailable;
	Vector3D myGarbageCollectorPosition;
	
	int mySpiralArrays;
	int myGarbageCollectorArrays;
	int mySomethingArrays;
	
	int myPlatformCount;
	
	Vector3D myPlayerPlatformCorrection;
	
	//static const GLfloat mySkyBoxVertices[];
	//static const GLfloat cubeTextureCoords[];
	//GLuint mySkyBoxTexture[1];
		
	//float myBuildSkyBoxDuration;
	//float myBuildPlatformDuration;
	
	//GLuint myGroundTexture;
	
	// Player Engine
	float myPlayerHeight;
	Md2Instance *myPlayer;
	Md2Manager myPlayerManager;




	// Game Engine
	void build();
	int simulate();
	void render();
	void buildCamera();
	void tickCamera();
	
	
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
	

	void buildStates();
	void pushState(bool shift);
	void setState(int n);

	void playerStartedJumping();
	void playerStoppedJumping();

	void buildFountain();
	void tickFountain();
	void drawFountain();
	void reset_life(int idx);


	int myPlayerRunCycle;
	int myPlayerJumpCycle;
	int myPlayerTransformedCycle;
	int myPlayerTransformUpCycle;
	int myPlayerTransformDownCycle;
	int myPlayerIsTransformed;
	int myPlayerNeedsTransform;
	
	inline std::string stringify(double x);
		
	GLuint myTreeTextures[1];
	
	// SkyBox Engine
	float mySkyBoxHeight;
	Md2Instance *mySkyBox;
	Md2Manager mySkyBoxManager;
	
};
