// Jon Bardin GPL


#include "MemoryLeak.h"


static GLuint g_lastTexture = 0;
static GLuint g_lastVertexBuffer = 0;
static GLuint g_lastTexcoordBuffer = 0;
static GLuint g_lastElementBuffer = 0;
static GLuint g_lastInterleavedBuffer = 0;
static GLuint g_vertexArrayObject = 0;
static int g_BufferCount = 0;


void AtlasSprite::ReleaseBuffers() {
  //g_lastVertexBuffer = -1;
  //g_lastTexcoordBuffer = -1;
  //g_lastElementBuffer = -1;
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


void AtlasSprite::Scrub() {
	//g_lastTexture = -1;
}


AtlasSprite::~AtlasSprite() {
  delete m_Position;
  delete m_Velocity;
  delete m_Scale;
}


AtlasSprite::AtlasSprite(foofoo *ff) : m_FooFoo(ff) {
  m_Fps = 0;
	m_Rotation = m_LastRotation = 0.0;
	m_Position = new float[2];
	m_Velocity = new float[2];
	m_Scale = new float[2];
	m_Scale[0] = 1.0;
	m_Scale[1] = 1.0;
	m_Position[0] = 0.0;
	m_Position[1] = 0.0;
	m_Velocity[0] = 0.0;
	m_Velocity[1] = 0.0;
	m_Life = 0.0;
	m_AnimationLife = 0.0;
	m_IsAlive = true;
	m_Frame = 0;
}


void AtlasSprite::Render() {
	if (m_FooFoo->m_numFrames == 0) {
    LOGV("Fail, animation is at least 1 frame\n");
    return;
  }

	if (m_FooFoo->m_Texture != g_lastTexture) {
		glBindTexture(GL_TEXTURE_2D, m_FooFoo->m_Texture);
		g_lastTexture = m_FooFoo->m_Texture;
	}

  glTranslatef(m_Position[0], m_Position[1], 0.0);
  
  //if (m_LastRotation != m_Rotation) {
    glRotatef(m_Rotation, 0.0, 0.0, 1.0);
    //m_LastRotation = m_Rotation;
  //}

  //without interleave
  /*
  if (m_FooFoo->m_VerticeBuffers[m_Frame] != g_lastVertexBuffer) {
    g_lastVertexBuffer = m_FooFoo->m_VerticeBuffers[m_Frame];
    glBindBuffer(GL_ARRAY_BUFFER, g_lastVertexBuffer);
    glVertexPointer(2, GL_SHORT, 0, (GLvoid*)((char*)NULL));
  }

  if (m_FooFoo->m_TextureBuffer[m_Frame] != g_lastTexcoordBuffer) {
    g_lastTexcoordBuffer = m_FooFoo->m_TextureBuffer[m_Frame];
    glBindBuffer(GL_ARRAY_BUFFER, g_lastTexcoordBuffer);
    glTexCoordPointer(2, GL_FLOAT, 0, (GLvoid*)((char*)NULL));
  }
  */

  //with interleave
  
  if (g_vertexArrayObject == 0) {
    glGenVertexArraysOES(1, &g_vertexArrayObject);
    glBindVertexArrayOES(g_vertexArrayObject);
    glVertexPointer(2, GL_SHORT, 0, (char *)NULL + (0));
    glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL + (8 * sizeof(GLshort)));
    glBindVertexArrayOES(0);
  }
  
  //glBindVertexArrayOES(g_vertexArrayObject);
  
  if (true) {
  //if (m_FooFoo->m_InterleavedBuffers[m_Frame] != g_lastInterleavedBuffer) {
    g_lastInterleavedBuffer = m_FooFoo->m_InterleavedBuffers[m_Frame];
    glBindBuffer(GL_ARRAY_BUFFER, g_lastInterleavedBuffer);
    glVertexPointer(2, GL_SHORT, 0, (char *)NULL + (0));
    glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL + (8 * sizeof(GLshort)));
  //}
  }
  
  if (true) {
  //if (m_FooFoo->m_IndexBuffers[m_Frame] != g_lastElementBuffer) {
    g_lastElementBuffer = m_FooFoo->m_IndexBuffers[m_Frame];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
  //}
  }
  
  glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
  //glDrawRangeElements(GL_TRIANGLE_STRIP, 0, 4, 4, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
  //glDrawArrays(GL_TRIANGLES, 0, 5);
  
  if (false) {
    glDisable(GL_TEXTURE_2D);
    glPointSize(1.0);
    glColor4f(0.0, 1.0, 0.0, 1.0);
    glDrawElements(GL_LINES, 4, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
  }

  glRotatef(-m_Rotation, 0.0, 0.0, 1.0);
  glTranslatef(-m_Position[0], -m_Position[1], 0.0);
}


void AtlasSprite::Simulate(float deltaTime) {
	float dx = m_Velocity[0] * deltaTime;
	float dy = m_Velocity[1] * deltaTime;
	m_Position[0] += dx;
	m_Position[1] += dy;
	m_Life += deltaTime;
	m_AnimationLife += deltaTime;
  if (m_IsAlive) {
    if (m_Fps > 0) {
      if (m_AnimationLife > (1.0 / (float)m_Fps)) {
        m_Frame++;
        m_AnimationLife = 0.0;
      }
      
      if (m_Frame < 0) {
        m_Frame = m_FooFoo->m_numFrames - 1;
      }
      
      if (m_Frame >= m_FooFoo->m_numFrames) {
        m_Frame = 0;
      }
    } else {
      m_Frame = fastAbs((((m_Life) / m_FooFoo->m_AnimationDuration) * m_FooFoo->m_numFrames));
      if (m_Frame >= m_FooFoo->m_numFrames) {
        m_Frame = m_FooFoo->m_numFrames - 1;
      }
    }
  }
}


foofoo *AtlasSprite::GetFoo(GLuint texture_index, int sprites_per_row, int rows, int start, int end, float life, float width, float height) {
  GLshort *vertices;
  GLfloat *texture;
  GLushort *indices;
	int *m_Frames;
	float duration = life + 0.1;
  int length = end - start;
  m_Frames = new int[length];

  for (unsigned int i=0; i<length; i++) {
    m_Frames[i] = start + i;
  }

	int total_count = sprites_per_row * rows;
	Sprite *m_Sprites;
	m_Sprites = new Sprite[length];

	GLfloat tdx = 1.0 / (float)sprites_per_row;
	GLfloat tdy = 1.0 / (float)rows;

	float texture_x = 0;
	float texture_y = 0;
	int ii = 0;

	for (unsigned int i=0; i<total_count; i++) {
		int b = (i % sprites_per_row);
		if (i == m_Frames[ii] && ii < length) {
			m_Sprites[ii].dx = width;
			m_Sprites[ii].dy = height;
			m_Sprites[ii].tx1 = texture_x;
			m_Sprites[ii].ty1 = texture_y;
			m_Sprites[ii].tx2 = texture_x + tdx;
			m_Sprites[ii].ty2 = texture_y + tdy;
			ii++;
		}
		texture_x += tdx;
		if (b == (sprites_per_row - 1)) {
			texture_x = 0;
			texture_y += tdy;
		}
	}

  delete m_Frames;

	foofoo *ff = new foofoo;
  ff->m_Texture = texture_index;
  ff->m_numFrames = length;
	ff->m_numBuffers = ff->m_numFrames;
  ff->m_numTextureBuffers = ff->m_numFrames;
  ff->m_numNormalBuffers = 0;
	//ff->m_VerticeBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	//ff->m_TextureBuffer = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_VertexArrays = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_AnimationStart = start;
	ff->m_AnimationEnd = end;
  ff->m_AnimationDuration = duration;

	//glGenBuffers(ff->m_numBuffers, ff->m_VerticeBuffers);
	//glGenBuffers(ff->m_numBuffers, ff->m_TextureBuffer);
	glGenBuffers(ff->m_numBuffers, ff->m_IndexBuffers);
	glGenBuffers(ff->m_numBuffers, ff->m_InterleavedBuffers);
	//glGenVertexArraysOES(ff->m_numBuffers, ff->m_VertexArrays);

  for (unsigned int i=0; i<length; i++) {
    GLshort w = m_Sprites[i].dx; 
    GLshort h = m_Sprites[i].dy; 
    vertices = (GLshort *) malloc(8 * sizeof(GLshort));
    vertices[0] =  (-w / 2);
    vertices[1] = (-h / 2);
    vertices[2] = (w / 2);
    vertices[3] = (-h / 2);
    vertices[4] = (w / 2);
    vertices[5] = (h / 2);
    vertices[6] = (-w / 2);
    vertices[7] = (h / 2);

    GLfloat tx = m_Sprites[i].tx1;
    GLfloat ty = m_Sprites[i].ty1;
    GLfloat tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
    GLfloat th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);

    texture = (GLfloat *) malloc(8 * sizeof(GLfloat));
    texture[0] = tx;
    texture[1] = (ty + th);
    texture[2] = tx + tw;
    texture[3] = (ty + th);
    texture[4] = tx + tw;
    texture[5] = ty;
    texture[6] = tx;
    texture[7] = ty;

    indices = (GLushort *) malloc(4 * sizeof(GLushort));
    indices[0] = 1;
    indices[1] = 2;
    indices[2] = 0;
    indices[3] = 3;


    size_t interleaved_buffer_size = (8 * sizeof(GLshort)) + (8 * sizeof(GLfloat));
	  ff->m_Stride = interleaved_buffer_size;

    //glBindVertexArrayOES(ff->m_VertexArrays[i]);

    //glVertexAttribPointerOES(ATT_POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(staticFmt), (void*)offsetof(staticFmt,position));
    //glEnableVertexAttribArray(ATT_POSITION);
    //glVertexAttribPointer(ATT_TEXCOORD, 2, GL_UNSIGNED_SHORT, GL_TRUE, sizeof(staticFmt), (void*)offsetof(staticFmt,texcoord));
    //glEnableVertexAttribArray(ATT_TEXCOORD);
    
    /*
    glVertexPointer(2,			// Data type count
                    GL_SHORT,		// Data type
                    sizeof(GLshort) * 2,	// Stride to the next vertex
                    0 );			// Vertex Buffer starting offset
 
    
    glTexCoordPointer(2,			// Data type count
                      GL_FLOAT,		// Data type
                      sizeof(GLfloat) * 2,	// Stride to the next vertex
                      0 );			// Vertex Buffer starting offset
    */
    
    glBindBuffer(GL_ARRAY_BUFFER, ff->m_InterleavedBuffers[i]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[i]);

    
    glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_STATIC_DRAW);
    
    glBufferSubData(GL_ARRAY_BUFFER, 0, (8 * sizeof(GLshort)), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, (8 * sizeof(GLshort)), (8 * sizeof(GLfloat)), texture);

    //glVertexPointer(2, GL_SHORT, 0, (char *)NULL + (0));
    //glTexCoordPointer(2, GL_FLOAT, 0, (char *)NULL + (8 * sizeof(GLshort)));

    /*
    glBindBuffer(GL_ARRAY_BUFFER, ff->m_VerticeBuffers[i]);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLshort), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ff->m_TextureBuffer[i]);
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat), texture, GL_STATIC_DRAW);
    */

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLshort), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glBindVertexArrayOES(0);

    free(vertices);
    free(texture);
    free(indices);
  }

  delete m_Sprites;

  //glFlush();
  //glFinish();

  return ff;
}
