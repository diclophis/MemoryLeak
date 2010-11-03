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
	Model(const aiScene *a) : m_Scene(a) {
		SetPosition(0.0, 0.0, 0.0);
		SetRotation(0.0, 0.0, 0.0);
		SetScale(1.0, 1.0, 1.0);
		build();
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
	
	/// \brief	returns the current position of the model at the current moment
	/// \return	the position array (3 floats; x,y,z)
	///
	const float* GetPosition() const {
		return m_Position;
	}
	
	
private:
	
	bool build();
	const aiScene *m_Scene;
	int numVBO;
	GLuint *vboID;
	GLuint *m_TextureBuffer;
	float m_Scale[3];
	float m_Position[3];
	float m_Rotation[3];
	int mNumFaces;
  int numFrames;
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
