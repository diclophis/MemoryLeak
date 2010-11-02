/*
 *  Model.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

class Model {
public:
	Model(const aiScene *a) : m_Scene(a) {
		build();
	};
	
	void render();
	
private:
	bool build();

	const aiScene *m_Scene;
	


	
	int frame;
	
	int numVBO;
	GLuint *vboID;
};