//
//  RaptorIsland.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

//#include "Engine.h"
//#include "MachineGun.h"

class RaptorIsland : public Engine {

public:
	
	~RaptorIsland();
	
	// Raptor Engine
	float myRaptorHeight;
	std::vector<Md2Instance *> myRaptors;
	Md2Manager myRaptorManager;
	
	// Barrel Engine
	float myBarrelHeight;
	std::vector<Md2Instance *> myBarrels;
	Md2Manager myBarrelManager;
	
	// SkyBox Engine
	float mySkyBoxHeight;
	Md2Instance *mySkyBox;
	Md2Manager mySkyBoxManager;
	
	// Player Engine
	float myPlayerHeight;
	Md2Instance *myPlayer;
	Md2Manager myPlayerManager;
	
	// Game Engine
	void build(int width, int height, std::vector<GLuint> textures, std::vector<foo*> models);
	int simulate();
	void render();
	void buildCamera();
	void tickCamera();
	
	// Steering Engine
	// a group (STL vector) of all vehicles in the PlugIn
	CtfSeeker *ctfSeeker; 
	std::vector<CtfBase*> all;
	unsigned int mMode;
	Vec3 steeringFromInput;

	GLfloat myLineVertices[6];
	CtfBase *myNearest;
	void hitTest(float x, float y);
	bool IntersectCircleSegment(
    const Vec3& c,        // center
    float r,                            // radius
    const Vec3& p1,     // segment start
    const Vec3& p2);     // segment end

  Vec3 m_LastCollide;

	MachineGun m_Gun;
	
};
