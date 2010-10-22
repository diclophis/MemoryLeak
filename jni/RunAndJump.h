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
	
	//#define NUM_PARTICLES 10
	
	/*
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
	

	*/
	
	GLfloat *mySomethingVertices;
	GLfloat *myGarbageCollectorVertices;
	GLfloat *mySpiralVertices;
	
	Platform *myPlatforms;
	
	PlayerState *myPlayerStates;
	int myPlayerStatesCount;
	
	//bool mySceneBuilt;

	
	float myGravity;
	//float mySimulationTime;
	//float myDeltaTime;
	
	Vector3D myPlayerPlatformIntersection;
	
	
	//Vector3D myCameraPosition;
	//Vector3D myCameraSpeed;
	//Vector3D myCameraTarget;
	
	
	//Vector3D myPlayerPosition;
	//Vector3D myPlayerSpeed;
	//Vector3D myPlayerAcceleration;
	//Vector3D myPlayerJumpStartPosition;
	
	//float myPlayerMaxSpeed;
	//float myPlayerJumpSpeed;
	
	//GLfloat myPlayerRotation;
	
	//int myPlayerAnimationIndex;
	//int myPlayerAnimationDirection;
	
	//bool myPlayerJumping;
	//bool myPlayerCanDoubleJump;
	//bool myPlayerOnPlatform;
	//bool myPlayerBelowPlatform;
	
	int myState;
	int myStatesToShow;
	int myLastStateAvailable;
	Vector3D myGarbageCollectorPosition;
	
	int mySpiralArrays;
	int myGarbageCollectorArrays;
	int mySomethingArrays;
	
	int myPlatformCount;
	
	//float myPlayerLastJump;
	//float myPlayerLastEnd;
	
	Vector3D myPlayerPlatformCorrection;
	
	static const GLfloat mySkyBoxVertices[];
	static const GLfloat cubeTextureCoords[];
	GLuint mySkyBoxTexture[1];
		
	float myBuildSkyBoxDuration;
	float myBuildPlatformDuration;
	
	GLuint myGroundTexture;
	
	Md2Instance *myPlayerMd2;
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

/*
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
 */
	
	//int screenWidth;
	//int screenHeight;
		
	//void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar);
	//void prepareFrame(int width, int height);
	
	int myPlayerRunCycle;
	int myPlayerJumpCycle;
	int myPlayerTransformedCycle;
	int myPlayerTransformUpCycle;
	int myPlayerTransformDownCycle;
	int myPlayerIsTransformed;
	int myPlayerNeedsTransform;
	
	inline std::string stringify(double x);
	
	//bool myGameStarted;
	//int myGameSpeed;
	
	GLuint myTreeTextures[1];
	
	//void camera_directions(float * out_rgt, float * out_up , float * out_look);

	// SkyBox Engine
	float mySkyBoxHeight;
	Md2Instance *mySkyBox;
	Md2Manager mySkyBoxManager;
	
};
