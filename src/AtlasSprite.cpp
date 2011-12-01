// Jon Bardin GPL


#include "MemoryLeak.h"


static GLuint g_lastTexture = -1;
static GLuint g_lastElementBuffer = -1;
static GLuint g_lastInterleavedBuffer = -1;
static GLuint g_lastVertexArrayObject = -1;

void AtlasSprite::ReleaseBuffers() {
  g_lastInterleavedBuffer = -1;
  g_lastElementBuffer = -1;
#ifdef HAS_VAO
  g_lastVertexArrayObject = -1;
  glBindVertexArrayOES(0);
#else
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
#endif
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


void AtlasSprite::Render(foofoo *batch_foo) {
	if (m_FooFoo->m_numFrames == 0) {
    LOGV("Fail, animation is at least 1 frame\n");
    return;
  }


  if (batch_foo == NULL) {
    //glPushMatrix();
    {

      glTranslatef(m_Position[0], m_Position[1], 0.0);
      glRotatef(m_Rotation, 0.0, 0.0, 1.0);

  #ifdef HAS_VAO
      if (m_FooFoo->m_VertexArrayObjects[m_Frame] == 0) {
        glGenVertexArraysOES(1, &m_FooFoo->m_VertexArrayObjects[m_Frame]);
        g_lastVertexArrayObject = m_FooFoo->m_VertexArrayObjects[m_Frame];
        glBindVertexArrayOES(g_lastVertexArrayObject);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glEnableClientState(GL_VERTEX_ARRAY);
        g_lastInterleavedBuffer = m_FooFoo->m_InterleavedBuffers[0];
        glBindBuffer(GL_ARRAY_BUFFER, g_lastInterleavedBuffer);
        g_lastElementBuffer = m_FooFoo->m_IndexBuffers[0];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
        glVertexPointer(2, GL_SHORT, m_FooFoo->m_Stride, (char *)NULL + (0) + (m_Frame * 4 * m_FooFoo->m_Stride));
        glTexCoordPointer(2, GL_FLOAT, m_FooFoo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)) + (m_Frame * 4 * m_FooFoo->m_Stride));
      } else {
        if (m_FooFoo->m_VertexArrayObjects[m_Frame] != g_lastVertexArrayObject) {
          g_lastVertexArrayObject = m_FooFoo->m_VertexArrayObjects[m_Frame];
          glBindVertexArrayOES(g_lastVertexArrayObject);
        }
      }
  #else
      if (m_FooFoo->m_IndexBuffers[0] != g_lastElementBuffer) {
        g_lastElementBuffer = m_FooFoo->m_IndexBuffers[0];
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
      }
      if (m_FooFoo->m_InterleavedBuffers[0] != g_lastInterleavedBuffer) {
        g_lastInterleavedBuffer = m_FooFoo->m_InterleavedBuffers[0];
        glBindBuffer(GL_ARRAY_BUFFER, g_lastInterleavedBuffer);
      }
      glVertexPointer(2, GL_SHORT, m_FooFoo->m_Stride, (char *)NULL + (0) + (m_Frame * 4 * m_FooFoo->m_Stride));
      glTexCoordPointer(2, GL_FLOAT, m_FooFoo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)) + (m_Frame * 4 * m_FooFoo->m_Stride));
  #endif

      if (m_FooFoo->m_Texture != g_lastTexture) {
        glBindTexture(GL_TEXTURE_2D, m_FooFoo->m_Texture);
        g_lastTexture = m_FooFoo->m_Texture;
      }
      
      glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
      
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
    //glPopMatrix();
  } else {
    //SpriteFoo *sprite_foo_source = (SpriteFoo *)(m_FooFoo->m_SpriteFoos + (0 * 4 * m_FooFoo->m_Stride));
    //SpriteFoo *sprite_foo_dest = (SpriteFoo *)(batch_foo->m_SpriteFoos + (batch_foo->m_NumBatched * 4 * m_FooFoo->m_Stride));
    //(SpriteFoo *)malloc(length * 4 * sizeof(SpriteFoo));
    //glVertexPointer(2, GL_SHORT, m_FooFoo->m_Stride, (char *)NULL + (0) + (m_Frame * 4 * m_FooFoo->m_Stride));
    //  glTexCoordPointer(2, GL_FLOAT, m_FooFoo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)) + (m_Frame * 4 * m_FooFoo->m_Stride));
    //m_Position[0], m_Position[1]

    //LOGV("set: %d\n", batch_foo->m_NumBatched);
    for (unsigned int i=0; i<4; i++) {
      //LOGV("setf: %d\n", (batch_foo->m_NumBatched * 4) + i);
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[0] = m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].vertex[0] + m_Position[0];
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[1] = m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].vertex[1] + m_Position[1];
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].texture[0] = m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].texture[0];
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].texture[1] = m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].texture[1];
      //if (((batch_foo->m_NumBatched * 4) + i) == 8) {
      //  LOGV("%d %d %d\n", m_Frame, batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[0], batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[1]);
      //} else {
      //  //LOGV("%d\n", ((batch_foo->m_NumBatched * 4) + i));
      //}

    /*
    sprite_foo_dest[0].vertex[0] = sprite_foo_source[0].vertex[0] + m_Position[0];
    sprite_foo_dest[0].vertex[1] = sprite_foo_source[0].vertex[1] + m_Position[1];
    sprite_foo_dest[1].vertex[0] = sprite_foo_source[1].vertex[0] + m_Position[0];
    sprite_foo_dest[1].vertex[1] = sprite_foo_source[1].vertex[1] + m_Position[1];
    sprite_foo_dest[2].vertex[0] = sprite_foo_source[2].vertex[0] + m_Position[0];
    sprite_foo_dest[2].vertex[1] = sprite_foo_source[2].vertex[1] + m_Position[1];
    sprite_foo_dest[3].vertex[0] = sprite_foo_source[3].vertex[0] + m_Position[0];
    sprite_foo_dest[3].vertex[1] = sprite_foo_source[3].vertex[1] + m_Position[1];

    sprite_foo_dest[0].texture[0] = sprite_foo_source[0].texture[0];
    sprite_foo_dest[0].texture[1] = sprite_foo_source[0].texture[1];
    sprite_foo_dest[1].texture[0] = sprite_foo_source[1].texture[0];
    sprite_foo_dest[1].texture[1] = sprite_foo_source[1].texture[1];
    sprite_foo_dest[2].texture[0] = sprite_foo_source[2].texture[0];
    sprite_foo_dest[2].texture[1] = sprite_foo_source[2].texture[1];
    sprite_foo_dest[3].texture[0] = sprite_foo_source[3].texture[0];
    sprite_foo_dest[3].texture[1] = sprite_foo_source[3].texture[1];
    */
      //LOGV("%d %d %d\n", m_Frame, batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[0], batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[1]);
    }

    //LOGV("%d %d %d\n", m_Frame, batch_foo->m_SpriteFoos[1].vertex[0], batch_foo->m_SpriteFoos[1].vertex[1]);
    //LOGV("%d %d %d\n", m_Frame, batch_foo->m_SpriteFoos[2].vertex[0], batch_foo->m_SpriteFoos[2].vertex[1]);
    //LOGV("%d %d %d\n", m_Frame, batch_foo->m_SpriteFoos[3].vertex[0], batch_foo->m_SpriteFoos[3].vertex[1]);
    
    batch_foo->m_NumBatched++;
  }
}


void AtlasSprite::RenderFoo(foofoo *foo) {

  //LOGV("draw: %d\n", foo->m_NumBatched);
	//if (m_FooFoo->m_Texture != g_lastTexture) {
		glBindTexture(GL_TEXTURE_2D, foo->m_Texture);
		g_lastTexture = foo->m_Texture;
	//}
  //if (foo->m_IndexBuffers[0] != g_lastElementBuffer) {
    g_lastElementBuffer = foo->m_IndexBuffers[0];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
  //}

  //if (m_FooFoo->m_InterleavedBuffers[0] != g_lastInterleavedBuffer) {
    g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
    glBindBuffer(GL_ARRAY_BUFFER, g_lastInterleavedBuffer);
    size_t interleaved_buffer_size = (foo->m_NumBatched * 4 * foo->m_Stride);
    glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, foo->m_SpriteFoos);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);
  //}
  glVertexPointer(2, GL_SHORT, foo->m_Stride, (char *)NULL + (0));
  glTexCoordPointer(2, GL_FLOAT, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));
  
  glDrawElements(GL_TRIANGLES, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
  //glDrawArrays(GL_TRIANGLE_STRIP, 0, (foo->m_NumBatched) * 3);

  if (false) {
    glDisable(GL_TEXTURE_2D);
    glPointSize(2.0);
    glColor4f(0.0, 1.0, 0.0, 1.0);
    glDrawElements(GL_POINTS, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
  }
  foo->m_NumBatched = 0;
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
      
      if (m_Frame > m_FooFoo->m_numFrames) {
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


foofoo *AtlasSprite::GetBatchFoo(GLuint texture_index, int max_frame_count) {
	foofoo *ff = new foofoo;
  ff->m_Texture = texture_index;
  ff->m_numFrames = max_frame_count;
  ff->m_SpriteFoos = (SpriteFoo *)malloc(ff->m_numFrames * 4 * sizeof(SpriteFoo));

  ff->m_numVertexArrayObjects = 1;
	ff->m_VertexArrayObjects = (GLuint*)calloc((ff->m_numVertexArrayObjects), sizeof(GLuint));

  ff->m_numInterleavedBuffers = 1;
	ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numInterleavedBuffers));

	glGenBuffers(ff->m_numInterleavedBuffers, ff->m_InterleavedBuffers);

  size_t size_of_sprite_foo = sizeof(SpriteFoo);
  //size_t interleaved_buffer_size = (ff->m_numFrames * 4 * size_of_sprite_foo);
  ff->m_Stride = size_of_sprite_foo;
  //glBindBuffer(GL_ARRAY_BUFFER, ff->m_InterleavedBuffers[0]);
  //glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_STATIC_DRAW);
  //glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, sprite_foos);
  //glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenBuffers(1, ff->m_IndexBuffers);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[0]);
  GLushort *indices;
  indices = (GLushort *) malloc(max_frame_count * 6 * sizeof(GLushort));
  for (unsigned int i=0; i<max_frame_count; i++) {
    indices[(i * 6) + 0] = (i * 4) + 1; //5
    indices[(i * 6) + 1] = (i * 4) + 2; //6
    indices[(i * 6) + 2] = (i * 4) + 0; //4

    indices[(i * 6) + 3] = (i * 4) + 0; //4
    indices[(i * 6) + 4] = (i * 4) + 2; //6
    indices[(i * 6) + 5] = (i * 4) + 3; //7
  }

  //for (unsigned int i=0; i<(max_frame_count * 6); i++) {
  //  LOGV("120-023: %d = %d\n", i, indices[i]);
  //}

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_frame_count * 6 * sizeof(GLshort), indices, GL_STATIC_DRAW);
  free(indices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  //g_AtlasSpriteIndexBuffer = m_IndexBuffers[0];
  
  return ff;
}


foofoo *AtlasSprite::GetFoo(GLuint texture_index, int sprites_per_row, int rows, int start, int end, float life, float width, float height) {
  GLshort *vertices;
  GLfloat *texture;
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
	ff->m_AnimationStart = start;
	ff->m_AnimationEnd = end;
  ff->m_AnimationDuration = duration;

  ff->m_numVertexArrayObjects = length;
	ff->m_VertexArrayObjects = (GLuint*)calloc((ff->m_numVertexArrayObjects), sizeof(GLuint));

  ff->m_numInterleavedBuffers = 1;

	ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numInterleavedBuffers));

  int sprite_foo_offset = 0;
  SpriteFoo *sprite_foos = (SpriteFoo *)malloc(length * 4 * sizeof(SpriteFoo));

	glGenBuffers(ff->m_numInterleavedBuffers, ff->m_InterleavedBuffers);

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

    for (unsigned int j=0; j<4; j++) {
      sprite_foos[sprite_foo_offset].vertex[0] = vertices[(j * 2) + 0]; 
      sprite_foos[sprite_foo_offset].vertex[1] = vertices[(j * 2) + 1];
      sprite_foos[sprite_foo_offset].texture[0] = texture[(j * 2) + 0]; 
      sprite_foos[sprite_foo_offset].texture[1] = texture[(j * 2) + 1];
      sprite_foo_offset++;
    }

    free(vertices);
    free(texture);

  }

  delete m_Sprites;

  size_t size_of_sprite_foo = sizeof(SpriteFoo);
  size_t interleaved_buffer_size = (ff->m_numFrames * 4 * size_of_sprite_foo);
  ff->m_Stride = size_of_sprite_foo; 
  ff->m_SpriteFoos = sprite_foos;
  glBindBuffer(GL_ARRAY_BUFFER, ff->m_InterleavedBuffers[0]);
  glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, sprite_foos);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  //free(sprite_foos);

  return ff;
}
