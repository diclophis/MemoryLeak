// Jon Bardin GPL


#include "MemoryLeak.h"

static GLuint g_lastTexture = 0;
static GLuint g_lastVertexBuffer = 0;
static GLuint g_lastTexcoordBuffer = 0;
static GLuint g_lastElementBuffer = 0;
//static int g_lastFrame = -1;

void AtlasSprite::ReleaseBuffers() {
	g_lastTexture = -1;
  g_lastVertexBuffer = -1;
  g_lastTexcoordBuffer = -1;
  g_lastElementBuffer = -1;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void AtlasSprite::Scrub() {
  //g_lastFrame = -1;
}

AtlasSprite::~AtlasSprite() {
LOGV("AtlasSprite::dealloc\n");
  free(vertices);
  free(texture);
  free(indices);
  delete m_Position;
  delete m_Velocity;
  delete m_Scale;
  delete m_Frames;
  delete m_Sprites;
  delete m_FooFoo;
}

AtlasSprite::AtlasSprite(GLuint t, int spr, int rows, int s, int e, float m, float vdx, float vdy) : m_Texture(t), m_SpritesPerRow(spr), m_Rows(rows), m_Start(s), m_End(e), m_MaxLife(m) {
LOGV("AtlasSprite::alloc\n");
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
	m_AnimationSpeed = 1.0;
	m_AnimationDuration = m_MaxLife + 0.1;
	m_AnimationLength = m_Animation.length();
	if (m_AnimationLength > 0) {
	  m_Frames = new int[m_AnimationLength];
		for (unsigned int i=0; i<m_AnimationLength; i++) {
			m_Frames[i] = m_Animation[i % m_AnimationLength] - 50;
		}
	} else {
		m_AnimationLength = m_End - m_Start;
	  m_Frames = new int[m_AnimationLength];
		for (unsigned int i=0; i<m_AnimationLength; i++) {
			m_Frames[i] = m_Start + i;
		}
	}
	int m_TotalCount = m_SpritesPerRow * m_Rows;
	m_Count = m_AnimationLength;
	m_Sprites = new Sprite[m_Count];
	short tdx = SHRT_MAX / m_SpritesPerRow;
	short tdy = SHRT_MAX / m_Rows;
  LOGV("tdx, tdy %d %d\n", tdx, tdy);

	short texture_x = 0;
	short texture_y = 0;
	int ii = 0;
	for (unsigned int i=0; i<m_TotalCount; i++) {
		int b = (i % m_SpritesPerRow);
		if (i == m_Frames[ii] && ii < m_Count) {
			m_Sprites[ii].dx = vdx;
			m_Sprites[ii].dy = vdy;
			m_Sprites[ii].tx1 = texture_x;
			m_Sprites[ii].ty1 = texture_y;
			m_Sprites[ii].tx2 = texture_x + tdx;
			m_Sprites[ii].ty2 = texture_y + tdy;
			ii++;
		}
		texture_x += tdx;
		if (b == (m_SpritesPerRow - 1)) {
			texture_x = 0;
			texture_y += tdy;
		}
	}

  int i = 0;
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

  GLshort tx = m_Sprites[i].tx1;
  GLshort ty = m_Sprites[i].ty1;
  GLshort tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
  GLshort th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);

  texture = (GLshort *) malloc(8 * sizeof(GLshort));
  texture[0] = 0; //0;//tx;
  texture[1] = 1024; //0;//(ty + th);
  texture[2] = 1024;//tx + tw;
  texture[3] = 1024;//(ty + th);
  texture[4] = 1024;//tx + tw;
  texture[5] = 0;//ty;
  texture[6] = 0;//tx;
  texture[7] = 0;//ty;

  indices = (GLushort *) malloc(4 * sizeof(GLushort));

  indices[0] = 1;
  indices[1] = 2;
  indices[2] = 0;
  indices[3] = 3;

	foofoo *ff = new foofoo;
  ff->m_numFrames = 1;
	ff->m_numBuffers = ff->m_numFrames;
  ff->m_numTextureBuffers = ff->m_numFrames;
  ff->m_numNormalBuffers = 0;
	ff->m_VerticeBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_IndexBuffers = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_TextureBuffer = (GLuint*)malloc(sizeof(GLuint) * (ff->m_numBuffers));
	ff->m_AnimationStart = 0;
	ff->m_AnimationEnd = 1;

	glGenBuffers(ff->m_numBuffers, ff->m_VerticeBuffers);
	glGenBuffers(ff->m_numBuffers, ff->m_IndexBuffers);
	glGenBuffers(ff->m_numBuffers, ff->m_TextureBuffer);

  glBindBuffer(GL_ARRAY_BUFFER, ff->m_VerticeBuffers[i]);
  glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLshort), vertices, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, ff->m_TextureBuffer[i]);
  LOGV("%d %d %d %d %d %d %d %d\n", texture[0], texture[1], texture[2], texture[3], texture[4], texture[5], texture[6], texture[7]);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLshort), texture, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ff->m_IndexBuffers[i]);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4 * sizeof(GLshort), indices, GL_STATIC_DRAW);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glFlush();
  glFinish();

  m_FooFoo = ff;

}


void AtlasSprite::Render() {

	if (m_AnimationLength == 0) {
    LOGV("Fail, animation is at least 1 frame\n");
    return;
  }


	if (m_Texture != g_lastTexture) {


		glBindTexture(GL_TEXTURE_2D, m_Texture);
		g_lastTexture = m_Texture;

	}

	glPushMatrix();
	{
    //glMatrixMode(GL_TEXTURE);
    //glLoadIdentity();
    //glScalef(5.0, 5.0, 5.0);
    //glMatrixMode(GL_MODELVIEW);
    //glEnable(GL_NORMALIZE);

		glTranslatef(m_Position[0], m_Position[1], 0.0);
    
    if (m_LastRotation != m_Rotation) {
      glRotatef(m_Rotation, 0.0, 0.0, 1.0);
      m_LastRotation = m_Rotation;
    }
    /*
		int i = (m_Frame % m_AnimationLength);
    GLshort w = m_Sprites[i].dx;
    GLshort h = m_Sprites[i].dy;
    GLfloat tx = m_Sprites[i].tx1;
    GLfloat ty = m_Sprites[i].ty1;
    GLfloat tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
    GLfloat th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);
    if (i != g_lastFrame) {
      vertices[0] =  (-w / 2);
      vertices[1] = (-h / 2);
      vertices[2] = (w / 2);
      vertices[3] = (-h / 2);
      vertices[4] = (w / 2);
      vertices[5] = (h / 2);
      vertices[6] = (-w / 2);
      vertices[7] = (h / 2);
      glVertexPointer(2, GL_SHORT, 0, vertices);
      texture[0] = tx;
      texture[1] = (ty + th);
      texture[2] = tx + tw;
      texture[3] = (ty + th);
      texture[4] = tx + tw;
      texture[5] = ty;
      texture[6] = tx;
      texture[7] = ty;
      glTexCoordPointer(2, GL_FLOAT, 0, texture);
      g_lastFrame = i;
    }
    
		const GLushort indices [] = {1, 2, 0, 3};
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, indices);
    */





    m_Frame = 0;

		if (m_FooFoo->m_VerticeBuffers[m_Frame] != g_lastVertexBuffer) {
      g_lastVertexBuffer = m_FooFoo->m_VerticeBuffers[m_Frame];
      glBindBuffer(GL_ARRAY_BUFFER, g_lastVertexBuffer);
      glVertexPointer(2, GL_SHORT, 0, (GLvoid*)((char*)NULL));
    }

		if (m_FooFoo->m_TextureBuffer[m_Frame] != g_lastTexcoordBuffer) {
      g_lastTexcoordBuffer = m_FooFoo->m_TextureBuffer[m_Frame];
      glBindBuffer(GL_ARRAY_BUFFER, g_lastTexcoordBuffer);
      glTexCoordPointer(2, GL_SHORT, 0, (GLvoid*)((char*)NULL));
    }

		if (m_FooFoo->m_IndexBuffers[m_Frame] != g_lastElementBuffer) {
      g_lastElementBuffer = m_FooFoo->m_IndexBuffers[m_Frame];
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_lastElementBuffer);
    }

    //glMatrixMode(GL_TEXTURE);
    //glLoadIdentity();
    //glScalef(1.0/512.0, 1.0/512.0, 1.0);
    //glMatrixMode(GL_MODELVIEW);
    //glLoadIdentity();

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, (GLvoid*)((char*)NULL));


    if (false) {
      glDisable(GL_TEXTURE_2D);
      //glLineWidth(2.0);
      glPointSize(10.0);
      glColor4f(0.0, 1.0, 0.0, 1.0);
      //glDrawElements(GL_LINES, 4, GL_UNSIGNED_BYTE, indices);
      //glDrawElements(GL_POINTS, 2 * 2, GL_UNSIGNED_BYTE, (GLvoid*)((char*)NULL));
      glColor4f(1.0, 1.0, 1.0, 1.0);
      glEnable(GL_TEXTURE_2D);
    }

		//glTranslatef(-m_Position[0], -m_Position[1], 0.0);
  }
	glPopMatrix();
  
}


void AtlasSprite::SetScale(float x, float y) {
  int i = (m_Frame % m_AnimationLength);
  m_Sprites[i].dx = (100.0 * x);
  m_Sprites[i].dy = (100.0 * y);
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
        m_Frame = m_AnimationLength - 1;
      }
      
      if (m_Frame >= m_AnimationLength) {
        m_Frame = 0;
      }
    } else {
      m_Frame = fastAbs((((m_Life) / m_AnimationDuration) * m_AnimationLength));
    }
  }
}
