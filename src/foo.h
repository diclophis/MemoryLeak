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

#ifdef __cplusplus
extern "C" {
#endif

struct foofoo {

#ifdef __cplusplus

	//! Default constructor
	foofoo()
	{
	}

	//! Default destructor. Delete the index array
	~foofoo()
	{
    glDeleteBuffers(m_numFrames, m_VerticeBuffers);
    glDeleteBuffers(m_numNormalBuffers, m_NormalBuffers);
    glDeleteBuffers(m_numFrames, m_IndexBuffers);
    glDeleteBuffers(m_numTextureBuffers, m_TextureBuffer);
    free(m_VerticeBuffers);
    if (m_numNormalBuffers > 0) {
      free(m_NormalBuffers);
    }
    free(m_IndexBuffers);
    free(m_TextureBuffer);
	}

#endif

	int m_numBuffers;
	GLuint *m_VerticeBuffers;
	GLuint *m_NormalBuffers;
	GLuint *m_IndexBuffers;
	GLuint *m_TextureBuffer;
	int m_numFaces;
	int m_numFrames;
  int m_numTextureBuffers;
  int m_numNormalBuffers;
	int m_AnimationStart;
	int m_AnimationEnd;
};

#ifdef __cplusplus
}
#endif
