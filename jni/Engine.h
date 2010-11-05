//
//  Engine.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//


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

  /*


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
  */
	
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
	std::vector<GLuint> *textures;
	std::vector<foo*> *models;
	Assimp::Importer importer;
	Engine(int width, int height, std::vector<GLuint> &textures, std::vector<foo*> &models);
	virtual ~Engine();
  virtual void build() = 0;
	int gameState;
	int tick();
	virtual int simulate() = 0;
	virtual void render() = 0;
	void draw(float rotation);
  void pause();
	void bindTexture(GLuint texture);
	void unbindTexture(GLuint texture);
	void resizeScreen(int width, int height);
	float randf();
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
	void buildCamera();
	virtual void tickCamera() = 0;
	void drawCamera();
	

	virtual void hitTest(float x, float y, int hitState) = 0;

	
private:

	
};
