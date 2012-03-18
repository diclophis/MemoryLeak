// Jon Bardin GPL


#include "MemoryLeak.h"


void AtlasSprite::ReleaseBuffers() {
#ifdef HAS_VAO
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
  delete m_TargetPosition;
}


AtlasSprite::AtlasSprite(foofoo *ff) : m_FooFoo(ff) {
  m_Fps = 0;
	m_Rotation = m_LastRotation = 0.0;
	m_Position = new float[2];
	m_Velocity = new float[2];
	m_Scale = new float[2];
	m_TargetPosition = new float[2];

  m_TargetPosition[0] = 0.0;
  m_TargetPosition[1] = 0.0;
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


void AtlasSprite::Render(StateFoo *sf, foofoo *batch_foo) {
	if (m_FooFoo->m_numFrames == 0) {
    //LOGV("Fail, animation is at least 1 frame\n");
    return;
  }
  if (batch_foo == NULL) {
    LOGV("not supported\n");
    return;
  } else {
    for (unsigned int i=0; i<4; i++) {
      int x = ((cos(m_Rotation) * m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].vertex[0]) - (sin(m_Rotation) * m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].vertex[1])) * m_Scale[0];
      int y = ((sin(m_Rotation) * m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].vertex[0]) + (cos(m_Rotation) * m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].vertex[1])) * m_Scale[1];

      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[0] = (x + m_Position[0]);
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].vertex[1] = (y + m_Position[1]);
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].texture[0] = m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].texture[0];
      batch_foo->m_SpriteFoos[(batch_foo->m_NumBatched * 4) + i].texture[1] = m_FooFoo->m_SpriteFoos[(m_Frame * 4) + i].texture[1];
    }
    batch_foo->m_NumBatched++;
  }
}


void AtlasSprite::RenderFoo(StateFoo *sf, foofoo *foo) {

  glEnable(GL_BLEND);
  glEnable(GL_TEXTURE_2D);


  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	
  if (foo->m_Texture != sf->g_lastTexture) {
		glBindTexture(GL_TEXTURE_2D, foo->m_Texture);
		sf->g_lastTexture = foo->m_Texture;
	}

#ifdef HAS_VAO
  if (foo->m_VertexArrayObjects[0] == 0) {
    glGenVertexArraysOES(1, &foo->m_VertexArrayObjects[0]);
    sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[0];
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);
    //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //glEnableClientState(GL_VERTEX_ARRAY);
    sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
    glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
    sf->g_lastElementBuffer = foo->m_IndexBuffers[0];
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
    glVertexPointer(2, GL_SHORT, foo->m_Stride, (char *)NULL + (0));
    glTexCoordPointer(2, GL_FLOAT, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));
  } else {
    if (foo->m_VertexArrayObjects[0] != sf->g_lastVertexArrayObject) {
      sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
      glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
      sf->g_lastElementBuffer = foo->m_IndexBuffers[0];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
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
  
  glVertexPointer(2, GL_SHORT, foo->m_Stride, (char *)NULL + (0));
  glTexCoordPointer(2, GL_FLOAT, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));
#endif

  size_t interleaved_buffer_size = (foo->m_NumBatched * 4 * foo->m_Stride);
  glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_DYNAMIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, foo->m_SpriteFoos);
  
  glDrawElements(GL_TRIANGLES, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

  if (false) {
    glDisable(GL_TEXTURE_2D);
    glPointSize(2.0);
    glColor4f(0.0, 1.0, 0.0, 1.0);
    glDrawElements(GL_LINES, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
  }

  foo->m_NumBatched = 0;

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}


void AtlasSprite::Simulate(float deltaTime) {
	//float dx = m_Velocity[0] * deltaTime;
	//float dy = m_Velocity[1] * deltaTime;
	//m_Position[0] += dx;
	//m_Position[1] += dy;
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
      
      if (m_Frame > m_FooFoo->m_numFrames - 1) {
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
  ff->m_numSpriteFoos = ff->m_numFrames * 4;

  ff->m_SpriteFoos = (SpriteFoo *)malloc(ff->m_numSpriteFoos * sizeof(SpriteFoo));

  ff->m_numVertexArrayObjects = 1;
	ff->m_VertexArrayObjects = (GLuint*)calloc((ff->m_numVertexArrayObjects), sizeof(GLuint));

  ff->m_numInterleavedBuffers = 1;
	ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numInterleavedBuffers));

	glGenBuffers(ff->m_numInterleavedBuffers, ff->m_InterleavedBuffers);

  size_t size_of_sprite_foo = sizeof(SpriteFoo);
  ff->m_Stride = size_of_sprite_foo;

  ff->m_numIndexBuffers = 1;
  ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numIndexBuffers));
  glGenBuffers(ff->m_numIndexBuffers, ff->m_IndexBuffers);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[0]);
  GLushort *indices;
  indices = (GLushort *) malloc(max_frame_count * 6 * sizeof(GLushort));
  for (unsigned int i=0; i<max_frame_count; i++) {
    indices[(i * 6) + 0] = (i * 4) + 1;
    indices[(i * 6) + 1] = (i * 4) + 2;
    indices[(i * 6) + 2] = (i * 4) + 0;
    indices[(i * 6) + 3] = (i * 4) + 0;
    indices[(i * 6) + 4] = (i * 4) + 2;
    indices[(i * 6) + 5] = (i * 4) + 3;
  }

  glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_frame_count * 6 * sizeof(GLshort), indices, GL_DYNAMIC_DRAW);
  free(indices);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  
  return ff;
}


foofoo *AtlasSprite::GetFoo(GLuint texture_index, int sprites_per_row, int rows, int start, int end, float life) {
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
			m_Sprites[ii].dx = 2.0;
			m_Sprites[ii].dy = 2.0;
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
  ff->m_numSpriteFoos = length * 4;

  int sprite_foo_offset = 0;
  SpriteFoo *sprite_foos = (SpriteFoo *)malloc(ff->m_numSpriteFoos * sizeof(SpriteFoo));

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
  ff->m_Stride = size_of_sprite_foo;
  ff->m_SpriteFoos = sprite_foos;

  return ff;
}


bool AtlasSprite::MoveToTargetPosition(float dt) {
	float dx = m_Position[0] - m_TargetPosition[0];
	float dy = m_Position[1] - m_TargetPosition[1];
	float tx = 0.0;
	float ty = 0.0;
	bool done = false;
	if ((fabs(dx) > 2.0) || (fabs(dy) > 2.0)) {
    if (dx != 0.0) {
      tx = -(dt * m_Velocity[0] * (dx / fastAbs(dx)));
    }
    if (dy != 0.0) {
      ty = -(dt * m_Velocity[1] * (dy / fastAbs(dy)));
    }
		done = false;
	} else {
		tx = -dx;
		ty = -dy;
		m_Velocity[0] = 0;
		m_Velocity[1] = 0;
		done = true;
	}
	m_Position[0] += tx;
	m_Position[1] += ty;
  return done;
}
