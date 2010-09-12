//
//  RaptorIsland.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

#include "Engine.h"

class RaptorIsland : public Engine {

public:
	

	std::vector<Md2Instance *> myRaptors;
	Md2Manager *myRaptorManager;
	
	
	void build(int width, int height, GLuint *textures, std::vector<foo*> models);
	int simulate();
	void render();
	void tickCamera();

};
