// Jon Bardin GPL


#include "MemoryLeak.h"
/*
#include "micropather.h"
#include <ctype.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include <vector>
#include <iostream>

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Model.h"
#include "octree.h"

#include "ModelOctree.h"
*/

namespace micropather {


ModelOctree::ModelOctree(std::vector<Model *> &m, Octree<int> &o, int i) : m_Models(&m), m_Scene(&o), m_ModelIndex(i) {
}


ModelOctree::~ModelOctree() {
}


float ModelOctree::LeastCostEstimate( void* nodeStart, void* nodeEnd ) 
{	
	int xStart, yStart, xEnd, yEnd;
    NodeToXY( nodeStart, &xStart, &yStart );
    NodeToXY( nodeEnd, &xEnd, &yEnd );
	int dx = xStart - xEnd;
	int dy = 0;
	int dz = yStart - yEnd;
	float least_cost = (float) sqrt( (double)(dx*dx) + (double)(dy*dy) + (double)(dz*dz));
	return least_cost;
}

void ModelOctree::AdjacentCost( void* node, std::vector<StateCost> *neighbors ) 
{
	int ax, ay;
	
    NodeToXY(node, &ax, &ay);

	int bx = ax;
	int by = 1;
	int bz = ay;
	
    const int dx[8] = { 1, 0, -1, 0};
    const int dz[8] = { 0, 1, 0, -1};
	
	int colliding_index;
	
	int lx = m_Models->at(m_ModelIndex)->m_Position[0] - bx;
	int lz = m_Models->at(m_ModelIndex)->m_Position[2] - bz;

	float look_distance = (float)sqrt( (double)(lx*lx) + (double)(lz*lz));
	
	if (look_distance > 10) {
		return;
	}
	
	float pass_cost = 0;
	
    for( int i=0; i<4; ++i ) {
		int nx = bx + dx[i];
		int nz = bz + dz[i];	
		bool passable = false;
		if (nx > 0 && nz > 0) {
			colliding_index = m_Scene->at(nx, by, nz);
			if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
				if (m_Models->at(colliding_index)->m_IsPlayer) {
					passable = true;
					pass_cost = 0.0;
				} else {
					passable = true;
					pass_cost = 100.0;
				}
			} else {
				// there wasnt anything in front of me, but maybe below that there is?
				colliding_index = m_Scene->at(nx, by - 1, nz);
				if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
				//	//something to stand on
					if (m_Models->at(colliding_index)->m_IsStuck) {
						pass_cost = 1.0;
						passable = true;
					}
				}
			}
		}
		
		if (passable) {
			StateCost nodeCost = { XYToNode(nx, nz), pass_cost };
			neighbors->push_back(nodeCost);
		} else {
		}
	}
}

}
