/*
 *  Model.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

class Model {
	aiScene *m_Scene;
	Model(aiScene &scene) : m_Scene(&scene) {
	};
	bool build();
	bool draw();
	
	int frame;
	
	int numVBO;
	int vboID;
};