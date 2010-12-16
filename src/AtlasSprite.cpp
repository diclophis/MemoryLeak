
#include "MemoryLeak.h"

#include "AtlasSprite.h"

AtlasSprite::AtlasSprite(GLuint t, int tw, int th, int sw, int sh, int s, int e, int spr, int rows) : m_Texture(t), m_TextureWidth(tw), m_TextureHeight(th), m_SpriteWidth(sw), m_SpriteHeight(sh), m_SpriteIndexStart(s), m_SpriteIndexEnd(e), m_SpritesPerRow(spr), m_Rows(rows) {
		
	m_Count = (m_SpriteIndexEnd - m_SpriteIndexStart) + 1;
	
	m_Sprites = new Sprite[m_Count];
	
	float tdx = 1.0 / (float)m_SpritesPerRow;
	float tdy = 1.0 / (float)m_Rows;
	int vdx = 50; //m_TextureWidth / m_SpritesPerRow;
	int vdy = 50; //m_TextureHeight / m_Rows;
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
	
	SetAnimation("000", 0);
}

void AtlasSprite::Render() {
	//LOGV("the fuck -- %d\n", m_Texture);
	//LOGV("the fuck: %d %s\n", (int)strlen(m_Animation), m_Animation);
	glEnable(GL_TEXTURE_2D);
	glClientActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_Texture);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	int ax = 0;
	int ay = 0;
	int sprites_to_draw = (int)strlen(m_Animation);
	for (unsigned int j = 0; j < sprites_to_draw; j++) {
		//int b = (i % m_SpritesPerRow);
		//ax = m_Sprites[i].dx * b;
		int i = m_Animation[j] - 48;
		float w = m_Sprites[i].dx;
		float h = m_Sprites[i].dy;
		float tx = m_Sprites[i].tx1;
		float ty = m_Sprites[i].ty1;
		float th = m_Sprites[i].tx2 - m_Sprites[i].tx1;
		float tw = m_Sprites[i].ty2 - m_Sprites[i].ty1;;
		float vertices[8] = {
			(-w / 2) + ax, (-h / 2) + ay,
			(w / 2) + ax, (-h / 2) + ay,
			(w / 2) + ax, (h / 2) + ay,
			(-w / 2) + ax, (h / 2) + ay
		};
		float texture[8] = {
			tx, ty,
			tx + tw, ty,
			tx + tw, ty + th,
			tx, ty + th
		};
		const GLubyte indices [] = {1, 2, 0, 3};
		glVertexPointer(2, GL_FLOAT, 0, vertices);
		glTexCoordPointer(2, GL_FLOAT, 0, texture);
		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, indices);
		//if (b == (m_SpritesPerRow - 1)) {
		//	ay += m_Sprites[i].dy;
		//}
		ax += m_Sprites[i].dx;
	}
	
	
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

}