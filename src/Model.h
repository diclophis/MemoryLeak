/*
 *  Model.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 11/1/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

class Model {


public:
	
	static foofoo *GetFoo(const aiScene *a);
	
	Model(const foofoo *a) : m_FooFoo(a) {
		SetPosition(0.0, 0.0, 0.0);
		SetRotation(0.0, 0.0, 0.0);
		SetScale(1.0, 1.0, 1.0);
	};
	
	void render(int frame);
	
	void SetPosition(float x,float y,float z) {
		m_Position[0] = x;
		m_Position[1] = y;
		m_Position[2] = z;
	}
	
	
	void SetScale(float x,float y,float z) {
		m_Scale[0] = x;
		m_Scale[1] = y;
		m_Scale[2] = z;
	}
	
	
	void SetRotation(float x, float y, float z) {
		m_Rotation[0] = x;
		m_Rotation[1] = y;
		m_Rotation[2] = z;
	}
	
	
	const float* GetPosition() const {
		return m_Position;
	}
	
	
	float Tick(float delta) {
		m_Position[0] += m_Velocity[0] * delta;
		m_Position[1] += m_Velocity[1] * delta;
		m_Position[2] += m_Velocity[2] * delta;
		m_Life += delta;
		return m_Life;
	}
	
	
	void SetLife(float life) {
		m_Life = life;
	}
	
	
	float GetLife() {
		return m_Life;
	}
	
	
	float sfrand() {
		static unsigned int mirand = 1;
		unsigned int a;
		mirand *= 16807;
		a = (mirand&0x007fffff) | 0x40000000;
		return( *((float*)&a) - 3.0f );
	}
	
	
	void SetVelocity(float x, float y, float z) {
		m_Velocity[0] = x;
		m_Velocity[1] = y;
		m_Velocity[2] = z;
	}

  void SetTexture(int t) {
    m_Texture = t;
  }

  void SetFrame(int f) {
    m_Frame = f;
  }
	
protected:
	
	const foofoo *m_FooFoo;
	
	/*
	int numVBO;
	GLuint *vboID;
	GLuint *m_TextureBuffer;

	int mNumFaces;
  int numFrames;
	 */
	
	float m_Life;
	float m_Scale[3];
	float m_Position[3];
	float m_Rotation[3];
	float m_Velocity[3];
  int m_Frame;
  int m_Texture;
};






/*
 
 |
 aiProcess_TransformUVCoords |
 aiProcess_GenUVCoords |
 aiProcess_CalcTangentSpace |
 aiProcess_GenNormals |
 aiProcess_GenSmoothNormals |
 aiProcess_SplitLargeMeshes |
 aiProcess_ImproveCacheLocality |
 aiProcess_FixInfacingNormals |
 aiProcess_OptimizeGraph |
 aiProcess_Triangulate |
 aiProcess_JoinIdenticalVertices |
 aiProcess_SortByPType
 
 */
/*
 aiProcess_OptimizeMeshes |
 
 aiProcess_OptimizeGraph |
 aiProcess_TransformUVCoords |
 aiProcess_GenUVCoords |
 aiProcess_CalcTangentSpace |
 aiProcess_GenNormals |
 aiProcess_GenSmoothNormals |
 aiProcess_SplitLargeMeshes |
 aiProcess_ImproveCacheLocality |
 aiProcess_FixInfacingNormals |
 */
