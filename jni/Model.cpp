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
	

	
	int numFrames = m_Scene->mNumMeshes;
	
	numVBO = (numFrames * 3);
	
	vboID = (GLuint*)malloc(sizeof(GLuint) * (numVBO));
	glGenBuffers(numVBO, vboID);
	
	/*
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
	 */
	
	//if (aimesh.HasTextureCoords(0)) {
	//	glTexCoordPointer(3, GL_FLOAT, 0, aimesh.mTextureCoords[0]);
	//}
	
	int cnt = m_Scene->mNumMeshes;
	for (unsigned int mm=0; mm<cnt; mm++) {
		//const aiMesh& aimesh = *m_Scene->mMeshes[mm];
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
	
	/*
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
	 */
	
	//glBindBufferARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, vboID[numVBO - 1]	);
	//glBufferDataARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, numIndices * sizeof(int),
	//					texInds, GL_STATIC_DRAW_ARB	);
	
	/*
	glBindBufferARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, vboID[numVBO]	);
	glBufferDataARB(	GL_ELEMENT_ARRAY_BUFFER_ARB, numIndices * sizeof(int),
					verInds, GL_STATIC_DRAW_ARB );
	*/
	/*
	free(verts);
	free(norms);
	free(texCrds);
	free(texInds);
	free(verInds);
	 */
	return true;
}

void Model::render() {
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//glEnable(GL_NORMALIZE);
	
	int frame = 0;
	
	glBindBuffer(GL_ARRAY_BUFFER, vboID[frame + 1]);
	glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
	
	glBindBuffer(GL_ARRAY_BUFFER, vboID[frame + 2]	);
	glNormalPointer(GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
	
	//glBindBufferARB(	GL_ARRAY_BUFFER_ARB, vboID[numVBO - 2]	);
	//glTexCoordPointer(	2, GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[frame + 0]);	
	glDrawElements(GL_TRIANGLES,3 * m_Scene->mMeshes[0]->mNumFaces, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
	
	//glDisable(GL_NORMALIZE);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	
}