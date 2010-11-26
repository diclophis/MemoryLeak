

using namespace micropather;

class ModelOctree : public Graph {

public:
	
	std::vector<Model *> m_Models;
	Octree<int> m_Scene;
	
	ModelOctree(std::vector<Model *> m, Octree<int> o, int i);
	~ModelOctree();

	
	float LeastCostEstimate( void* nodeStart, void* nodeEnd );
	void AdjacentCost( void* node, std::vector< StateCost > *neighbors );
	void PrintStateInfo( void* node );
	
	int m_ModelIndex;
	
};
