/*
 *  foo.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 9/6/10.
 *  GPL
 *
 */

static GLuint g_AtlasSpriteIndexBuffer = 0;

struct foo {
	FILE *fp;
	unsigned int off;
	unsigned int len;
};

#ifdef __cplusplus
extern "C" {
#endif

struct foofoo {

#ifdef __cplusplus

	//! Default constructor
	foofoo()
	{
    m_AnimationDuration = 0;
    m_numFaces = 0;
    m_numFrames = 0;
    m_numInterleavedBuffers = 0;
    m_numTextureBuffers = 0;
    m_numNormalBuffers = 0;
    m_AnimationStart = 0;
    m_AnimationEnd = 0;
    m_Stride = 0;
    m_numVertexArrayObjects = 0;
    m_numIndexBuffers = 1;
	  m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (m_numIndexBuffers));
    if (g_AtlasSpriteIndexBuffer == 0) {
      glGenBuffers(m_numIndexBuffers, m_IndexBuffers);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffers[0]);
      GLushort *indices;
      indices = (GLushort *) malloc(4 * sizeof(GLushort));
      indices[0] = 1;
      indices[1] = 2;
      indices[2] = 0;
      indices[3] = 3;
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLshort), indices, GL_STATIC_DRAW);
      free(indices);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      g_AtlasSpriteIndexBuffer = m_IndexBuffers[0];
    } else {
      m_IndexBuffers[0] = g_AtlasSpriteIndexBuffer;
    }
	}

	//! Default destructor. Delete the index array
	~foofoo()
	{
    glDeleteBuffers(m_numBuffers, m_VerticeBuffers);
    glDeleteBuffers(m_numBuffers, m_IndexBuffers);
    glDeleteBuffers(m_numNormalBuffers, m_NormalBuffers);
    glDeleteBuffers(m_numTextureBuffers, m_TextureBuffer);
    glDeleteBuffers(m_numInterleavedBuffers, m_InterleavedBuffers);
    if (m_numBuffers > 0) {
      free(m_VerticeBuffers);
    }
    if (m_numIndexBuffers > 0) {
      free(m_IndexBuffers);
    }
    if (m_numNormalBuffers > 0) {
      free(m_NormalBuffers);
    }
    if (m_numTextureBuffers > 0) {
      free(m_TextureBuffer);
    }
    if (m_numInterleavedBuffers > 0) {
      free(m_InterleavedBuffers);
    }
	}

#endif

	int m_numBuffers;
	GLuint *m_VerticeBuffers;
	GLuint *m_NormalBuffers;
	GLuint *m_IndexBuffers;
	GLuint *m_TextureBuffer;
	GLuint *m_InterleavedBuffers;
	GLuint *m_VertexArrayObjects;
	GLuint m_Texture;
  float m_AnimationDuration;
	int m_numFaces;
	int m_numFrames;
  int m_numInterleavedBuffers;
  int m_numTextureBuffers;
  int m_numNormalBuffers;
  int m_numIndexBuffers;
  int m_numVertexArrayObjects;
	int m_AnimationStart;
	int m_AnimationEnd;
  size_t m_Stride;
};

#ifdef __cplusplus
}
#endif
