
#include "MemoryLeak.h"

#include "AtlasSprite.h"

static GLuint g_lastTexture = 0;

AtlasSprite::AtlasSprite(GLuint t, int spr, int rows, const std::string &str, int s, int e, float m) : m_Texture(t), m_SpritesPerRow(spr), m_Rows(rows), m_Animation(str), m_Start(s), m_End(e), m_MaxLife(m) {
	m_Position = new float[2];
	m_Velocity = new float[2];
	
	m_Position[0] = 0.0;
	m_Position[1] = 0.0;
	
	m_Velocity[0] = 0.0;
	m_Velocity[1] = 0.0;
	
	m_Life = 0.0;
	m_IsAlive = true;
	
	m_Frame = 0;
	m_AnimationSpeed = 1.0;
	m_AnimationDuration = m_MaxLife + 0.5;
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
	float vdx = 50.0;
	float vdy = 50.0;
	float texture_x = 0.0;
	float texture_y = 0.0;
	unsigned int i;
	for (i=0; i<m_Count; i++) {
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
	//glEnable(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, m_Texture);

	if (m_Texture != g_lastTexture) {
		glBindTexture(GL_TEXTURE_2D, m_Texture);
		g_lastTexture = m_Texture;
	}
	
	//glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	float ax = m_Position[0];
	float ay = m_Position[1];

	glPushMatrix();
	{
		glTranslatef(ax, ay, 0.0);
		//glRotatef(randf() * 100.0, 0.0, 0.0, 1.0);
		//glScalef((m_Life * 1.1) + 0.5, (m_Life * 1.1) + 0.5, (m_Life * 1.1) + 0.5);
	
	//int sprites_to_draw = (int)strlen(m_Animation);
	//int sprites_to_draw = 3;
	//for (unsigned int j = 0; j < sprites_to_draw; j++) {
		//int b = (i % m_SpritesPerRow);
		//ax = m_Sprites[i].dx * b;
		//int i = m_Animation[j] - 48;
		//int i = j;
		//int i = m_Frame;
		//LOGV("%d %d\n", m_Frame, m_AnimationLength);
		int i = m_Frames[m_Frame % m_AnimationLength];
		//int i = m_Frame % m_Count;
		//LOGV("=%d\n", i);

		float w = m_Sprites[i].dx;
		float h = m_Sprites[i].dy;
		//upper left, lower right
		float tx = m_Sprites[i].tx1;
		float ty = m_Sprites[i].ty1;
		float tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
		float th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);
		/*
		float vertices[8] = {
			(-w / 2.0) + ax, (-h / 2.0) + ay,
			(w / 2.0) + ax, (-h / 2.0) + ay,
			(w / 2.0) + ax, (h / 2.0) + ay,
			(-w / 2.0) + ax, (h / 2.0) + ay
		};
		*/
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
		//if (b == (m_SpritesPerRow - 1)) {
		//	ay += m_Sprites[i].dy;
		//}
		
		ax += m_Sprites[i].dx;
		ay += w;

	//}
	}
	glPopMatrix();
	
	//glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	//glBindTexture(GL_TEXTURE_2D, 0);
	//glDisable(GL_TEXTURE_2D);
}

void AtlasSprite::Simulate(float deltaTime) {
	float dx = m_Velocity[0] * deltaTime;
	float dy = m_Velocity[1] * deltaTime;
	m_Position[0] += dx;
	m_Position[1] += dy;
	
	m_Life += deltaTime;
	m_Frame = (((m_Life) / m_AnimationDuration) * m_AnimationLength);
};

/*
void AtlasSprite::SetAnimation(std::string a) {
	//snprintf(m_Animation, sizeof(m_Animation), "%s", a);
	//m_Animation = &a;
	//m_AnimationLength = a.length();
	//memcpy(m_Animation, a, m_AnimationLength);
	//m_Animation = a;
	
	//const char* -> char*
	//name = new char [strlen(str)+1];
	//strcpy(name, str);
	//name = new char [strlen(p.name)+1];
	
}
*/

