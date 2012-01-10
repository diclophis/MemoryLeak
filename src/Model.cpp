/*
 *  Model.cpp
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
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
	foofoo *ff = new foofoo;
  ff->m_Texture = texture_index;
  ff->m_numFaces = max_face_count * max_model_count;
  ff->m_ModelFoos = (ModelFoo *)malloc(ff->m_numFaces * 3 * sizeof(ModelFoo));
  ff->m_IndexFoo = (GLshort *)malloc((ff->m_numFaces * 3) * sizeof(GLshort));

  ff->m_numVertexArrayObjects = 1;
	ff->m_VertexArrayObjects = (GLuint*)calloc((ff->m_numVertexArrayObjects), sizeof(GLuint));

  ff->m_numInterleavedBuffers = 1;
	ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numInterleavedBuffers));
	glGenBuffers(ff->m_numInterleavedBuffers, ff->m_InterleavedBuffers);

  size_t size_of_model_foo = sizeof(ModelFoo);
  ff->m_Stride = size_of_model_foo;

  ff->m_numIndexBuffers = 1;
  ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numIndexBuffers));
  glGenBuffers(ff->m_numIndexBuffers, ff->m_IndexBuffers);

  /*
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[0]);
  size_t interleaved_element_buffer_size = (ff->m_numFaces * 3) * sizeof(GLshort);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, interleaved_element_buffer_size, NULL, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, ff->m_InterleavedBuffers[0]);
  size_t interleaved_buffer_size = (ff->m_numFaces * 3) * sizeof(ModelFoo);
  glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_STATIC_DRAW);
  */

  return ff;
}


foofoo *Model::GetFoo(const aiScene *a, int s, int e) {
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
	ff->m_ModelFoos = (ModelFoo *)malloc((ff->m_numFaces * 3 * ff->m_numFrames) * sizeof(ModelFoo));
  ff->m_IndexFoo = (GLshort *)malloc((ff->m_numFaces * 3) * sizeof(GLshort));

	if (a->mMeshes[0]->HasTextureCoords(0)) {
	} else {
    LOGV("no tex coords\n");
		exit(1);
	}

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


void Model::RenderFoo(StateFoo *sf, foofoo *foo) {

	if (foo->m_Texture != sf->g_lastTexture) {
		sf->g_lastTexture = foo->m_Texture;
		glBindTexture(GL_TEXTURE_2D, sf->g_lastTexture);
	}

#ifdef HAS_VAO
  if (foo->m_VertexArrayObjects[0] == 0) {
    glGenVertexArraysOES(1, &foo->m_VertexArrayObjects[0]);
    sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[0];
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
    sf->g_lastElementBuffer = foo->m_IndexBuffers[0];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
    glVertexPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + (0));
    glNormalPointer(GL_FLOAT, foo->m_Stride, (char *)(NULL) + (3 * sizeof(GLfloat)));
    glTexCoordPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + ((3 * sizeof(GLfloat)) + (3 * sizeof(GLfloat))));
  } else {
    if (foo->m_VertexArrayObjects[0] != sf->g_lastVertexArrayObject) {
      sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[0];
      glBindVertexArrayOES(sf->g_lastVertexArrayObject);
    }
  }
#else
  if (foo->m_IndexBuffers[0] != sf->g_lastElementBuffer) {
    sf->g_lastElementBuffer = foo->m_IndexBuffers[0];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
  }

  if (foo->m_InterleavedBuffers[0] != sf->g_lastInterleavedBuffer) {
    sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
  }

  glVertexPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + (0));
	glNormalPointer(GL_FLOAT, foo->m_Stride, (char *)(NULL) + (3 * sizeof(GLfloat)));
  glTexCoordPointer(3, GL_FLOAT, foo->m_Stride, (char *)NULL + ((3 * sizeof(GLfloat)) + (3 * sizeof(GLfloat))));
#endif

  size_t interleaved_element_buffer_size = (foo->m_NumBatched) * sizeof(GLshort);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, interleaved_element_buffer_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, interleaved_element_buffer_size, foo->m_IndexFoo);


  size_t interleaved_buffer_size = (foo->m_NumBatched * foo->m_Stride);
  glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, foo->m_ModelFoos);


  glDrawElements(GL_TRIANGLES, foo->m_NumBatched, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

  if (false) {
    glDisable(GL_TEXTURE_2D);
    glColor4f(1.0, 1.0, 0.0, 1.0);
    glPointSize(5.0);
    glDrawElements(GL_POINTS, foo->m_NumBatched, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
  }


}


void Model::Render(StateFoo *sf, foofoo *batch_foo) {
  
  //memcpy(&batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)], &m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + 0], (m_FooFoo->m_numFaces * 3) * sizeof(ModelFoo));

  for (int i=0; i<(m_FooFoo->m_numFaces * 3); i++) {
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].vertex[0] = (m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].vertex[0] * m_Scale[0]) + m_Position[0];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].vertex[1] = (m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].vertex[1] * m_Scale[1]) + m_Position[1];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].vertex[2] = (m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].vertex[2] * m_Scale[2]) + m_Position[2];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].normal[0] = m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].normal[0];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].normal[1] = m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].normal[1];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].normal[2] = m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].normal[2];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].texture[0] = m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].texture[0];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].texture[1] = m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].texture[1];
    batch_foo->m_ModelFoos[(batch_foo->m_NumBatched)].texture[2] = m_FooFoo->m_ModelFoos[(m_Frame * (m_FooFoo->m_numFaces * 3)) + i].texture[2];
    batch_foo->m_IndexFoo[(batch_foo->m_NumBatched)] = batch_foo->m_NumBatched;
    batch_foo->m_NumBatched++;
  }
}


bool Model::IsCollidedWith(Model *other) {
	bool cx = false;
	bool cy = false;
	bool cz = false;

	float other_s;
	float other_b;

	other_s = 0.4;
	other_b = 0.4;

	float mlx = m_Position[0] - other_s;
	float mrx = m_Position[0] + other_s;
	float olx = other->m_Position[0] - other_b; 
	float orx = other->m_Position[0] + other_b; 
	cx = ((olx >= mlx && olx <= mrx) || (orx >= mlx && orx <= mrx));

	float mlz = m_Position[2] - other_s;
	float mrz = m_Position[2] + other_s;
	float olz = other->m_Position[2] - other_b; 
	float orz = other->m_Position[2] + other_b; 
	cz = ((olz >= mlz && olz <= mrz) || (orz >= mlz && orz <= mrz));

	float mly = m_Position[1] - 0.5;
	float mry = m_Position[1] + 0.5;
	float oly = other->m_Position[1] - 0.5; 
	float ory = other->m_Position[1] + 0.5; 
	cy = ((oly >= mly && oly <= mry) || (ory >= mly && ory <= mry));

	if (cx && cz && cy) {
		if (other->m_IsStuck) {
			if ((m_Position[1] - 0.5) < (other->m_Position[1] + 0.5)) {
				float dy = (other->m_Position[1] + 0.5) - (m_Position[1] - 0.5);
				m_Position[1] += dy;
			}
		}
		return true;
	} else {
		return false;
	}
}


float Model::Simulate(float dt, bool pushing) {
	m_IsPushing = pushing;
	m_Life += dt;

  /*
	if (m_FooFoo->m_numFrames > 1) {
		if (m_Life > (1.0 / (float)m_Fps)) {
			m_Frame++;
			m_Life = 0.0;
		}
		
		if (m_Frame >= m_FooFoo->m_numFrames) {
			m_Frame = 0;
		}
	}
  */

  m_Position[1] = fastSinf(m_Life * 3.0) * 175.0;
  
  {
		//float tx = -sin(DEGREES_TO_RADIANS(m_Rotation[1]));
		//float tz = cos(DEGREES_TO_RADIANS(m_Rotation[1]));
		//m_Position[0] += tx * (m_Velocity[0] * dt);
		//m_Position[1] += (m_Velocity[1] * dt);
		//m_Position[2] += tz * (m_Velocity[0] * dt);
	}
	
	return m_Life;
}


void Model::Die(float dt) {
	SetVelocity(0.0, 0.0, 0.0);
	ScaleTo(0.1, 0.1, 0.1, dt);
}


void Model::Live(float dt) {
	m_Life += dt;
	if (m_Climbing) {
	} else if (m_IsFalling) {
	} else if (!m_IsPushing) {
		if (m_IsMoving) {
			MoveTo(m_Velocity[0], m_Velocity[1], m_Velocity[2], dt);
		} else {
			if (m_Steps->size() > 1) {
				int ax, ay;
				micropather::ModelOctree::NodeToXY(m_Steps->at(1), &ax, &ay);
				float sx = m_Position[0];
				float sy = m_Position[1];
				float sz = m_Position[2];
				float dx = ax - sx;
				float dy = sy - sy;
				float dz = ay - sz;
				if (dx != 0.0 || dy != 0.0 || dz != 0.0) {
					SetVelocity(sx + dx, sy + dy, sz + dz);
					m_Steps->erase(m_Steps->begin());
					m_IsMoving = true;
				}
			} else {
				m_IsMoving = false;
			}
		}
	} else {
		LOGV("stuck\n");
	}
}


void Model::Harm(Model *other) {
	//do damage and stuff
	other->SetVelocity(0.0, 0.0, 0.0);
	other->m_Life -= 100.0;
}


void Model::Help(Model *other, float dt) {
  //shield and stuff
}


void Model::CollideWith(Model *other, float dt) {
}


void Model::Move(int direction) {
	if (IsMovable()) {
		switch (direction) {
			case 0:
				SetVelocity(m_Position[0] + 1.0, m_Position[1], m_Position[2]);
				break;
			case 1:
				SetVelocity(m_Position[0], m_Position[1], m_Position[2] + 1.0);
				break;
			case 2:
				SetVelocity(m_Position[0] - 1.0, m_Position[1], m_Position[2]);
				break;
			case 3:
				SetVelocity(m_Position[0], m_Position[1], m_Position[2] - 1.0);
				break;
			default:
				break;
		}
		m_Direction = direction;
		m_IsMoving = true;
	}
}


bool Model::MoveTo(float x, float y, float z, float dt) {
	float dx = m_Position[0] - x;
	float dy = m_Position[1] - y;
	float dz = m_Position[2] - z;
	float tx = 0.0;
	float ty = 0.0;
	float tz = 0.0;
	bool done = false;
	if (fabs(dx) > 0.05 || fabs(dy) > 0.05 || fabs(dz) > 0.05) {
		tx = -((dx) * dt * 5.0);
		ty = -((dy) * dt * 5.0);
		tz = -((dz) * dt * 5.0);
		done = false;
	} else {
		tx = -dx;
		ty = -dy;
		tz = -dz;
		m_Velocity[0] = 0;
		m_Velocity[1] = 0;
		m_Velocity[2] = 0;
		m_IsMoving = false;
		done = true;
	}
	m_Position[0] += tx;
	m_Position[1] += ty;
	m_Position[2] += tz;
	return done;
}


bool Model::ClimbTo(float y, float dt) {
	m_IsMoving = false;
	m_Position[1] = m_Position[1] + (y * dt);
	return true;
}
