//

#include "MemoryLeak.h"
#include "Model.h"
#include "Engine.h"
#include "octree.h"
#include "micropather.h"
#include "ModelOctree.h"
#include "PixelPusher.h"

PixelPusher::PixelPusher(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l) : Engine(w, h, t, m, l) {
	LOGV("PixelPusher::PixelPusher\n");
	m_Menu = new Model(m_FooFoos.at(3));
	m_Menu->SetTexture(m_Textures->at(5));
	m_Menu->SetFrame(0);
	m_Space = new Octree<int>(1024, -1);
	m_Touches = (float *)malloc(sizeof(float) * 4);
	m_Touches[0] = m_Touches[1] = m_Touches[2] = m_Touches[3] = 0;
}


void PixelPusher::Build() {
	m_CameraRotation = -45.0;
	m_CameraRotationSpeed = 0.0;
	m_CameraHeight = 50.0;
	m_CameraClimbSpeed = 0.0;
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	m_CameraPosition[0] = 0.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;
	Load(0);
	m_FooFoos.clear();



	


	
	
	
	
	
	
	
	//std::vector<void *> path;
	//m_Models[m_PlayerIndex]->SetSteps(&path);
	
	//Model *m = m_Models.at(m_PlayerIndex);
	ModelOctree *model_octree = new ModelOctree(m_Models, *m_Space, m_PlayerIndex);
	MicroPather micro_pather(model_octree);
	
	float totalCost;
	
	int x1 = 0;
	int y1 = 0;
	int z1 = 0;
	int x2 = 2;
	int y2 = 1;
	int z2 = 1;
	
	x1 = m_Models.at(m_PlayerIndex)->m_Position[0];
	y1 = m_Models.at(m_PlayerIndex)->m_Position[1];
	z1 = m_Models.at(m_PlayerIndex)->m_Position[2];
	
	x2 = m_Models.at(m_TargetIndex)->m_Position[0];
	y2 = m_Models.at(m_TargetIndex)->m_Position[1];
	z2 = m_Models.at(m_TargetIndex)->m_Position[2];
	
	aiVector3D *startState = new aiVector3D;
	startState->x = x1;
	startState->y = y1;
	startState->z = z1;
	
	aiVector3D *endState = new aiVector3D;
	endState->x = x2;
	endState->y = y2;
	endState->z = z2;
	
	int result = micro_pather.Solve(startState, endState, m_Models[m_PlayerIndex]->m_Steps, &totalCost);
	if (result == MicroPather::SOLVED) {
		LOGV("SOLVED\n");
		
		
		LOGV("MicroPather returned %d\n", result);
		
		
		unsigned k;
		// Wildly inefficient demo code.
		unsigned size = m_Models[m_PlayerIndex]->m_Steps->size();
		for (k=0; k<size; ++k) {
			
			aiVector3D *step = (aiVector3D *)m_Models[m_PlayerIndex]->m_Steps->at(k);
			
			LOGV("path %f, %f, %f\n", step->x, step->y, step->z);
			
			int path_index = m_Space->at(step->x + 32, step->y + 32 - 1, step->z + 32);
			if (path_index >= 0) {
				m_Models.at(path_index)->SetScale(1.5, 1.5, 1.5);
			}
			
			
			/*
			 x = index / 64;
			 y = index - x * 64;			
			 LOGV("path %d, %d\n", x, y);
			 
			 
			 
			 //CGPoint p = [layer positionAt:ccp(x,y)];
			 //p.x += 16;
			 //p.y += 16;
			 */
		}
	}
	


	
	
	
	
	
	
	
	
	
	
	
	
	
	



}


PixelPusher::~PixelPusher() {
	LOGV("dealloc GameController\n");
}


void PixelPusher::Render() {
	//m_Menu->Render();
}


void PixelPusher::Hit(float x, float y, int hitState) {
	float dx;
	float dy;
	int d1 = 0;
	int d2 = 0;
	int d3 = 0;
	int d4 = 0;
	int r = (((int)RadiansToDegrees(m_CameraRotation)) % 360);
	
	//LOGV("x:%f, y:%f\n", x, y);
	
	switch (hitState) {
		case 0:
			m_Touches[0] = x;
			m_Touches[1] = y;
			m_Touches[2] = x;
			m_Touches[3] = y;
			break;
			
		case 1:
			dx = x - m_Touches[2];
			dy = y - m_Touches[3];
			if (fabs(m_Touches[1]) > (m_ScreenHeight * 0.25)) {
        //do nothing
			} else {
        if (fabs(dx) > fabs(dy)) {
          m_CameraRotation += (DEGREES_TO_RADIANS(dx) * 0.4);
        } else {
          m_CameraHeight += (DEGREES_TO_RADIANS(dy) * 10.0);
        }
			}
			m_Touches[2] = x;
			m_Touches[3] = y;
			break;
			
		case 2:
			m_Touches[2] = x;
			m_Touches[3] = y;
			dx = m_Touches[2] - m_Touches[0];
			dy = m_Touches[3] - m_Touches[1];

			if ((r >= 0 && r <= 45) || (r <= 0 && r >= -45)) {
				d1 = 3;
				d2 = 1;
				d3 = 0;
				d4 = 2;
			} else if ((r >= 45 && r <= 135)) {
				d1 = 0;
				d2 = 2;
				d3 = 1;
				d4 = 3;
			} else if ((r <= -45 && r >= -135)) {
				d1 = 2;
				d2 = 0;
				d3 = 3;
				d4 = 1;
			} else if ((r >= 135 && r <= 225)) {
				d1 = 1;
				d2 = 3;
				d3 = 2;
				d4 = 0;
			} else if ((r <= -135 && r >= -225)) {
				d1 = 3;
				d2 = 1;
				d3 = 0;
				d4 = 2;			
			} else if ((r >= 225 && r <= 315)) {
				d1 = 2;
				d2 = 0;
				d3 = 3;
				d4 = 1;	
			} else if ((r <= -225 && r >= -315)) {
				d1 = 0;
				d2 = 2;
				d3 = 1;
				d4 = 3;
			} else if ((r >= 315 && r <= 360) || (r <= -315 && r >= -360)) {
				d1 = 3;
				d2 = 1;
				d3 = 0;
				d4 = 2;
			} else {
				LOGV("the fuck\n");
			}

			if (fabs(m_Touches[1]) > (m_ScreenHeight * 0.25) && (fabs(dx) > 50 || fabs(dy) > 50)) {
			  if (fabs(dx) > fabs(dy)) {
					if (dx > 0) {
						m_Models[m_PlayerIndex]->Move(d1);
					} else {
						m_Models[m_PlayerIndex]->Move(d2);
					}
        } else {
          if (dy > 0) {
            m_Models[m_PlayerIndex]->Move(d3);
          } else {
            m_Models[m_PlayerIndex]->Move(d4);
          }
        }
			}
			break;
			
		default:
			break;
	}
}


int PixelPusher::Simulate() {

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	for (unsigned int mm=0; mm<m_SimulatedModels.size(); mm++) {
		int m = m_SimulatedModels.at(mm);

		
		bool collided = false;
		int bx = m_Models[m]->m_Position[0] + 32;
		int by = m_Models[m]->m_Position[1] + 32;
		int bz = m_Models[m]->m_Position[2] + 32;

		
		int colliding_index = -1;
		int d = 1;
		for (unsigned int i=(bx - d); i<=(bx + d); i++) {
			for (unsigned int j=(by - d); j<=(by + d); j++) {
				for (unsigned int k=(bz - d); k<=(bz + d); k++) {
					//LOGV("xyz %d %d %d\n", i, j, k);
					colliding_index = m_Space->at(i, j, k);
					if (colliding_index >= 0 && colliding_index != m) {
						if (m_Models[m]->IsCollidedWith(m_Models[colliding_index])) {
							collided = true;
							if (m_Models[m]->m_IsPlayer && m_Models[colliding_index]->m_IsHarmfulToPlayers) {
								m_Models[colliding_index]->Harm(m_Models[m]);
							} else if (m_Models[colliding_index]->IsMovable()) {

								if (m_Models[m]->m_IsPlayer) {
									m_Models[colliding_index]->Move(m_Models[m]->m_Direction);
								} else if (m_Models[colliding_index]->m_IsPlayer) {
									m_Models[m]->Move(m_Models[colliding_index]->m_Direction);
								} else {
									m_Models[colliding_index]->Move(m_Models[m]->m_Direction);
								}
							} else {
								m_Models[m]->Stand();
							}
						}
					}
				}
			}
		}
		
		if (!collided) {
			//LOGV("foo :%d is fall\n", m);
			m_Models[m]->Fall();
		}
		
		m_Models[m]->Simulate(m_DeltaTime);
		if (m_Models[m]->m_IsMoving) {
			//
		} else {
			m_Space->erase(bx, by, bz);
			bx = m_Models[m]->m_Position[0] + 32;
			by = m_Models[m]->m_Position[1] + 32;
			bz = m_Models[m]->m_Position[2] + 32;
			if (by > 0) {
				m_Space->set(bx, by, bz, m);
			}
		}
	}
	
	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];

	float m_CameraDiameter = 75.0;
	float cx = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
	float cz = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];

	m_CameraTarget[0] = 0.0; //m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = 0.0; //m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = 0.0; //m_Models[m_PlayerIndex]->m_Position[2];
	
	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];
	
	m_CameraPosition[0] = cx;
	m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
	m_CameraPosition[2] = cz;
	
	m_Menu->Simulate(m_DeltaTime);
	
	
	
	
	
	
	
	
	
	
	
	
	
		
	
	
	
	
	
	
	
	
	
	
	return 1;
}

const char *PixelPusher::byte_to_binary(int x) {
	static char b[5];
	b[0] = '\0';
	int z;
	for (z = 16; z > 0; z >>= 1) {
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}

void PixelPusher::Load(int level_index) {
	int current[4];
	current[0] = current[1] = current[2] = current[3] = 0;
	char *level = (char *)malloc(sizeof(char) * m_LevelFoos->at(level_index)->len);

	fseek(m_LevelFoos->at(level_index)->fp, m_LevelFoos->at(level_index)->off, SEEK_SET);
	fread(level, sizeof(char), m_LevelFoos->at(level_index)->len, m_LevelFoos->at(level_index)->fp);

	unsigned int i = 0;
	unsigned int l = m_LevelFoos->at(level_index)->len;

	char *pos = NULL;
	const char *dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	int idx = -1;
	int *data = (int *)malloc(sizeof(int) * l);
	const char *code;
	for (unsigned int j=0; j<l; j++) {
		pos = index(dictionary, level[j]);
		if (pos != NULL) {
			idx = pos - dictionary;
			data[j] = idx;
		} else {
			throw 666;
		}
	}

	m_TerrainEndIndex = 0;

	int t = 0;

	while ( i < l ) {
		code = byte_to_binary(data[i++]);
		if ( code[1] == '1' ) current[0] += data[i++] - 32;
		if ( code[2] == '1' ) current[1] += data[i++] - 32;
		if ( code[3] == '1' ) current[2] += data[i++] - 32;
		if ( code[4] == '1' ) current[3] += data[i++] - 32;
		if ( code[0] == '1' ) {
			m_Models.push_back(new Model(m_FooFoos.at(0)));
			switch (current[3]) {
				case 9:
					t = 0;
					m_Models[m_TerrainEndIndex]->m_IsPlayer = false;
					m_Models[m_TerrainEndIndex]->m_IsHarmfulToPlayers = false;
					m_Models[m_TerrainEndIndex]->m_IsStuck = true;
					break;

				case 8:
					m_PlayerIndex = m_TerrainEndIndex;
					t = 1;
					m_Models[m_TerrainEndIndex]->m_IsPlayer = true;
					m_SimulatedModels.push_back(m_TerrainEndIndex);
					break;

				case 0:
					m_TargetIndex = m_TerrainEndIndex;
					t = 2;
					m_Models[m_TerrainEndIndex]->m_IsStuck = false;
					m_SimulatedModels.push_back(m_TerrainEndIndex);
					break;
					
				default:
					break;
			}

			m_Space->set(current[0] + 32, current[1] + 32, current[2] + 32, m_TerrainEndIndex);
			m_Models[m_TerrainEndIndex]->SetPosition(current[0], current[1], current[2]);
			m_Models[m_TerrainEndIndex]->SetTexture(m_Textures->at(t));
			m_Models[m_TerrainEndIndex]->SetFrame(0);
			m_Models[m_TerrainEndIndex]->SetScale(0.97, 0.97, 0.97);
			LOGV("wha %d %d %d %d\n", current[0], current[1], current[2], current[3]);
		}
		m_TerrainEndIndex++;
	}
	
	free(level);
	free(data);
}
