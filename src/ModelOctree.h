// Jon Bardin GPL

namespace micropather {

class ModelOctree : public micropather::Graph {

public:
	
	std::vector<Model *> *m_Models;
	Octree<int> *m_Scene;
	int m_ModelIndex;

	ModelOctree(std::vector<Model *> &m, Octree<int> &o, int i);
	~ModelOctree();
	float LeastCostEstimate( void* nodeStart, void* nodeEnd );
	void AdjacentCost( void* node, std::vector< StateCost > *neighbors );
	void PrintStateInfo( void* node ) {};

	static void NodeToXY( void* node, int* x, int* y )
	{
    int*  data = reinterpret_cast<int*>(node);
    int   index    = *data;
    delete data;


		//int index = (int)node;
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
