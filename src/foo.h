/*
 *  foo.h
 *  MemoryLeak
 *
 *  Created by Jon Bardin on 9/6/10.
 *  GPL
 *
 */

struct foo {
	FILE *fp;
	unsigned int off;
	unsigned int len;
};

typedef struct
{
  GLshort vertex[2];
  GLfloat texture[2];
} SpriteFoo;

typedef struct
{
  GLshort vertex[3];
  GLfloat normal[3];
  GLfloat texture[2];
} ModelFoo;

typedef struct {
  GLuint g_lastTexture;
  GLuint g_lastElementBuffer;
  GLuint g_lastInterleavedBuffer;
  GLuint g_lastVertexArrayObject;
} StateFoo;

#ifdef __cplusplus
extern "C" {
#endif

struct foofoo {

#ifdef __cplusplus

	foofoo()
	{
    m_AnimationDuration = 0;
    m_numFaces = 0;
    m_numFrames = 0;
    m_numInterleavedBuffers = 0;
    m_numTextureBuffers = 0;
    m_numNormalBuffers = 0;
    m_numIndexBuffers = 0;
    m_AnimationStart = 0;
    m_AnimationEnd = 0;
    m_Stride = 0;
    m_numVertexArrayObjects = 0;
    m_numBuffers = 0;
    m_NumBatched = 0;
    m_NumBatchedElements = 0;
    m_numSpriteFoos = 0;
    m_numModelFoos = 0;
	}

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
    if (m_numSpriteFoos > 0) {
      free(m_SpriteFoos);
    }
    if (m_numModelFoos > 0) {
      free(m_ModelFoos);
    }
#ifdef HAS_VAO
    glDeleteVertexArraysOES(m_numVertexArrayObjects, m_VertexArrayObjects);
#endif
    if (m_numVertexArrayObjects > 0) {
      free(m_VertexArrayObjects);
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
  int m_numSpriteFoos;
  int m_numModelFoos;
	int m_AnimationStart;
	int m_AnimationEnd;
  int m_NumBatched;
  int m_NumBatchedElements;
  size_t m_Stride;
  SpriteFoo *m_SpriteFoos;
  ModelFoo *m_ModelFoos;
};

#ifdef __cplusplus
}
#endif
