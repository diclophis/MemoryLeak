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
	int m_numBuffers;
	GLuint *m_VerticeBuffers;
	GLuint *m_NormalBuffers;
	GLuint *m_IndexBuffers;
	GLuint *m_TextureBuffer;
	int m_numFaces;
	int m_numFrames;
	int m_AnimationStart;
	int m_AnimationEnd;
};