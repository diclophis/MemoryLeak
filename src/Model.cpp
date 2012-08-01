/*
 *  Model.cpp
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 GPL. All rights reserved.
 *
 */


#include "MemoryLeak.h"


void Model::ReleaseBuffers() {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


Model::~Model() {
  LOGV("dealloc model\n");
  free(m_Scale);
  free(m_Position);
  free(m_Rotation);
  free(m_Velocity);
  delete m_Steps;
}


Model::Model(foofoo *a) : m_FooFoo(a) {
  m_Frame = 0;
  m_IsPlayer = false;
  m_IsEnemy = false;
  m_IsBomb = false;
  m_IsShield = false;
  m_IsStuck = false;
  m_IsHarmfulToPlayers = false;
  m_IsHardfultoEnemies = false;
  m_IsHelpfulToPlayers = false;
  m_IsHelpfulToEnemies = false;
  m_NeedsClimbBeforeMove = false;
  m_NeedsClimbAfterMove = false;
  m_IsMoving = false;

  m_Scale = (float *)malloc(3 * sizeof(float));
  m_Position = (float *)malloc(3 * sizeof(float));
  m_Rotation = (float *)malloc(3 * sizeof(float));
  m_Velocity = (float *)malloc(3 * sizeof(float));
  m_Climbing = NULL;
  m_IsFalling = false;

  m_Life = 0.0;
  m_Fps = 30.0;
  m_Theta = 0.0;
  m_IsAlive = false;

  SetScale(1.0, 1.0, 1.0);
  SetPosition(0.0, 0.0, 0.0);
  SetRotation(0.0, 0.0, 0.0);
  SetVelocity(0.0, 0.0, 0.0);

  m_Steps = new std::vector<void *>;
}


foofoo *Model::GetBatchFoo(GLuint texture_index, int max_face_count, int max_model_count) {
  size_t size_of_model_foo = sizeof(ModelFoo);
  foofoo *ff = new foofoo;
  ff->m_Stride = size_of_model_foo;
  ff->m_Texture = texture_index;
  ff->m_numFaces = max_face_count * max_model_count;
  ff->m_numModelFoos = ff->m_numFaces * 3;
  ff->m_ModelFoos = (ModelFoo *)malloc(ff->m_numModelFoos * sizeof(ModelFoo));
  ff->m_IndexFoo = (GLshort *)malloc((ff->m_numFaces * 3) * sizeof(GLshort));
  ff->m_BufferCount = 2;

  ff->m_numVertexArrayObjects = ff->m_BufferCount;
  ff->m_VertexArrayObjects = (GLuint*)calloc((ff->m_numVertexArrayObjects), sizeof(GLuint));
  for (int i = 0; i < ff->m_numVertexArrayObjects; i++) {
    ff->m_VertexArrayObjects[i] = 0;
  }

  ff->m_numInterleavedBuffers = ff->m_BufferCount;
  ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numInterleavedBuffers));
  glGenBuffers(ff->m_numInterleavedBuffers, ff->m_InterleavedBuffers);
  for (int i = 0; i < ff->m_numInterleavedBuffers; i++) {
    glBindBuffer(GL_ARRAY_BUFFER, ff->m_InterleavedBuffers[i]);
    size_t interleaved_buffer_size = (ff->m_numFaces * ff->m_Stride);
    glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, ff->m_ModelFoos, GL_DYNAMIC_DRAW);
  }

  ff->m_numIndexBuffers = ff->m_BufferCount;
  ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numIndexBuffers));
  glGenBuffers(ff->m_numIndexBuffers, ff->m_IndexBuffers);
  for (int i = 0; i < ff->m_numIndexBuffers; i++) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[i]);
    size_t interleaved_element_buffer_size = (ff->m_numFaces) * sizeof(GLshort);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, interleaved_element_buffer_size, ff->m_IndexFoo, GL_DYNAMIC_DRAW);
  }

  return ff;
}

/*
foofoo *Model::GetFoo(const aiScene *a, int s, int e) {
  if (!a->mMeshes[0]->HasTextureCoords(0)) {
    LOGV("no tex coords\n");
    return NULL;
  }

  foofoo *ff = new foofoo;
  int interp = 10;
  if (a->mNumMeshes > 1) {
    ff->m_numFrames = (e - s) * interp;
  } else {
    interp = 1;
    ff->m_numFrames = a->mNumMeshes;
  }

  ff->m_numFaces = a->mMeshes[0]->mNumFaces;
  ff->m_AnimationStart = s;
  ff->m_AnimationEnd = e;
  ff->m_numModelFoos = (ff->m_numFaces * 3 * ff->m_numFrames);
  ff->m_ModelFoos = (ModelFoo *)malloc(ff->m_numModelFoos * sizeof(ModelFoo));
  ff->m_IndexFoo = (GLshort *)malloc((ff->m_numFaces * 3) * sizeof(GLshort));

  int model_foo_offset = 0;
  for (int mm=ff->m_AnimationStart; mm<ff->m_AnimationEnd; mm++) {
    for (int iiii=0; iiii<interp; iiii++) {
      for(unsigned int i=0,j=0; i<a->mMeshes[mm]->mNumFaces; ++i,j+=3) {
        ff->m_IndexFoo[j]   = a->mMeshes[mm]->mFaces[i].mIndices[0];
        ff->m_IndexFoo[j+1] = a->mMeshes[mm]->mFaces[i].mIndices[1];
        ff->m_IndexFoo[j+2] = a->mMeshes[mm]->mFaces[i].mIndices[2];
      }

      if (iiii == 0) {
        for(unsigned int ik=0; ik<a->mMeshes[mm]->mNumVertices; ++ik) {
          memcpy(&ff->m_ModelFoos[model_foo_offset].vertex[0], &a->mMeshes[mm]->mVertices[ik], 3 * sizeof(GLfloat));
          memcpy(&ff->m_ModelFoos[model_foo_offset].normal[0], &a->mMeshes[mm]->mNormals[ik], 3 * sizeof(GLfloat));
          memcpy(&ff->m_ModelFoos[model_foo_offset].texture[0], &a->mMeshes[0]->mTextureCoords[0][ik], 3 * sizeof(GLfloat));
          model_foo_offset++;
        }
      } else {
        float percent_of_way = (float)iiii / (float)interp;
        for(unsigned int ik=0,jk=0; ik<a->mMeshes[mm]->mNumVertices; ++ik, jk+=3) {
          ff->m_ModelFoos[model_foo_offset].vertex[0] = a->mMeshes[mm]->mVertices[ik][0] + (percent_of_way * (a->mMeshes[mm + 1]->mVertices[ik][0] - a->mMeshes[mm]->mVertices[ik][0]));
          ff->m_ModelFoos[model_foo_offset].vertex[1] = a->mMeshes[mm]->mVertices[ik][1] + (percent_of_way * (a->mMeshes[mm + 1]->mVertices[ik][1] - a->mMeshes[mm]->mVertices[ik][1]));
          ff->m_ModelFoos[model_foo_offset].vertex[2] = a->mMeshes[mm]->mVertices[ik][2] + (percent_of_way * (a->mMeshes[mm + 1]->mVertices[ik][2] - a->mMeshes[mm]->mVertices[ik][2]));
          ff->m_ModelFoos[model_foo_offset].normal[0] = a->mMeshes[mm]->mNormals[ik][0];
          ff->m_ModelFoos[model_foo_offset].normal[1] = a->mMeshes[mm]->mNormals[ik][1];
          ff->m_ModelFoos[model_foo_offset].normal[2] = a->mMeshes[mm]->mNormals[ik][2];
          ff->m_ModelFoos[model_foo_offset].texture[0] = a->mMeshes[0]->mTextureCoords[0][ik].x;
          ff->m_ModelFoos[model_foo_offset].texture[1] = a->mMeshes[0]->mTextureCoords[0][ik].y;
          model_foo_offset++;
        }
      }
    }
  }

  return ff;
}
*/


void Model::RenderFoo(StateFoo *sf, foofoo *foo, bool copy) {
  
  if (foo->m_Texture != sf->g_lastTexture) {
    sf->g_lastTexture = foo->m_Texture;
    glBindTexture(GL_TEXTURE_2D, sf->g_lastTexture);
  }

#ifdef HAS_VAO
  if (foo->m_VertexArrayObjects[sf->m_LastBufferIndex] == 0) {
    glGenVertexArraysOES(1, &foo->m_VertexArrayObjects[sf->m_LastBufferIndex]);
    sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[sf->m_LastBufferIndex];
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    sf->g_lastElementBuffer = -1;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, foo->m_IndexBuffers[sf->m_LastBufferIndex]);
    
    sf->g_lastInterleavedBuffer = -1;
    glBindBuffer(GL_ARRAY_BUFFER, foo->m_InterleavedBuffers[sf->m_LastBufferIndex]);
    
    glVertexPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + (0));
    glNormalPointer(GL_FLOAT, foo->m_Stride, (char *)(NULL) + (3 * sizeof(GLfloat)));
    glTexCoordPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + ((3 * sizeof(GLfloat)) + (3 * sizeof(GLfloat))));

  }
    
  if (foo->m_VertexArrayObjects[sf->m_LastBufferIndex] != sf->g_lastVertexArrayObject) {
    sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[sf->m_LastBufferIndex];
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);
  }
  
  if (foo->m_IndexBuffers[sf->m_LastBufferIndex] != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = foo->m_IndexBuffers[sf->m_LastBufferIndex];
    //TODO: figure out why this is redundant
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }
  
  if (foo->m_InterleavedBuffers[sf->m_LastBufferIndex] != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[sf->m_LastBufferIndex];
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }
    
#else

  if (!sf->m_EnabledStates) {
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    sf->m_EnabledStates = true;
  }
  
  if (foo->m_IndexBuffers[sf->m_LastBufferIndex] != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = foo->m_IndexBuffers[sf->m_LastBufferIndex];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }
  
  if (foo->m_InterleavedBuffers[sf->m_LastBufferIndex] != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[sf->m_LastBufferIndex];
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }
  
  glVertexPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + (0));
	glNormalPointer(GL_FLOAT, foo->m_Stride, (char *)(NULL) + (3 * sizeof(GLfloat)));
  glTexCoordPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + ((3 * sizeof(GLfloat)) + (3 * sizeof(GLfloat))));
  
#endif

  size_t interleaved_element_buffer_size = (foo->m_NumBatched) * sizeof(GLshort);
  size_t interleaved_buffer_size = (foo->m_NumBatched * foo->m_Stride);

  glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, foo->m_ModelFoos);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, interleaved_element_buffer_size, foo->m_IndexFoo);

  glDrawElements(GL_TRIANGLES, foo->m_NumBatched, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

  if (false) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 0.0, 1.0);
    glPointSize(5.0);
    glDrawElements(GL_POINTS, foo->m_NumBatched, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
  }

  sf->m_LastBufferIndex++;
  if (sf->m_LastBufferIndex > (foo->m_BufferCount - 1)) {
    sf->m_LastBufferIndex = 0;
  }

  foo->m_NumBatched = 0;

}


void Model::Render(StateFoo *sf, foofoo *batch_foo) {
  int num_faces_times_vertices_per_face = (m_FooFoo->m_numFaces * 3);
  int frame_offset = (m_Frame * (num_faces_times_vertices_per_face));
  ModelFoo *l = NULL;
  ModelFoo *r = NULL;
  for (int i=0; i<(num_faces_times_vertices_per_face); i++) {
    int frame_offset_plus_index = frame_offset + i;
    l = &batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)];
    r = &m_FooFoo->m_ModelFoos[frame_offset_plus_index];
    l->vertex[0] = (r->vertex[0] * m_Scale[0]) + m_Position[0];
    l->vertex[1] = (r->vertex[1] * m_Scale[1]) + m_Position[1];
    l->vertex[2] = (r->vertex[2] * m_Scale[2]) + m_Position[2];
    l->normal[0] = r->normal[0];
    l->normal[1] = r->normal[1];
    l->normal[2] = r->normal[2];
    l->texture[0] = r->texture[0];
    l->texture[1] = r->texture[1];
    l->texture[2] = r->texture[2];
    batch_foo->m_IndexFoo[(batch_foo->m_NumBatched)] = batch_foo->m_NumBatched;
    batch_foo->m_NumBatched++;
  }
}


float Model::Simulate(float st, float dt, bool pushing) {
	m_IsPushing = pushing;

	if (m_FooFoo->m_numFrames > 1) {
		if (m_Life > (1.0 / (float)m_Fps)) {
			m_Frame++;
			m_Life = 0.0;
		}
		
		if (m_Frame >= m_FooFoo->m_numFrames) {
			m_Frame = 0;
		}
	}

	return m_Life;
}
