/*
 *  Model.cpp
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "importgl.h"
#include "OpenGLCommon.h"

#include "assimp.hpp"
#include "aiPostProcess.h"
#include "aiScene.h"

#include "Model.h"



bool Model::build() {
	
	numFrames = m_Scene->mNumMeshes;
	mNumFaces = m_Scene->mMeshes[0]->mNumFaces;
	
	numVBO = (numFrames * 3);
	vboID = (GLuint*)malloc(sizeof(GLuint) * (numVBO));
	m_TextureBuffer = (GLuint*)malloc(sizeof(GLuint) * (1));

	glGenBuffers(numVBO, vboID);
	glGenBuffers(1, m_TextureBuffer);
	
	const aiMesh& aimesh = *m_Scene->mMeshes[0];

	if (aimesh.HasTextureCoords(0)) {
		glBindBuffer(GL_ARRAY_BUFFER, m_TextureBuffer[0]);
		glBufferData(GL_ARRAY_BUFFER, aimesh.mNumVertices * 3 * sizeof(float), aimesh.mTextureCoords[0], GL_STATIC_DRAW);
	} else {
		throw 1;
	}
	
	int cnt = m_Scene->mNumMeshes;
	for (unsigned int mm=0; mm<cnt; mm++) {
		unsigned short* indices = new unsigned short[m_Scene->mMeshes[mm]->mNumFaces * 3];
		for(unsigned int i=0,j=0; i<m_Scene->mMeshes[mm]->mNumFaces; ++i,j+=3)
		{
			indices[j]   = m_Scene->mMeshes[mm]->mFaces[i].mIndices[0];
			indices[j+1] = m_Scene->mMeshes[mm]->mFaces[i].mIndices[1];
			indices[j+2] = m_Scene->mMeshes[mm]->mFaces[i].mIndices[2];
		}
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[(mm * 3) + 0]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Scene->mMeshes[mm]->mNumFaces * 3 * sizeof(short), indices, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, vboID[(mm * 3) + 1]);
		glBufferData(GL_ARRAY_BUFFER, m_Scene->mMeshes[mm]->mNumVertices * 3 * sizeof(float), m_Scene->mMeshes[mm]->mVertices, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ARRAY_BUFFER, vboID[(mm * 3) + 2]);
		glBufferData(GL_ARRAY_BUFFER, m_Scene->mMeshes[mm]->mNumVertices * 3 * sizeof(float), m_Scene->mMeshes[mm]->mNormals, GL_STATIC_DRAW);
		delete indices;
	}

	return true;
}

void Model::render(int frame) {
	glPushMatrix();

	glTranslatef(m_Position[0],m_Position[1],m_Position[2]);
	glRotatef(m_Rotation[0],0,0,1);
	glRotatef(m_Rotation[1],0,1,0);
	glRotatef(m_Rotation[2],1,0,0);
	glScalef(m_Scale[0],m_Scale[1],m_Scale[2]);
	
    glBindBuffer(GL_ARRAY_BUFFER, vboID[frame + 1]);
    glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
    
    glBindBuffer(GL_ARRAY_BUFFER, vboID[frame + 2]	);
    glNormalPointer(GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_TextureBuffer[0]);
    glTexCoordPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[frame + 0]);
    glDrawElements(GL_TRIANGLES,3 * mNumFaces, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

	glPopMatrix();
}
