/*
 *  Model.cpp
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#include "MemoryLeak.h"


static GLuint g_lastTexture = 0;
static GLuint g_lastVertexBuffer = 0;
static GLuint g_lastNormalBuffer = 0;
static GLuint g_lastTexcoordBuffer = 0;
static GLuint g_lastElementBuffer = 0;

//static GLuint g_staticVertexBuffer = 0;
//static GLuint g_staticNormalBuffer = 0;
//static GLuint g_staticTexcoordBuffer = 0;
//static GLuint g_staticElementBuffer = 0;

void Model::ReleaseBuffers() {
	g_lastVertexBuffer = 0;
	g_lastNormalBuffer = 0;
	g_lastTexcoordBuffer = 0;
	g_lastElementBuffer = 0;
	g_lastTexture = 0;
	
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


Model::Model(const foofoo *a, int t, bool u) : m_FooFoo(a), m_UsesStaticBuffer(u) {
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

  SetTexture(t);
	SetScale(1.0, 1.0, 1.0);
	SetPosition(0.0, 0.0, 0.0);
	SetRotation(0.0, 0.0, 0.0);
	SetVelocity(0.0, 0.0, 0.0);
	m_Steps = new std::vector<void *>;
	
	m_FramesOfAnimationCount = m_FooFoo->m_numFrames; //m_FooFoo->m_AnimationEnd - m_FooFoo->m_AnimationStart;	
}


//foofoos must contain #of frame info, look into replace interp
//implement addverticestofoofoobuffersatposition,rotation,scale,frame
foofoo *Model::GetFoo(const aiScene *a, int s, int e) {
  //glEnable(GL_DEPTH_TEST);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  
	foofoo *ff = new foofoo;
  
  int interp = 3;
	if (a->mNumMeshes > 1) {
		//ff->m_numFrames = ((a->mRootNode->mNumMeshes - 1) * interp);
    ff->m_numFrames = (e - s) * interp;
	} else {
		interp = 1;
		ff->m_numFrames = a->mNumMeshes;
	}
	
	ff->m_numFaces = a->mMeshes[0]->mNumFaces;
	ff->m_numBuffers = ff->m_numFrames;
	ff->m_VerticeBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_NormalBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));

	ff->m_TextureBuffer = (GLuint*)malloc(sizeof(GLuint) * (1));
	ff->m_AnimationStart = s;
	ff->m_AnimationEnd = e;
	
	glGenBuffers(ff->m_numBuffers, ff->m_VerticeBuffers);
	glGenBuffers(ff->m_numBuffers, ff->m_NormalBuffers);
	glGenBuffers(ff->m_numBuffers, ff->m_IndexBuffers);

	glGenBuffers(1, ff->m_TextureBuffer);

	const aiMesh& aimesh = *a->mMeshes[0];

	if (aimesh.HasTextureCoords(0)) {
		glBindBuffer(GL_ARRAY_BUFFER, ff->m_TextureBuffer[0]);
		glBufferData(GL_ARRAY_BUFFER, aimesh.mNumVertices * 3 * sizeof(float), aimesh.mTextureCoords[0], GL_STATIC_DRAW);
	} else {
    LOGV("no tex coords\n");
		exit(1);
	}

	int used_buffer = 0;
	
	for (int mm=ff->m_AnimationStart; mm<ff->m_AnimationEnd; mm++) { //keyframes
		for (int iiii=0; iiii<interp; iiii++) {
			
			unsigned short* indices = new unsigned short[a->mMeshes[mm]->mNumFaces * 3];
			float* vertices = new float[a->mMeshes[mm]->mNumVertices * 3];

			for(unsigned int i=0,j=0; i<a->mMeshes[mm]->mNumFaces; ++i,j+=3) {
				indices[j]   = a->mMeshes[mm]->mFaces[i].mIndices[0];
				indices[j+1] = a->mMeshes[mm]->mFaces[i].mIndices[1];
				indices[j+2] = a->mMeshes[mm]->mFaces[i].mIndices[2];
			}
		
      
			if (iiii == 0) {
				for(unsigned int ik=0,jk=0; ik<a->mMeshes[mm]->mNumVertices; ++ik, jk+=3) {
					vertices[jk] = a->mMeshes[mm]->mVertices[ik][0];
					vertices[jk+1] = a->mMeshes[mm]->mVertices[ik][1];
					vertices[jk+2] = a->mMeshes[mm]->mVertices[ik][2];
				}
      }

      /*
				
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[used_buffer]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, a->mMeshes[mm]->mNumFaces * 3 * sizeof(short), indices, GL_STATIC_DRAW);
				
				glBindBuffer(GL_ARRAY_BUFFER, ff->m_VerticeBuffers[used_buffer]);
				glBufferData(GL_ARRAY_BUFFER, a->mMeshes[mm]->mNumVertices * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
				
				glBindBuffer(GL_ARRAY_BUFFER, ff->m_NormalBuffers[used_buffer]);
				glBufferData(GL_ARRAY_BUFFER, a->mMeshes[mm]->mNumVertices * 3 * sizeof(float), a->mMeshes[mm]->mNormals, GL_STATIC_DRAW);
				used_buffer++;
			} else {
      */
				//if (mm < (a->mRootNode->mNumMeshes - 1)) {
					float percent_of_way = (float)iiii / (float)interp;

					for(unsigned int ik=0,jk=0; ik<a->mMeshes[mm]->mNumVertices; ++ik, jk+=3) {
						vertices[jk+0] = a->mMeshes[mm]->mVertices[ik][0] + (percent_of_way * (a->mMeshes[mm + 1]->mVertices[ik][0] - a->mMeshes[mm]->mVertices[ik][0]));
						vertices[jk+1] = a->mMeshes[mm]->mVertices[ik][1] + (percent_of_way * (a->mMeshes[mm + 1]->mVertices[ik][1] - a->mMeshes[mm]->mVertices[ik][1]));
						vertices[jk+2] = a->mMeshes[mm]->mVertices[ik][2] + (percent_of_way * (a->mMeshes[mm + 1]->mVertices[ik][2] - a->mMeshes[mm]->mVertices[ik][2]));
					}
					
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[used_buffer]);
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, a->mMeshes[mm]->mNumFaces * 3 * sizeof(short), indices, GL_STATIC_DRAW);
					
					glBindBuffer(GL_ARRAY_BUFFER, ff->m_VerticeBuffers[used_buffer]);
					glBufferData(GL_ARRAY_BUFFER, a->mMeshes[mm]->mNumVertices * 3 * sizeof(float), vertices, GL_STATIC_DRAW);
					
					glBindBuffer(GL_ARRAY_BUFFER, ff->m_NormalBuffers[used_buffer]);
					glBufferData(GL_ARRAY_BUFFER, a->mMeshes[mm]->mNumVertices * 3 * sizeof(float), a->mMeshes[mm]->mNormals, GL_STATIC_DRAW);
					used_buffer++;
				//}
			//}
			
			delete vertices;
			delete indices;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
	//glDisable(GL_DEPTH_TEST);
  
	return ff;
}


void Model::Render() {
  
  //NOTE: this checks the depth buffer
  //GLint f;
  //glGetIntegerv(GL_DEPTH_BITS, &f);
  
  glEnableClientState(GL_VERTEX_ARRAY);

  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnable(GL_NORMALIZE);
  
	glPushMatrix();
	{
		glTranslatef(m_Position[0],m_Position[1],m_Position[2]);
		glRotatef(m_Rotation[1],0,-1,0);
		glRotatef(m_Rotation[0],0,0,1);
		glRotatef(m_Rotation[2],1,0,0);
		glScalef(m_Scale[0],m_Scale[1],m_Scale[2]);
	
		if (m_Texture != g_lastTexture) {
			glBindTexture(GL_TEXTURE_2D, m_Texture);
			g_lastTexture = m_Texture;
		}

		if (m_FooFoo->m_VerticeBuffers[m_Frame] != g_lastVertexBuffer) {
			g_lastVertexBuffer = m_FooFoo->m_VerticeBuffers[m_Frame];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastVertexBuffer);
      
			glVertexPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));

		}

		if (m_FooFoo->m_NormalBuffers[m_Frame] != g_lastNormalBuffer) {
			g_lastNormalBuffer = m_FooFoo->m_NormalBuffers[m_Frame];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastNormalBuffer);

			glNormalPointer(GL_FLOAT, 0, (GLvoid*)((char*)NULL)	);

		}

		if (m_FooFoo->m_TextureBuffer[0] != g_lastTexcoordBuffer) {
			g_lastTexcoordBuffer = m_FooFoo->m_TextureBuffer[0];
			glBindBuffer(GL_ARRAY_BUFFER, g_lastTexcoordBuffer);

			glTexCoordPointer(3, GL_FLOAT, 0, (GLvoid*)((char*)NULL));

		}

		if (m_FooFoo->m_IndexBuffers[m_Frame] != g_lastElementBuffer) {
			g_lastElementBuffer = m_FooFoo->m_IndexBuffers[m_Frame];
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);

		}
		glDrawElements(GL_TRIANGLES, (3 * m_FooFoo->m_numFaces), GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    //glDrawElements(GL_LINES, (3 * m_FooFoo->m_numFaces), GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

	}
	glPopMatrix();
  
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_NORMALIZE);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
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
	
	
	if (m_FooFoo->m_numFrames > 1) {
		m_Life += dt;
		if (m_Life > (1.0 / (float)m_Fps)) {
			m_Frame++;
			m_Life = 0.0;
		}
		
		if (m_Frame >= m_FramesOfAnimationCount) {
			m_Frame = 0;
		}
	} else {
		float tx = -sin(DEGREES_TO_RADIANS(m_Rotation[1]));
		float tz = cos(DEGREES_TO_RADIANS(m_Rotation[1]));
		m_Position[0] += tx * (m_Velocity[0] * dt);
		m_Position[1] += (m_Velocity[1] * dt);
		//m_Position[1] = 0.7 + (fastSinf(m_Life) * 0.1);
		m_Position[2] += tz * (m_Velocity[0] * dt);
		
		//LOGV("vy: %f\n", m_Velocity[1]);
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
		//LOGV("climbing\n");
	} else if (m_IsFalling) {
		//LOGV("falling\n");
	} else if (!m_IsPushing) {
		if (m_IsMoving) {
			//LOGV("moving\n");
			MoveTo(m_Velocity[0], m_Velocity[1], m_Velocity[2], dt);
		} else {
			if (m_Steps->size() > 1) {
				//LOGV("stepping\n");
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
