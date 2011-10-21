// Jon Bardin GPL


#include "MemoryLeak.h"

static GLuint g_lastTexture = -1;
static int g_lastFrame = -1;

void AtlasSprite::ReleaseBuffers() {
	g_lastTexture = -1;
}

void AtlasSprite::Scrub() {
  g_lastFrame = -1;
}

AtlasSprite::~AtlasSprite() {
  free(vertices);
  free(texture);
  delete m_Position;
  delete m_Velocity;
  delete m_Scale;
  delete m_Frames;
  delete m_Sprites;
}

AtlasSprite::AtlasSprite(GLuint t, int spr, int rows, int s, int e, float m, float vdx, float vdy) : m_Texture(t), m_SpritesPerRow(spr), m_Rows(rows), m_Start(s), m_End(e), m_MaxLife(m) {
  m_Fps = 0;
	m_Rotation = 0.0;
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
	//m_Count = m_SpritesPerRow * m_Rows;
	int m_TotalCount = m_SpritesPerRow * m_Rows;
	m_Count = m_AnimationLength;
	m_Sprites = new Sprite[m_Count];
	float tdx = 1.0 / (float)m_SpritesPerRow;
	float tdy = 1.0 / (float)m_Rows;
	float texture_x = 0.0;
	float texture_y = 0.0;
	int ii = 0;
	for (unsigned i=0; i<m_TotalCount; i++) {
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
			texture_x = 0.0;
			texture_y += tdy;
		}
	}
  vertices = (GLshort *) malloc(8 * sizeof(GLshort));
  texture = (GLfloat *)malloc(8 * sizeof(GLfloat));
}


void AtlasSprite::Render() {

	if (m_AnimationLength == 0) {
    LOGV("Fail, animation is at least 1 frame\n");
    return;
  }

  glEnable(GL_TEXTURE_2D);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);

	if (m_Texture != g_lastTexture) {

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, m_Texture);
		g_lastTexture = m_Texture;

	}

	glPushMatrix();
	{
		glTranslatef(m_Position[0], m_Position[1], 0.0);
    glRotatef(m_Rotation, 0.0, 0.0, 1.0);
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

    if (false) {
      glDisable(GL_TEXTURE_2D);
      glLineWidth(2.0);
      glColor4f(1.0, 0.0, 0.0, 1.0);
      glDrawElements(GL_LINES, 4, GL_UNSIGNED_BYTE, indices);
      glColor4f(1.0, 1.0, 1.0, 1.0);
      glEnable(GL_TEXTURE_2D);
    }
	}
	glPopMatrix();

  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisable(GL_TEXTURE_2D);
  
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
