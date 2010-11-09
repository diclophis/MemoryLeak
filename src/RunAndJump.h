//
//  GLViewController.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//  Copyright __MyCompanyName__ 2009. All rights reserved.
//

class RunAndJump : public Engine {
	
public:

  RunAndJump(int width, int height, std::vector<GLuint> &x_textures, std::vector<foo*> &x_models) : Engine(width, height, x_textures, x_models) {
    LOGV("\n\n\n1234!!!\n\n\n\n");
  };

	~RunAndJump();
	void hitTest(float x, float y, int hitState);
	
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
	
	
	//GLfloat *mySomethingVertices;
	//GLfloat *myGarbageCollectorVertices;
	//GLfloat *mySpiralVertices;
	
	std::vector<Platform> myPlatforms;

  int mySegmentIndex;
  int mySegmentCount;
	std::vector<Model *> mySegments;
	
	//PlayerState *myPlayerStates;
	//int myPlayerStatesCount;
	
	//Vector3D myPlayerPlatformIntersection;

	int myTerrainIndex;
	int myTerrainCount;
	float myTerrainHeight;
	std::vector<Model *> myTerrains;
	
	//int myState;
	//int myStatesToShow;
	//int myLastStateAvailable;
	//Vector3D myGarbageCollectorPosition;
	
	//int mySpiralArrays;
	//int myGarbageCollectorArrays;
	//int mySomethingArrays;
	
	int myPlatformCount;
	
	//Vector3D myPlayerPlatformCorrection;
	
	float myPlayerHeight;

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

	void iteratePlatform(int operation);
	void tickPlatformSegment(float beginX, float beginY, float endX, float endY);

	//void buildSpiral();
	//void tickSpiral();
	//void drawSpiral();
	
	
	//void buildGarbageCollector();
	//void tickGarbageCollector();
	//void drawGarbageCollector();
	
	//void buildSomething();
	//void tickSomething();
	//void drawSomething();
	

	//void buildStates();
	//void pushState(bool shift);
	//void setState(int n);

	void playerStartedJumping();
	void playerStoppedJumping();

	//void buildFountain();
	//void tickFountain();
	//void drawFountain();
	//void reset_life(int idx);


	//int myPlayerRunCycle;
	//int myPlayerJumpCycle;
	//int myPlayerTransformedCycle;
	//int myPlayerTransformUpCycle;
	//int myPlayerTransformDownCycle;
	//int myPlayerIsTransformed;
	//int myPlayerNeedsTransform;
	
	//inline std::string stringify(double x);

	//GLuint myTreeTextures[1];
	
	// SkyBox Engine
	//float mySkyBoxHeight;
	//Md2Instance *mySkyBox;
	//Md2Manager mySkyBoxManager;
	
	Model *m_Player;
	Model *m_SkyBox;

  Vector3D myPlayerPlatformIntersection;
  Vector3D myPlayerPosition;
  Vector3D myPlayerSpeed;
  Vector3D myPlayerAcceleration;
  Vector3D myPlayerJumpStartPosition;
  float myPlayerMaxSpeed;
  float myPlayerJumpSpeed;
  GLfloat myPlayerRotation;
  bool myPlayerJumping;
  bool myPlayerCanDoubleJump;
  bool myPlayerOnPlatform;
  bool myPlayerBelowPlatform;
  float myPlayerLastJump;
  float myPlayerLastEnd;
  Vector3D myPlayerPlatformCorrection;

	//aiScene *m_BoxScene;
	//aiScene *m_RingScene;
	
  MachineGun *m_Gun;

	
};