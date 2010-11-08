// Machine Gun Particle Effect



class MachineGun : public Model {

public:

	//MachineGun(Assimp::Importer &importer);
	MachineGun(const aiScene *a) : Model(a) {
		build();
	};
	int m_NumParticles;
	std::vector<Model *> m_Particles;
	
	void build();
	
	void reset_particle(int idx);
	float Tick(float deltaTime);
	void render();
	int m_Last;
};