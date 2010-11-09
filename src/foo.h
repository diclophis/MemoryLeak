/*
 *  foo.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 9/6/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


struct foo {
	FILE *fp;
	unsigned int off;
	unsigned int len;
};

struct foofoo {
	const aiScene *m_Scene;
	int numVBO;
	GLuint *vboID;
	GLuint *m_TextureBuffer;
float m_Scale[3];
float m_Position[3];
float m_Rotation[3];
float m_Life;
float m_Velocity[3];
int mNumFaces;
int numFrames;