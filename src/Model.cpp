/*
 *  Model.cpp
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include "MemoryLeak.h"
#include "aiScene.h"
#include "Model.h"
#include "Engine.h"

static GLuint g_lastTexture = 0;
static GLuint g_lastVertexBuffer = 0;
static GLuint g_lastNormalBuffer = 0;
static GLuint g_lastTexcoordBuffer = 0;
static GLuint g_lastElementBuffer = 0;



Model::Model(const foofoo *a) : m_FooFoo(a) {
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

	m_Scale = (float *)malloc(3 * sizeof(float));
	m_Position = (float *)malloc(3 * sizeof(float));
	m_Rotation = (float *)malloc(3 * sizeof(float));
	m_Velocity = (float *)malloc(3 * sizeof(float));
  m_Climbing = NULL;
  m_IsFalling = false;
	
	SetScale(1.0, 1.0, 1.0);
	SetPosition(0.0, 0.0, 0.0);
	SetRotation(0.0, 0.0, 0.0);
	SetVelocity(0.0, 0.0, 0.0);
}


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
		for(unsigned int i=0,j=0; i<a->mMeshes[mm]->mNumFaces; ++i,j+=3) {
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


void Model::Render() {
	glPushMatrix();
	{
		glTranslatef(m_Position[0],m_Position[1],m_Position[2]);
		glRotatef(m_Rotation[0],0,0,1);
		glRotatef(m_Rotation[1],0,1,0);
		glRotatef(m_Rotation[2],1,0,0);
		glScalef(m_Scale[0],m_Scale[1],m_Scale[2]);
	
		if (m_Texture != g_lastTexture) {
			glBindTexture(GL_TEXTURE_2D, m_Texture);
			g_lastTexture = m_Texture;
		}

		if (m_FooFoo->m_VerticeBuffers[m_Frame + 1] != g_lastVertexBuffer) {
			g_lastVertexBuffer = m_FooFoo->m_VerticeBuffers[m_Frame + 1];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastVertexBuffer);
			glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
		}

		if (m_FooFoo->m_VerticeBuffers[m_Frame + 2] != g_lastNormalBuffer) {
			g_lastNormalBuffer = m_FooFoo->m_VerticeBuffers[m_Frame + 2];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastNormalBuffer);
			glNormalPointer(GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);
		}

		if (m_FooFoo->m_TextureBuffer[0] != g_lastTexcoordBuffer) {
			g_lastTexcoordBuffer = m_FooFoo->m_TextureBuffer[0];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastTexcoordBuffer);
			glTexCoordPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
		}

		if (m_FooFoo->m_VerticeBuffers[m_Frame + 0] != g_lastElementBuffer) {
			g_lastElementBuffer = m_FooFoo->m_VerticeBuffers[m_Frame + 0];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
		}

		glDrawElements(GL_TRIANGLES, 3 * m_FooFoo->m_numFaces, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
	}
	glPopMatrix();
}


bool Model::IsCollidedWith(Model *other) {

	float mlx = m_Position[0] - (0.5 * m_Scale[0]);
	float mrx = m_Position[0] + (0.5 * m_Scale[0]);
	float olx = other->m_Position[0] - (0.5 * other->m_Scale[0]); 
	float orx = other->m_Position[0] + (0.5 * other->m_Scale[0]); 
  bool cx = ((olx >= mlx && olx <= mrx) || (orx >= mlx && orx <= mrx));

	float mly = m_Position[1] - (0.5 * m_Scale[0]);
	float mry = m_Position[1] + (0.5 * m_Scale[0]);
	float oly = other->m_Position[1] - (0.5 * other->m_Scale[1]); 
	float ory = other->m_Position[1] + (0.5 * other->m_Scale[1]); 
  bool cy = ((oly >= mly && oly <= mry) || (ory >= mly && ory <= mry));

  //LOGV("%f %f %f %f %d %d\n", mly, mry, oly, ory, cx, cy);

	if (cx && cy) {
		return true;
	} else {
		return false;
	}
}

float Model::Simulate(float dt) {
  if (m_Life < 0.0) {
    Die(dt);
  } else {
    Live(dt);
  }
  
  return m_Life;
}

void Model::Die(float dt) {
  //shrink and disappear
  SetVelocity(0.0, 0.0, 0.0);
  ScaleTo(0.1, 0.1, 0.1, dt);
}

void Model::Live(float dt) {
  //move and what not
  m_Life += dt;

  if (m_Climbing) {
    if (ClimbTo(m_Climbing->m_Position[1] + m_Climbing->m_Scale[1], dt)) {
      m_Climbing = NULL;
    }
  } else if (m_IsFalling) {
    ClimbTo(m_Position[1] - 1.0, dt);
  } else {
    //m_Position[0] += m_Velocity[0] * dt;
    //m_Position[1] += m_Velocity[1] * dt;
    //m_Position[2] += m_Velocity[2] * dt;
	  if (m_IsPlayer) {
		  if (m_Velocity[0] != 0.0 || m_Velocity[2] != 0.0) {
			  //LOGV("(%f, %f) => MoveTo(%f, %f) => Over(%f)\n", m_Position[0], m_Position[2], m_Velocity[0], m_Velocity[2], dt);
			  MoveTo(m_Velocity[0], m_Velocity[2], dt);
		  }
	  }
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
	LOGV("move(%d)\n", direction);
	if (!IsMoving()) {
  switch (direction) {
    case 0:
		  SetVelocity(m_Position[0] + 1.0, 0.0, m_Position[2]);
      break;
	case 1:
		  SetVelocity(m_Position[0], 0.0, m_Position[2] + 1.0);
	  break;
	case 2:
		  SetVelocity(m_Position[0] - 1.0, 0.0, m_Position[2]);
	  break;
	case 3:
		  SetVelocity(m_Position[0], 0.0, m_Position[2] - 1.0);
	  break;
    default:
      break;
  }
	}
}

bool Model::MoveTo(float x, float z, float dt) {
	float dx = m_Position[0] - x;
	float dz = m_Position[2] - z;
	if (fabs(dx) > 0.05 || fabs(dz) > 0.05) {
		//LOGV("tween\n");
		m_Position[0] = m_Position[0] - ((dx) * dt * 10.0);
		m_Position[2] = m_Position[2] - ((dz) * dt * 10.0);
		return false;
	} else {
		m_Position[0] = x;
		m_Position[2] = z;
		m_Velocity[0] = 0;
		m_Velocity[2] = 0;
		return true;
	}
}

bool Model::ClimbTo(float y, float dt) {
  //lerp from here to there at sx
  float dy = m_Position[1] - y;
  //LOGV("%f\n", dy);
  if (fabs(dy) > 0.05) {
    m_Position[1] = m_Position[1] - ((1.1 * dy) * dt);
    return false;
  } else {
    m_Position[1] = y;
    return true;
  }
}
