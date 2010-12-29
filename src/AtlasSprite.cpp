
#include "MemoryLeak.h"

#include "AtlasSprite.h"

AtlasSprite::AtlasSprite(GLuint t, int spr, int rows) : m_Texture(t), m_SpritesPerRow(spr), m_Rows(rows) {
		
	m_Animation = (char *)malloc(3 * sizeof(char));
	m_Position = (float *)malloc(2 * sizeof(float));
	m_Velocity = (float *)malloc(2 * sizeof(float));

	m_Position[0] = 0.0;
	m_Position[1] = 0.0;
	
	m_Velocity[0] = 0.0;
	m_Velocity[1] = 0.0;
	
	m_Life = 0.0;
	m_IsAlive = true;
	
	m_Frame = 0;
	m_AnimationSpeed = 20.0;
	
	m_Count = m_SpritesPerRow * m_Rows;
	
	m_Sprites = new Sprite[m_Count];
	
	float tdx = 1.0 / (float)m_SpritesPerRow;
	float tdy = 1.0 / (float)m_Rows;
	float vdx = 50.0;
	float vdy = 50.0;
	float texture_x = 0.0;
	float texture_y = 0.0;
	for (unsigned int i=0; i<m_Count; i++) {
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
	
	SetAnimation("01234567", 0);
}

void AtlasSprite::Render() {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_Texture);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	float ax = m_Position[0];
	float ay = m_Position[1];
	
	
	//int sprites_to_draw = (int)strlen(m_Animation);
	//int sprites_to_draw = 3;
	//for (unsigned int j = 0; j < sprites_to_draw; j++) {
		//int b = (i % m_SpritesPerRow);
		//ax = m_Sprites[i].dx * b;
		//int i = m_Animation[j] - 48;
		//int i = j;
		//int i = m_Frame;
		int i = m_Animation[m_Frame % m_AnimationLength] - 48;
		float w = m_Sprites[i].dx;
		float h = m_Sprites[i].dy;
		//upper left, lower right
		float tx = m_Sprites[i].tx1;
		float ty = m_Sprites[i].ty1;
		float tw = (m_Sprites[i].tx2 - m_Sprites[i].tx1);
		float th = (m_Sprites[i].ty2 - m_Sprites[i].ty1);
		float vertices[8] = {
			(-w / 2.0) + ax, (-h / 2.0) + ay,
			(w / 2.0) + ax, (-h / 2.0) + ay,
			(w / 2.0) + ax, (h / 2.0) + ay,
			(-w / 2.0) + ax, (h / 2.0) + ay
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
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
}

void AtlasSprite::Simulate(float deltaTime) {
	if (m_IsAlive) {
		float dx = m_Velocity[0] * deltaTime;
		float dy = m_Velocity[1] * deltaTime;
		SetPosition(m_Position[0] + dx, m_Position[1] + dy);
		m_Life += deltaTime;
		m_Frame = (int)(m_Life * m_AnimationSpeed);
	}
};
