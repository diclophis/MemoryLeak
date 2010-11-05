// Machine Gun Particle Effect



class MachineGun : public Model {

public:

	//MachineGun(Assimp::Importer &importer);
	MachineGun(const aiScene *a) : Model(a) {
		build();
		LOGV("foo");
	};
	int m_NumParticles;
	std::vector<Model *> m_Particles;
	
	void build();
	
	void tickFountain();
	void drawFountain();
	void reset_particle(int idx);
	
};
