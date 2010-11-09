/*
 *  Model.cpp
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include "MemoryLeak.h"
#include "Engine.h"
#include "aiScene.h"
#include "Model.h"


static GLuint g_lastVertexBuffer = 0;
static GLuint g_lastNormalBuffer = 0;
static GLuint g_lastTexcoordBuffer = 0;
static GLuint g_lastElementBuffer = 0;


foofoo *Model::GetFoo(const aiScene *a) {
	
	foofoo *ff = new foofoo;
	
	ff->m_numFrames = a->mNumMeshes;
	ff->m_numFaces = a->mMeshes[0]->mNumFaces;
	ff->m_numBuffers = (ff->m_numFrames * 3);
	ff->m_VerticeBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_TextureBuffer = (GLuint*)malloc(sizeof(GLuint) * (1));
	 
	glGenBuffers(ff->m_numBuffers, ff->m_VerticeBuffers);
	glGenBuffers(1, ff->m_TextureBuffer);

	const aiMesh& aimesh = *a->mMeshes[0];

	if (aimesh.HasTextureCoords(0)) {
		glBindBuffer(GL_ARRAY_BUFFER, ff->m_TextureBuffer[0]);
		glBufferData(GL_ARRAY_BUFFER, aimesh.mNumVertices * 3 * sizeof(float), aimesh.mTextureCoords[0], GL_STATIC_DRAW);
	} else {
		throw 1;
	}

	for (unsigned int mm=0; mm<a->mNumMeshes; mm++) {
		unsigned short* indices = new unsigned short[a->mMeshes[mm]->mNumFaces * 3];
		//LOGV("foo: %s\n", a->mMeshes[mm]->mName.data);
		for(unsigned int i=0,j=0; i<a->mMeshes[mm]->mNumFaces; ++i,j+=3)
		{
			indices[j]   = a->mMeshes[mm]->mFaces[i].mIndices[0];
			indices[j+1] = a->mMeshes[mm]->mFaces[i].mIndices[1];
			indices[j+2] = a->mMeshes[mm]->mFaces[i].mIndices[2];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_VerticeBuffers[(mm * 3) + 0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, a->mMeshes[mm]->mNumFaces * 3 * sizeof(short), indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, ff->m_VerticeBuffers[(mm * 3) + 1]);
		glBufferData(GL_ARRAY_BUFFER, a->mMeshes[mm]->mNumVertices * 3 * sizeof(float), a->mMeshes[mm]->mVertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, ff->m_VerticeBuffers[(mm * 3) + 2]);
		glBufferData(GL_ARRAY_BUFFER, a->mMeshes[mm]->mNumVertices * 3 * sizeof(float), a->mMeshes[mm]->mNormals, GL_STATIC_DRAW);
		delete indices;
	}
	
	return ff;
}


void Model::render(int frame) {
	glPushMatrix();
	{
		glTranslatef(m_Position[0],m_Position[1],m_Position[2]);
		glRotatef(m_Rotation[0],0,0,1);
		glRotatef(m_Rotation[1],0,1,0);
		glRotatef(m_Rotation[2],1,0,0);
		glScalef(m_Scale[0],m_Scale[1],m_Scale[2]);
		
		if (m_FooFoo->m_VerticeBuffers[frame + 1] != g_lastVertexBuffer) {
			g_lastVertexBuffer = m_FooFoo->m_VerticeBuffers[frame + 1];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastVertexBuffer);
			glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
		}

		if (m_FooFoo->m_VerticeBuffers[frame + 2] != g_lastNormalBuffer) {
			g_lastNormalBuffer = m_FooFoo->m_VerticeBuffers[frame + 2];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastNormalBuffer);
			glNormalPointer(GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
		}

		if (m_FooFoo->m_TextureBuffer[0] != g_lastTexcoordBuffer) {
			g_lastTexcoordBuffer = m_FooFoo->m_TextureBuffer[0];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastTexcoordBuffer);
			glTexCoordPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
		}

		if (m_FooFoo->m_VerticeBuffers[frame + 0] != g_lastElementBuffer) {
			g_lastElementBuffer = m_FooFoo->m_VerticeBuffers[frame + 0];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
		}		
		
		glDrawElements(GL_TRIANGLES,3 * m_FooFoo->m_numFaces, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
	}
	glPopMatrix();
}
