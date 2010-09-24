// Machine Gun Fountain

#include "math.h"
#include <sstream>

#include "OpenSteer/Vec3.h"
#include "importgl.h"
#include "OpenGLCommon.h"
#include "MachineGun.h"


MachineGun::MachineGun(GLuint texture, GLfloat *lineVertices) {
	m_Texture = texture;
	m_Vertices = lineVertices;
}

//returns a random float between 0 and 1
float MachineGun::randf() {
	//random hack since no floating point random function
	//optimize later
	return (lrand48() % 255) / 255.f;
}


void MachineGun::reset_vertex(int idx) {
	int i = idx * 3;
	vertices[i + 0] = generator[idx].x;
	vertices[i + 1] = generator[idx].y;
	vertices[i + 2] = generator[idx].z;
}


void MachineGun::random_velocity(int idx) {
	OpenSteer::Vec3 a = OpenSteer::Vec3(m_Vertices[0], m_Vertices[1], m_Vertices[2]);
	OpenSteer::Vec3 b = a.normalize();
	b *= 2.0;
	velocity[idx].x = b.x + (randf() * 0.1 - 0.05);
	velocity[idx].y = b.y + (randf() * 0.1 - 0.05);
	velocity[idx].z = b.z + (randf() * 0.1 - 0.05);
}


void MachineGun::reset_particle(int idx) {

	generator[idx].x = m_Vertices[3];
	generator[idx].y = m_Vertices[4];
	generator[idx].z = m_Vertices[5];
	
	reset_vertex(idx);
	random_velocity(idx);
	reset_life(idx);
}


void MachineGun::update_vertex(int idx) {
	int i = idx * 3;
	vertices[i] += velocity[idx].x;
	vertices[i+1] += velocity[idx].y;
	vertices[i+2] += velocity[idx].z;
	//velocity[idx].y += 0.1 * randf(); 
}


/*
static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,1.0f,1.0f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},
	{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},
	{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f}
};
 */

static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,0.0f,0.0f},{0.0f,0.9f,0.1f},{0.0f,0.9f,0.1f},{0.0f,0.9f,0.1f},
	{0.0f,0.5f,0.1f},{0.0f,0.5f,0.1f},{0.0f,0.5f,0.1f},{0.0f,0.5f,0.5f},
	{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f}
};


void MachineGun::update_color(int idx) {
	int i = idx * 4;
	//float distanceFromPlayer = myPlayerPosition.x - vertices[idx * 3];
	//float percentOf = (distanceFromPlayer) / 40.0;
	//int ii = randf() * 11;//(int)(percentOf * 12);
	//if (ii > 11) {
	//	ii = 11;
	//}
	int ii = 0;
	
	colors[i+0] = ccolors[ii][0];
	colors[i+1] = ccolors[ii][1];
	colors[i+2] = ccolors[ii][2];
	colors[i+3] = 1.0; //i / (float)11.0;	
}


void MachineGun::reset_life(int i) {
	life[i] = (randf() * 50.0);
}


void MachineGun::buildFountain() {	
	srand48(time(NULL));
	
	int i = 0;
	for(i=0;i<num_particles;i++) {
		elements[i] = i;
	}
	
	for(i=0;i<num_particles;i++) {
		reset_particle(i);
		//life[i] -= (float)i * (0.01);
	}	
}


void MachineGun::tickFountain() {	
	int i = 0; //particle index
	for(i=0; i<num_particles; i++) {
		life[i] -= 1.0;
		if(life[i] <= 0.0) {
			reset_particle(i);
		} else {
			update_color(i);
			update_vertex(i);
		}
	}
}


void MachineGun::drawFountain() {
	
	
	/*
	 glBindTexture(GL_TEXTURE_2D, 0);
	 glEnableClientState(GL_VERTEX_ARRAY);
	 glColor4f(1.0, 0.0, 0.0, 1.0);
	 glLineWidth(2.0);
	 glVertexPointer(3, GL_FLOAT, 0, myLineVertices);
	 glDrawArrays(GL_LINES, 0, 2);
	 glDisableClientState(GL_VERTEX_ARRAY);
	 glColor4f(1.0, 1.0, 1.0, 1.0);
	 */
	
	
	
	//glDisable(GL_DEPTH_TEST);
	//glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);

	glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	//glBlendFunc(GL_DST_COLOR, GL_ONE);
	
	if (true) {
		glBindTexture(GL_TEXTURE_2D, m_Texture);

#ifdef DESKTOP
		glEnable(GL_POINT_SPRITE);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
#else
		glEnable(GL_POINT_SPRITE_OES);
		glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
#endif
		
		glEnableClientState(GL_VERTEX_ARRAY); 
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		int split = 5;
		int time = num_particles / split;
		for (int i=0; i<split; i++) {
			glPointSize(50.0 - (i * 10));
			glDrawArrays(GL_POINTS, i * time, time);
		}
		
#ifdef DESKTOP
		glDisable(GL_POINT_SPRITE);
#else
		glDisable(GL_POINT_SPRITE_OES);
#endif
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_FLOAT, 0, colors);
		glPointSize(2.0);
		glDrawElements(GL_POINTS, num_particles, GL_UNSIGNED_SHORT, elements);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
	glDisable(GL_BLEND);
}
