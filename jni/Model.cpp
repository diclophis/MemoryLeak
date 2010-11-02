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

#include "Engine.h"
#include "RunAndJump.h"

#include "Model.h"

bool Model::build() {
	

	
	int numFrames = m_Scene->mNumFrames;
	
	numVBO = (numFrames * 2) + 3;
	
	vboID = (GLuint*)malloc(sizeof(GLuint) * (numVBO + 1));
	glGenBuffersARB(numVBO, vboID);
	
	int numVerts = model.header->num_Vertices;
	
	float *verts = (float*)malloc(sizeof(float) * numVerts * 3);
	float *norms = (float*)malloc(sizeof(float) * numVerts * 3);
	
	for (int f = 0; f < numFrames; f++) {
		
		for (int v = 0; v < numVerts; v++) {
			
			for (int s = 0; s < 3; s++) {
				
				verts[(v * 3) + s] = ((float)model.frames[f].vertices[v].v[s]
									  * model.frames[f].scale[s])
				+ model.frames[f].translate[s];
				norms[(v * 3) + s] = model.normals[(f * numVerts) + v][s];
			}
			
		}
		
		glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[f * 2]	);
		glBufferDataARB(	GL_ARRAY_BUFFER_ARB, numVerts * 3 * sizeof(float),
						verts, GL_STATIC_DRAW_ARB	);
		
		glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[(f * 2) + 1]	);
		glBufferDataARB(	GL_ARRAY_BUFFER_ARB, numVerts * 3 * sizeof(float),
						norms, GL_STATIC_DRAW_ARB	);
		
	}
	
	int numTexCrds = model.header->num_TexCoords;
	
	float *texCrds = (float*)malloc(sizeof(float) * numTexCrds * 2);
	
	for (int t = 0; t < numTexCrds; t++) {
		
		texCrds[(t * 2)] =	(float)model.texpos[t].s 
		/ model.header->skinWidth;
		texCrds[(t * 2) + 1] =	(float)model.texpos[t].t
		/ model.header->skinHeight;
	}
	
	glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[numVBO - 2]	);
	glBufferDataARB(	GL_ARRAY_BUFFER_ARB, numTexCrds * 2 * sizeof(float),
					texCrds, GL_STATIC_DRAW_ARB	);
	
	numIndices = model.header->num_Triangles * 3;
	
	int *texInds = (int*)malloc(sizeof(int) * numIndices);
	int *verInds = (int*)malloc(sizeof(int) * numIndices);
	
	for (int i = 0; i < numIndices / 3; i++) {
		
		for (int u = 0; u < 3; u++) {
			
			texInds[(i * 3) + u] = (int)model.triangles[i].texture[u];// * 2;
			verInds[(i * 3) + u] = (int)model.triangles[i].vertex[u];// * 3;
			
		}
		
	}
	
	//glBindBufferARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, vboID[numVBO - 1]	);
	//glBufferDataARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, numIndices * sizeof(int),
	//					texInds, GL_STATIC_DRAW_ARB	);
	
	glBindBufferARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, vboID[numVBO]	);
	glBufferDataARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, numIndices * sizeof(int),
					verInds, GL_STATIC_DRAW_ARB );
	
	free(verts);
	free(norms);
	free(texCrds);
	free(texInds);
	free(verInds);
}

bool Model::render() {
	
	glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[frame]	);
	glVertexPointer(	3, GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
	
	glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[frame + 1]	);
	glNormalPointer(	GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
	
	glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[numVBO - 2]	);
	glTexCoordPointer(	2, GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
	
	glBindBufferARB(	GL_ELEMENT_ARRAY_BUFFER, vboID[numVBO]	);
	glDrawRangeElementsEXT(		GL_TRIANGLES, 0, numIndices, 
						   numIndices, GL_UNSIGNED_INT,
						   (GLvoid*)((char*)NULL)	);
	
	//glDrawElements(	GL_TRIANGLES, 
	//				numIndices, 
	//				GL_UNSIGNED_INT, 
	//				(GLvoid*)((char*)NULL)	);
}