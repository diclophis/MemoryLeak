//
//  RaptorIsland.h
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//

#include "Engine.h"

class RaptorIsland : public Engine {

public:
	
	void build(int width, int height, GLuint *textures, foo *playerFoo);
	int simulate();
	void render();
	void tickCamera();

};
