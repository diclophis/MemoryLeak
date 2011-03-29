
#include "MemoryLeak.h"

#include "AtlasSprite.h"

#define GL_PIXEL_UNPACK_BUFFER_ARB 0x88EC

static GLuint g_lastTexture = 0;

void AtlasSprite::ReleaseBuffers() {
	g_lastTexture = 0;
}

AtlasSprite::AtlasSprite(GLuint t, int spr, int rows, int s, int e, float m, float vdx, float vdy) : m_Texture(t), m_SpritesPerRow(spr), m_Rows(rows), m_Start(s), m_End(e), m_MaxLife(m) {
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
	m_Frames = new int[1024];
	if (m_AnimationLength > 0) {
		for (unsigned int i=0; i<m_AnimationLength; i++) {
			m_Frames[i] = m_Animation[i % m_AnimationLength] - 50;
		}
	} else {
		m_AnimationLength = m_End - m_Start;
		for (unsigned int i=0; i<m_AnimationLength; i++) {
			m_Frames[i] = m_Start + i;
		}
	}
	m_Count = m_SpritesPerRow * m_Rows;
	m_Sprites = new Sprite[m_Count];
	float tdx = 1.0 / (float)m_SpritesPerRow;
	float tdy = 1.0 / (float)m_Rows;
	//float vdx = 50.0;
	//float vdy = 50.0;
	float texture_x = 0.0;
	float texture_y = 0.0;
	for (unsigned i=0; i<m_Count; i++) {
		int b = (i % m_SpritesPerRow);
		m_Sprites[i].dx = vdx;
		m_Sprites[i].dy = vdy;
		m_Sprites[i].tx1 = texture_x;
		m_Sprites[i].ty1 = texture_y;
		m_Sprites[i].tx2 = texture_x + tdx;
		m_Sprites[i].ty2 = texture_y + tdy;
		texture_x += tdx;
		if (b == (m_SpritesPerRow - 1)) {
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
		//glRotatef(randf() * 2.0, 0.0, 0.0, 1.0);
		//m_Rotation *= (randf() > 0.99) ? -1.0 : 1.0;
		//glRotatef((m_Life * (25.0 * m_Rotation)), 0.0, 0.0, 1.0);
		glRotatef(m_Rotation, 0.0, 0.0, 1.0);
		glScalef(m_Scale[0], m_Scale[1], 1.0);
	
		int i = m_Frames[m_Frame % m_AnimationLength];

		float w = m_Sprites[i].dx;
		float h = m_Sprites[i].dy;
		//upper left, lower right
		float tx = m_Sprites[i].tx1;
		float ty = m_Sprites[i].ty1;
		float tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
		float th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);
		float vertices[8] = {
			(-w / 2.0), (-h / 2.0),
			(w / 2.0), (-h / 2.0),
			(w / 2.0), (h / 2.0),
			(-w / 2.0), (h / 2.0)
		};
		float texture[8] = {
			tx, (ty + th),
			tx + tw, (ty + th),
			tx + tw, ty,
			tx, ty
		};
		const GLubyte indices [] = {1, 2, 0, 3};
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texture);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);
		//this works with tartan thing
		//glLineWidth(10.0);
		//glDrawElements(GL_LINES, 4, GL_UNSIGNED_BYTE, indices);

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
	
	float m_Fps = 3.0;
	//LOGV("life: %f\n", m_Life);
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
	
	//m_Frame = fastAbs((((m_Life) / m_AnimationDuration) * m_AnimationLength));
};
