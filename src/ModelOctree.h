

//using namespace micropather;

namespace micropather {


class ModelOctree : public micropather::Graph {

public:
	
	std::vector<Model *> *m_Models;
	Octree<int> *m_Scene;
	
	ModelOctree(std::vector<Model *> &m, Octree<int> &o, int i);
	~ModelOctree();

	
	float LeastCostEstimate( void* nodeStart, void* nodeEnd );
	void AdjacentCost( void* node, std::vector< StateCost > *neighbors );
	void PrintStateInfo( void* node );
	
	int m_ModelIndex;
	
	
	static void NodeToXY( void* node, int* x, int* y )
	{
		int index = (int)node;
		*y = index / 64;
		*x = index - *y * 64;
	}
	
	static void* XYToNode( int x, int y )
	{
		return (void*) ( y*64 + x );
	}
	
	void SetModelIndex (int i) {
		m_ModelIndex = i;
	};
	
};
	
};
