#include "micropather.h"
#include <ctype.h>
#include <stdio.h>
#include <memory.h>
#include <math.h>

#include <vector>
#include <iostream>

#include "MemoryLeak.h"

#include "FooIO.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Model.h"
#include "octree.h"

#include "ModelOctree.h"

namespace micropather {
	


ModelOctree::ModelOctree(std::vector<Model *>m, Octree<int> o, int i) : m_Models(m), m_Scene(o), m_ModelIndex(i) {
}


ModelOctree::~ModelOctree() {
}


float ModelOctree::LeastCostEstimate( void* nodeStart, void* nodeEnd ) 
{
	//int xStart, yStart, xEnd, yEnd;
	//NodeToXY( nodeStart, &xStart, &yStart );
	//NodeToXY( nodeEnd, &xEnd, &yEnd );
	
	/* Compute the minimum path cost using distance measurement. It is possible
	 to compute the exact minimum path using the fact that you can move only 
	 on a straight line or on a diagonal, and this will yield a better result.
	 */
	
	//Vector3D *fooStart = (Vector3D*)nodeStart;
	//Vector3D *fooEnd = (Vector3D*)nodeEnd;
	//int dx = fooStart->x - fooEnd->x;
	//int dy = fooStart->y - fooEnd->y;
	//int dz = fooStart->z - fooEnd->z;
	
	int xStart, yStart, xEnd, yEnd;
    NodeToXY( nodeStart, &xStart, &yStart );
    NodeToXY( nodeEnd, &xEnd, &yEnd );
	int dx = xStart - xEnd;
	int dy = 0;
	int dz = yStart - yEnd;
	
	
	float least_cost = (float) sqrt( (double)(dx*dx) + (double)(dy*dy) + (double)(dz*dz));
	//LOGV("least_cost %f %f %f\n", fooStart->x, fooEnd->x, least_cost);
	return least_cost;
}

void ModelOctree::AdjacentCost( void* node, std::vector<StateCost> *neighbors ) 
{
	//Vector3D *pos = (Vector3D *)node;
	//int bx = pos->x + 32;
	//int by = pos->y + 32;
	//int bz = pos->z + 32;
	
	int ax, ay;
	
    NodeToXY(node, &ax, &ay);

	int bx = ax - 10 + 32;
	int by = 1 + 32;
	int bz = ay - 10 + 32;
	
	LOGV("starting %d %d\n", bx, bz);
	
	//int sx = m_Models[m_ModelIndex]->m_Position[0] + 32;
	//int sy = m_Models[m_ModelIndex]->m_Position[1] + 32;
	//int sz = m_Models[m_ModelIndex]->m_Position[2] + 32;

		
    const int dx[8] = { 1, 0, -1, 0};
    const int dz[8] = { 0, 1, 0, -1};
    const float cost[8] = { 1.0f, 1.01f, 1.0f, 1.01f};
	
	int colliding_index;
	
    for( int i=0; i<4; ++i ) {
		int nx = bx + dx[i];
		int nz = bz + dz[i];
		bool passable = false;
		colliding_index = m_Scene.at(nx, by, nz);
		if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
			/*
			//something is in my way
			if (m_Models[colliding_index]->m_IsStuck) {
			} else {
				//I can probably move this block
				//but it may harm me
				if (m_Models[colliding_index]->m_IsPlayer) {
					passable = true;
				}
			}
			*/
			if (m_Models[colliding_index]->m_IsPlayer) {
				LOGV("cannot move to this block %d %d %d %d\n", nx, by, nz, colliding_index);
			}
		} else {
			// there wasnt anything in front of me, but maybe below that there is?
			colliding_index = m_Scene.at(nx, by - 1, nz);
			if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
			//	//something to stand on
				if (m_Models[colliding_index]->m_IsStuck) {
					passable = true;
				}
			}
		}
		
		if (passable) {
			StateCost nodeCost = { XYToNode(nx - 32 + 10, nz - 32 + 10), cost[i] };
			neighbors->push_back(nodeCost);
		} else {

		}
	}
}

void ModelOctree::PrintStateInfo( void* node ) 
{
Vector3D *foo = (Vector3D*)node;
	LOGV("\n%f %f %f\n", foo->x, foo->y, foo->z);
}

}