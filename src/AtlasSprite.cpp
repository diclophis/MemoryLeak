
#include "MemoryLeak.h"

#include "AtlasSprite.h"

#define GL_PIXEL_UNPACK_BUFFER_ARB 0x88EC

static GLuint g_lastTexture = 0;

void AtlasSprite::ReleaseBuffers() {
	g_lastTexture = 0;
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
	
	float ax = m_Position[0];
	float ay = m_Position[1];

	glPushMatrix();
	{
		glTranslatef(ax, ay, 0.0);
		glRotatef(m_Rotation, 0.0, 0.0, 1.0);
		glScalef(m_Scale[0], m_Scale[1], 1.0);
		//int i = m_Frames[m_Frame % m_AnimationLength];
		int i = (m_Frame % m_AnimationLength);
		GLfloat w = m_Sprites[i].dx;
		GLfloat h = m_Sprites[i].dy;
		GLfloat tx = m_Sprites[i].tx1;
		GLfloat ty = m_Sprites[i].ty1;
		GLfloat tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
		GLfloat th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);
		GLfloat vertices[8] = {
			(-w / 2.0), (-h / 2.0),
			(w / 2.0), (-h / 2.0),
			(w / 2.0), (h / 2.0),
			(-w / 2.0), (h / 2.0)
		};
		GLfloat texture[8] = {
			tx, (ty + th),
			tx + tw, (ty + th),
			tx + tw, ty,
			tx, ty
		};

		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texture);
    
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		const GLubyte indices [] = {1, 2, 0, 3};
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);
/*
    if (true) {
      glDisable(GL_TEXTURE_2D);
      glLineWidth(2.0);
      glColor4f(1.0, 1.0, 1.0, 1.0);
      glDrawElements(GL_LINES, 4, GL_UNSIGNED_BYTE, indices);
      glEnable(GL_TEXTURE_2D);
    }
*/

		ax += m_Sprites[i].dx;
		ay += w;
	}
	glPopMatrix();
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
};
