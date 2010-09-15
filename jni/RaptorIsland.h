//
//  RaptorIsland.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

#include "Engine.h"

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
	
	// Game Engine
	void build(int width, int height, std::vector<GLuint> textures, std::vector<foo*> models);
	int simulate();
	void render();
	void buildCamera();
	void tickCamera();
};
