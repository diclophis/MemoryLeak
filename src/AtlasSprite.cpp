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
  m_Position = new double[2];
  m_Velocity = new float[2];
  m_Scale = new float[2];
  m_TargetPosition = new double[2];
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
  m_OldFrame = 0;
  m_IsNinePatch = false;
  m_LastUsedBullet = 0;
}


void AtlasSprite::Render(StateFoo *sf, foofoo *batch_foo, float offsetX, float offsetY) {
  if (m_FooFoo->m_numFrames == 0) {
    return;
  }

  if (batch_foo == NULL) {
    LOGV("not supported\n");
    return;
  }

  float vx, vy, tx, ty;

  bool use_r = (m_Rotation != 0.0);
  float cos_r = fastSinf((M_PI / 2.0) - m_Rotation);
  float sin_r = fastSinf(m_Rotation);

  //TODO: document why 4 has to be 4
  int frame_times_four = m_Frame * 4;

  if (m_IsNinePatch) {
    float corner_size = 10.0;
    float percent_square = 3.0 / 8.0;
    float percent_narrow = 2.0 / 8.0;
    float row_height_square = percent_square * m_Scale[0] * 2.0;
    float row_height_narrow = percent_narrow * m_Scale[0] * 2.0;
    float row_height = row_height_square;
    float row_offset = 0;
    float texture_base_x = m_FooFoo->m_SpriteFoos[(m_Frame)].texture[0];
    float texture_base_y = m_FooFoo->m_SpriteFoos[(m_Frame)].texture[1];
    //float texture_offset_x = 0.0;
    float texture_offset = 0.0, texture_offset_y = 0.0;
    float half_scale_x = m_Scale[0];
    float half_scale_y = m_Scale[1];
    float percent_height = percent_square;
    int i;

    for (unsigned int j=0; j<3; j++) {
      // left
      i = 0;
      vx = -half_scale_x;
      vy = -half_scale_y + row_offset;
      tx = texture_base_x;
      ty = texture_base_y - texture_offset_y;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = (-half_scale_x + corner_size);
      vy = -half_scale_y + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * percent_square);
      ty = texture_base_y - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = (-half_scale_x + corner_size);
      vy = (-half_scale_y + row_height) + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * percent_square);
      ty = texture_base_y - (percent_height * m_FooFoo->m_texCoordHeight) - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = -half_scale_x;
      vy = (-half_scale_y + row_height) + row_offset;
      tx = texture_base_x;
      ty = texture_base_y - (percent_height * m_FooFoo->m_texCoordHeight) - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      batch_foo->m_NumBatched++;

      // middle
      i = 0;
      vx = (-half_scale_x + row_height_square);
      vy = -half_scale_y + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * percent_square);
      ty = texture_base_y - texture_offset_y;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;
      
      vx = (-half_scale_x + row_height_square) + row_height_narrow;
      vy = -half_scale_y + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * (percent_square + percent_narrow));
      ty = texture_base_y - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = (-half_scale_x + row_height_square) + row_height_narrow;
      vy = (-half_scale_y + row_height) + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * (percent_square + percent_narrow));
      ty = texture_base_y - (percent_height * m_FooFoo->m_texCoordHeight) - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = (-half_scale_x + row_height_square);
      vy = (-half_scale_y + row_height) + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * percent_square);
      ty = texture_base_y - (percent_height * m_FooFoo->m_texCoordHeight) - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      batch_foo->m_NumBatched++;

      // right
      i = 0;
      vx = (-half_scale_x + row_height_square) + row_height_narrow;
      vy = -half_scale_y + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * (percent_square + percent_narrow));
      ty = texture_base_y - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;
      
      vx = (-half_scale_x + row_height_square) + (row_height_narrow + row_height_square);
      vy = -half_scale_y + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth);
      ty = texture_base_y - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = (-half_scale_x + row_height_square) + (row_height_narrow + row_height_square);
      vy = (-half_scale_y + row_height) + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth);
      ty = texture_base_y - (percent_height * m_FooFoo->m_texCoordHeight) - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      vx = (-half_scale_x + row_height_square) + row_height_narrow;
      vy = (-half_scale_y + row_height) + row_offset;
      tx = texture_base_x + (m_FooFoo->m_texCoordWidth * (percent_square + percent_narrow));
      ty = texture_base_y - (percent_height * m_FooFoo->m_texCoordHeight) - texture_offset;
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
      i++;

      batch_foo->m_NumBatched++;

      if (row_height == row_height_square) {
        percent_height = percent_narrow;
        row_offset += row_height_square;
        row_height = row_height_narrow;
        texture_offset += (percent_square * m_FooFoo->m_texCoordHeight);
      } else {
        percent_height = percent_square;
        row_offset += row_height_narrow;
        row_height = row_height_square;
        texture_offset += (percent_narrow * m_FooFoo->m_texCoordHeight);
      }
      texture_offset_y = texture_offset;
    }
  } else {
    for (unsigned int i=0; i<4; i++) {
      vx = (m_FooFoo->m_SpriteFoos[(frame_times_four) + i].vertex[0] + offsetX) * m_Scale[0];
      vy = (m_FooFoo->m_SpriteFoos[(frame_times_four) + i].vertex[1] + offsetY) * m_Scale[1];
      tx = m_FooFoo->m_SpriteFoos[(frame_times_four) + i].texture[0];
      ty = m_FooFoo->m_SpriteFoos[(frame_times_four) + i].texture[1];
      BlitVertice(batch_foo, i, vx, vy, tx, ty, cos_r, sin_r, use_r);
    }
    batch_foo->m_NumBatched++;
  }
}


void AtlasSprite::BlitVertice(foofoo *batch_foo, int i, float vx, float vy, float tx, float ty, float cos_r, float sin_r, bool use_r) {
  //int x = ((cos(m_Rotation) * vx) - (sin(m_Rotation) * vy));
  //int y = ((sin(m_Rotation) * vx) + (cos(m_Rotation) * vy));

  int x;
  int y;

  if (use_r) {
    x = ((cos_r * vx) - (sin_r * vy));
    y = ((sin_r * vx) + (cos_r * vy));
  } else {
    x = vx;
    y = vy;
  }

  int batched_times_four = (batch_foo->m_NumBatched * 4) + i;
    
  batch_foo->m_SpriteFoos[(batched_times_four)].vertex[0] = (x + m_Position[0]);
  batch_foo->m_SpriteFoos[(batched_times_four)].vertex[1] = (y + m_Position[1]);
  batch_foo->m_SpriteFoos[(batched_times_four)].texture[0] = (tx);
  batch_foo->m_SpriteFoos[(batched_times_four)].texture[1] = (ty);
}


void AtlasSprite::RenderFoo(StateFoo *sf, foofoo *foo) {


  if (foo->m_Texture != sf->g_lastTexture) {
    glBindTexture(GL_TEXTURE_2D, foo->m_Texture);
    sf->g_lastTexture = foo->m_Texture;
  }
  
#ifdef HAS_VAO

  if (foo->m_VertexArrayObjects[0] == 0) {
    glGenVertexArraysOES(1, &foo->m_VertexArrayObjects[0]);
    sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[0];
    glBindVertexArrayOES(sf->g_lastVertexArrayObject);

/*
#ifdef USE_GLES2
    glActiveTexture(GL_TEXTURE0);
#else
    glEnable(GL_TEXTURE_2D);
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
*/

//
   
    if (foo->m_IndexBuffers[0] != sf->g_lastElementBuffer) {
      sf->g_lastElementBuffer = foo->m_IndexBuffers[0];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
    }
    
    if (foo->m_InterleavedBuffers[0] != sf->g_lastInterleavedBuffer) {
      sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
      glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
    }

#ifdef USE_GLES2
    
    glVertexAttribPointer(sf->g_PositionAttribute, 2, GL_SHORT, GL_FALSE, foo->m_Stride, (char *)NULL + (0));
    glVertexAttribPointer(sf->g_TextureAttribute, 2, GL_FLOAT, GL_FALSE, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));
    glEnableVertexAttribArray(sf->g_PositionAttribute);
    glEnableVertexAttribArray(sf->g_TextureAttribute);
    
#else
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(2, GL_SHORT, foo->m_Stride, (char *)NULL + (0));
    glTexCoordPointer(2, GL_FLOAT, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));
    
#endif
    
    
//

  } else {
    if (foo->m_VertexArrayObjects[0] != sf->g_lastVertexArrayObject) {
      sf->g_lastVertexArrayObject = foo->m_VertexArrayObjects[0];
      glBindVertexArrayOES(sf->g_lastVertexArrayObject);
    }

    /*
    //may
    if (foo->m_IndexBuffers[0] != sf->g_lastElementBuffer) {
      sf->g_lastElementBuffer = foo->m_IndexBuffers[0];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sf->g_lastElementBuffer);
    }
    */
    
    if (foo->m_InterleavedBuffers[0] != sf->g_lastInterleavedBuffer) {
      sf->g_lastInterleavedBuffer = foo->m_InterleavedBuffers[0];
      glBindBuffer(GL_ARRAY_BUFFER, sf->g_lastInterleavedBuffer);
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

#ifdef USE_GLES2

  glVertexAttribPointer(sf->g_PositionAttribute, 2, GL_SHORT, GL_FALSE, foo->m_Stride, (char *)NULL + (0));
  glVertexAttribPointer(sf->g_TextureAttribute, 2, GL_FLOAT, GL_FALSE, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));

  glEnableVertexAttribArray(sf->g_PositionAttribute);
  glEnableVertexAttribArray(sf->g_TextureAttribute);

#else
  
  glVertexPointer(2, GL_SHORT, foo->m_Stride, (char *)NULL + (0));
  glTexCoordPointer(2, GL_FLOAT, foo->m_Stride, (char *)NULL + (2 * sizeof(GLshort)));

#endif

#endif

  size_t interleaved_buffer_size = (foo->m_NumBatched * 4 * foo->m_Stride);
  glBufferData(GL_ARRAY_BUFFER, interleaved_buffer_size, NULL, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW might be faster...
  glBufferSubData(GL_ARRAY_BUFFER, 0, interleaved_buffer_size, foo->m_SpriteFoos);
  
  if (!sf->m_EnabledStates) {
    
#ifdef HAS_VAO

    //states are enabled via VAO
    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS);


#else

    glEnable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_ALWAYS);

#ifdef USE_GLES2

    glActiveTexture(GL_TEXTURE0);
    
    glEnableVertexAttribArray(sf->g_PositionAttribute);
    glEnableVertexAttribArray(sf->g_TextureAttribute);
    
#else

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#endif

#endif

    sf->m_EnabledStates = true;
  }
  
  glDrawElements(GL_TRIANGLES, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));

  if (false) {

#if 0
#ifdef USE_GLES2

    glActiveTexture(0);
    
    glDisableVertexAttribArray(sf->g_TextureAttribute);
#else
    glDisable(GL_TEXTURE_2D);
    glPointSize(10.0);
    glColor4f(0.0, 1.0, 0.0, 1.0);
#endif

    //glDrawElements(GL_POINTS, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));
    glDrawElements(GL_LINES, foo->m_NumBatched * 6, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));


    glColor4f(1.0, 1.0, 1.0, 0.0);

    glEnable(GL_TEXTURE_2D);
#endif

  }


}


void AtlasSprite::Simulate(float deltaTime) {
  m_Life += deltaTime;
  m_AnimationLife += deltaTime;
  //if (m_IsAlive) {
    if (m_Fps > 0) {
      if (m_AnimationLife > (1.0 / (float)m_Fps)) {
        //LOGV("m_Frame: %d\n", m_Frame);
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
      //m_Frame = fastAbs((((m_Life) / m_FooFoo->m_AnimationDuration) * m_FooFoo->m_numFrames));
      //if (m_Frame >= m_FooFoo->m_numFrames) {
      //  m_Frame = m_FooFoo->m_numFrames - 1;
      //}
    }
  //}
}


foofoo *AtlasSprite::GetBatchFoo(GLuint texture_index, int max_frame_count) {
  size_t size_of_sprite_foo = sizeof(SpriteFoo);
  GLushort *indices = (GLushort *) malloc(max_frame_count * 6 * sizeof(GLushort));
	foofoo *ff = new foofoo;

  ff->m_Texture = texture_index;
  ff->m_numFrames = max_frame_count;
  ff->m_numSpriteFoos = ff->m_numFrames * 4;
  ff->m_SpriteFoos = (SpriteFoo *)malloc(ff->m_numSpriteFoos * sizeof(SpriteFoo));

#ifdef HAS_VAO

  ff->m_numVertexArrayObjects = 1;
  ff->m_VertexArrayObjects = (GLuint*)calloc((ff->m_numVertexArrayObjects), sizeof(GLuint));

#endif

  ff->m_numInterleavedBuffers = 1;
  ff->m_InterleavedBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numInterleavedBuffers));

  ff->m_numIndexBuffers = 1;
  ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numIndexBuffers));

  ff->m_Stride = size_of_sprite_foo;

  glGenBuffers(ff->m_numInterleavedBuffers, ff->m_InterleavedBuffers);
  glBindBuffer(GL_ARRAY_BUFFER, ff->m_InterleavedBuffers[0]);
  glBufferData(GL_ARRAY_BUFFER, max_frame_count * 4 * ff->m_Stride, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  for (unsigned int i=0; i<max_frame_count; i++) {
    indices[(i * 6) + 0] = (i * 4) + 1;
    indices[(i * 6) + 1] = (i * 4) + 2;
    indices[(i * 6) + 2] = (i * 4) + 0;
    indices[(i * 6) + 3] = (i * 4) + 0;
    indices[(i * 6) + 4] = (i * 4) + 2;
    indices[(i * 6) + 5] = (i * 4) + 3;
  }

  glGenBuffers(ff->m_numIndexBuffers, ff->m_IndexBuffers);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[0]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_frame_count * 6 * sizeof(GLshort), indices, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  free(indices);

  return ff;
}


foofoo *AtlasSprite::GetFoo(GLuint texture_index, int sprites_per_row, int rows, int start, int end, float life) {
  GLshort *vertices;
  GLfloat *texture;
	float duration = life + 0.1;
  int length = end - start;
	int *m_Frames = new int[length];
	int total_count = sprites_per_row * rows;
	Sprite *m_Sprites = new Sprite[length];
	GLfloat tdx = (1.0 / (float)sprites_per_row);
	GLfloat tdy = (1.0 / (float)rows);

	float texture_x = 0;
	float texture_y = 0;
	int ii = 0;

  for (unsigned int i=0; i<length; i++) {
    m_Frames[i] = start + i;
  }

	for (unsigned int i=0; i<total_count; i++) {
		int b = (i % sprites_per_row);
		if (i == m_Frames[ii] && ii < length) {
			m_Sprites[ii].dx = 2.0;
			m_Sprites[ii].dy = 2.0;
			m_Sprites[ii].tx1 = texture_x;
			m_Sprites[ii].ty1 = texture_y;
			m_Sprites[ii].tx2 = (texture_x + (1.0 * tdx));
			m_Sprites[ii].ty2 = (texture_y + (1.0 * tdy));
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
  ff->m_texCoordWidth = tdx;
  ff->m_texCoordHeight = tdy;

  SpriteFoo *sprite_foos = (SpriteFoo *)malloc(ff->m_numSpriteFoos * sizeof(SpriteFoo));

  int sprite_foo_offset = 0;
  for (unsigned int i=0; i<length; i++) {
    GLshort w = m_Sprites[i].dx; 
    GLshort h = m_Sprites[i].dy; 
    vertices = (GLshort *) malloc(8 * sizeof(GLshort));
    vertices[0] =  (-w / 2.0);
    vertices[1] = (-h / 2.0);
    vertices[2] = (w / 2.0);
    vertices[3] = (-h / 2.0);
    vertices[4] = (w / 2.0);
    vertices[5] = (h / 2.0);
    vertices[6] = (-w / 2.0);
    vertices[7] = (h / 2.0);

    float fuzz = 0.0555533333;
    GLfloat tx = m_Sprites[i].tx1 + (fuzz * (m_Sprites[i].tx2 - m_Sprites[i].tx1));
    GLfloat ty = m_Sprites[i].ty1 + (fuzz * (m_Sprites[i].ty2 - m_Sprites[i].ty1));
    GLfloat tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1) * (1.0 - fuzz * 2.0);
    GLfloat th = (m_Sprites[i].ty2 - m_Sprites[i].ty1) * (1.0 - fuzz * 2.0);

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
      sprite_foos[sprite_foo_offset].texture[0] = (texture[(j * 2) + 0]); 
      sprite_foos[sprite_foo_offset].texture[1] = (texture[(j * 2) + 1]);
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
  bool done = false;
  float dx = m_Position[0] - m_TargetPosition[0];
  float dy = m_Position[1] - m_TargetPosition[1];

  float dir_x = 1.0;
  if (dx < 0.0) {
    dir_x = -1.0;
  }

  float dir_y = 1.0;
  if (dy < 0.0) {
    dir_y = -1.0;
  }

  float tx = 0.0;
  float ty = 0.0;

  if (dx != 0.0) {
    tx = -(0.1 * dt) * m_Velocity[0] * dir_x;
  }

  if (dy != 0.0) {
    ty = -(0.1 * dt) * m_Velocity[1] * dir_y;
  }

  m_Position[0] += tx;
  m_Position[1] += ty;

	if ((fastAbs(tx) > fastAbs(dx)) || (fastAbs(dx) < 1.0)) {
    m_Position[0] = m_TargetPosition[0];
  }
  if ((fastAbs(ty) > fastAbs(dy)) || (fastAbs(dy) < 1.0)) {
    m_Position[1] = m_TargetPosition[1];
  }

  if ((m_Position[0] == m_TargetPosition[0]) && (m_Position[1] == m_TargetPosition[1])) {
    done = true;
  }

  return done;
}

void AtlasSprite::SetPosition(float x,float y) {
  m_Position[0] = x;
  m_Position[1] = y;
}
