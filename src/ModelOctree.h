// Jon Bardin GPL

namespace micropather {

class ModelOctree : public micropather::Graph {

public:
	
	Octree<int> *m_Space;

	ModelOctree(Octree<int> &o);
	~ModelOctree();

	float LeastCostEstimate(void* nodeStart, void* nodeEnd);
	void AdjacentCost(void* node, std::vector<StateCost> *neighbors);
	void PrintStateInfo(void* node) {};

	static void NodeToXY(void* node, int* x, int* y) {
    int *data = reinterpret_cast<int*>(node);
    int index = *data;
    delete data;
		*y = index / 24;
		*x = index - *y * 24;
	}
	
	static void* XYToNode(int x, int y) {
		return (void *) ((y * 24) + x);
	}
};
	
};
