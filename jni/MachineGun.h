// Machine Gun Particle Effect

const int num_particles = 40;

class MachineGun {

public:

	MachineGun() {};
	MachineGun(GLuint texture);

	GLuint m_Texture;
	GLfloat *m_Vertices;

	void SetVertices(GLfloat *lineVertices) {
		m_Vertices = lineVertices;
	};
	
	GLfloat vertices[num_particles * 3];
	GLfloat colors[num_particles * 4];
	GLushort elements[num_particles];
	Vector3D generator[num_particles]; //keep track of generator (origin) for each particle
	Vector3D velocity[num_particles]; //keep track of velocity vector for each particle
	float alpha[num_particles]; //keep track of alpha for display
	float life[num_particles]; //keep track of life of particle
	GLuint myFountainTextures[1];
	Vector3D myFountainPosition;
	void buildFountain();
	void tickFountain();
	void drawFountain();
	void reset_life(int idx);
	float randf();
	void reset_vertex(int idx);
	void random_velocity(int idx);
	void reset_particle(int idx);
	void update_vertex(int idx);
	void update_color(int idx);

};
