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
		//m_Models[m_ModelIndex]->SetPosition(bx - 32, by - 32, bz - 32);
		bool passable = false;
		colliding_index = m_Scene.at(nx, by, nz);
		if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
			//something is in my way
			if (m_Models[colliding_index]->m_IsStuck) {
			} else {
				//I can probably move this block
				//but it may harm me
				passable = true;
			}
		} else {
			// there wasnt anything in front of me, but maybe below that there is?
			colliding_index = m_Scene.at(nx, by - 1, nz);
			if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
				//something to stand on
				passable = true;
			}
		}
		
		if (passable) {

			
			//aiVector3D *nextStep = new aiVector3D;
			//nextStep->x = nx - 32;
			//nextStep->y = by - 32;
			//nextStep->z = nz - 32;
			//StateCost nodeCost = {nextStep, cost[i]};
			
			StateCost nodeCost = { XYToNode(nx - 32 + 10, nz - 32 + 10), cost[i] };

			
			neighbors->push_back(nodeCost);

		} else {

		}
	}
	
	
	/*
	 int colliding_index = -1;
	 int d = 1;
	for (unsigned int i=(bx - d); i<=(bx + d); i++) {
		for (unsigned int k=(bz - d); k<=(bz + d); k++) {
			if (i == bx && k == bz) {
				//LOGV("my position\n");
			} else if (bx == i || bz == k) {
				colliding_index = m_Scene.at(i, j, k);
				if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
					//found a brick
					if (m_Models[colliding_index]->m_IsStuck) {
						LOGV("wall\n");
					} else if (m_Models[colliding_index]->m_IsPlayer) {
						LOGV("the fuck!!!\n");
					} else {
						
						//Vector3D *nextStep = new Vector3D;
						//nextStep->x = m_Models[colliding_index]->m_Position[0];
						//nextStep->y = m_Models[colliding_index]->m_Position[1];
						//nextStep->z = m_Models[colliding_index]->m_Position[2];
						//StateCost nodeCost = {nextStep, 1.0};
						//neighbors->push_back(nodeCost);
						
					}
				} else {
					//no brick in this direction
					//but check if there one below it
					colliding_index = m_Scene.at(i, j - 1, k);
					if (colliding_index >= 0 && colliding_index != m_ModelIndex) {
						//LOGV("I am standing here => %d %d %d\n", (int)pos->x, (int)pos->y, (int)pos->z);
						//LOGV("I am going    here => %d %d %d\n", (int)i - 32, j - 32, k - 32);
						Vector3D *nextStep = new Vector3D;
						nextStep->x = m_Models[colliding_index]->m_Position[0];
						nextStep->y = m_Models[colliding_index]->m_Position[1] + 1;
						nextStep->z = m_Models[colliding_index]->m_Position[2];
						StateCost nodeCost = {nextStep, 1.0};
						neighbors->push_back(nodeCost);
					}
				}
			} else {
				//LOGV("diag\n");
			}
		}
	}
	
	//LOGV("what\n");
	 */
}

void ModelOctree::PrintStateInfo( void* node ) 
{
Vector3D *foo = (Vector3D*)node;
	LOGV("\n%f %f %f\n", foo->x, foo->y, foo->z);
}

}