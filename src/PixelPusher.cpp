//

#include "MemoryLeak.h"

#include "octree.h"
#include "micropather.h"

#include "Model.h"
#include "ModelOctree.h"

#include "Engine.h"

#include "AtlasSprite.h"
#include "SpriteGun.h"

#include "PixelPusher.h"

PixelPusher::PixelPusher(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l) : Engine(w, h, t, m, l) {
	LOGV("PixelPusher::PixelPusher\n");
	m_Menu = new Model(m_FooFoos.at(3));
	m_Menu->SetTexture(m_Textures->at(5));
	m_Menu->SetScale(0.1, 0.1, 0.1);
	m_Menu->SetFrame(0);
	m_Space = new Octree<int>(1024, -1);
	m_Touches = (float *)malloc(sizeof(float) * 4);
	m_Touches[0] = m_Touches[1] = m_Touches[2] = m_Touches[3] = 0;
	m_CircleIndex = 0;
	m_AiIndex = 0;
	m_LastAiSolved = -1;
}


void PixelPusher::Build() {

	m_CameraRotation = -33.0;
	m_CameraRotationSpeed = 0.0;
	m_CameraHeight = 10.0;
	m_CameraClimbSpeed = 0.0;
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	m_CameraPosition[0] = 0.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;
	Load(0);
	m_FooFoos.clear();
	
	m_ModelOctree = new micropather::ModelOctree(m_Models, *m_Space, m_PlayerIndex);
	m_Pather = new micropather::MicroPather(m_ModelOctree);
	
	LOGV("the fuck: 123 - %d %d\n", (int)m_Textures->size(), m_Textures->at(0));
	

	m_AtlasSprite = new AtlasSprite(m_Textures->at(5), 8, 8);	
	m_SpriteGun = new SpriteGun(m_Textures->at(5), 8, 8);
}


PixelPusher::~PixelPusher() {
	LOGV("dealloc GameController\n");
}


void PixelPusher::Render() {
	m_AtlasSprite->Render();
	m_SpriteGun->Render();
}


void PixelPusher::Hit(float x, float y, int hitState) {
	
	m_SpriteGun->SetPosition(x - (0.5 * m_ScreenWidth), 0.5 * m_ScreenHeight - y);
	
	m_AtlasSprite->SetPosition(x - (0.5 * m_ScreenWidth), 0.5 * m_ScreenHeight - y);

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
						m_Models.at(m_PlayerIndex)->Move(d1);
					} else {
						m_Models.at(m_PlayerIndex)->Move(d2);
					}
        } else {
          if (dy > 0) {
            m_Models.at(m_PlayerIndex)->Move(d3);
          } else {
            m_Models.at(m_PlayerIndex)->Move(d4);
          }
        }
			}
			break;
			
		default:
			break;
	}
}


int PixelPusher::Simulate() {
	
	m_AtlasSprite->Simulate(m_DeltaTime);
	m_SpriteGun->Simulate(m_DeltaTime);
	
	/*
	for (unsigned int i = 0; i < 20; i++) {
		for (unsigned int j = 0; j < 20; j++) {
			for (unsigned int k = 0; k < 20; k++) {
				m_Space->erase(i, j, k);
			}
		}
	}
	
	for (unsigned int i=0; i<m_Models.size(); i++) {
		int rx = round(m_Models[i]->m_Position[0]);
		int ry = round(m_Models[i]->m_Position[1]);
		int rz = round(m_Models[i]->m_Position[2]);
		m_Space->set(rx, ry, rz, i);
	}
	*/
	
	
	m_LastAiSolved = -1;

	int collided_index = -1;
	
	int px = round(m_Models.at(m_PlayerIndex)->m_Position[0]);
	//int py = round(m_Models.at(m_PlayerIndex)->m_Position[1]);
	int pz = round(m_Models.at(m_PlayerIndex)->m_Position[2]);
	
	for (unsigned int m=0; m<m_SimulatedModels.size(); m++) {
		int i = m_SimulatedModels.at(m);
		if (!m_Models[i]->m_IsPlayer) {
			int bx = round(m_Models[i]->m_Position[0]);
			//int by = round(m_Models[i]->m_Position[1]);
			int bz = round(m_Models[i]->m_Position[2]);
			void *startState = micropather::ModelOctree::XYToNode(bx, bz);
			void *endState = micropather::ModelOctree::XYToNode(px, pz);
			float totalCost;
			m_Pather->Reset();
			m_ModelOctree->SetModelIndex(i);
			int solved = m_Pather->Solve(startState, endState, m_Models[i]->m_Steps, &totalCost);
			switch (solved) {
				case micropather::MicroPather::SOLVED:
					//LOGV("solved\n");
					break;
				case micropather::MicroPather::NO_SOLUTION:
					//LOGV("none\n");
					break;
				case micropather::MicroPather::START_END_SAME:
					//LOGV("same\n");
					break;	
				default:
					break;
			}
		}
	}
	
	for (unsigned int m=0; m<m_SimulatedModels.size(); m++) {
		int i = m_SimulatedModels.at(m);
		
		int bx = round(m_Models[i]->m_Position[0]);
		int by = round(m_Models[i]->m_Position[1]);
		int bz = round(m_Models[i]->m_Position[2]);
		
		m_Models[i]->Simulate(m_DeltaTime, false);
		
		int ax = round(m_Models[i]->m_Position[0]);
		int ay = round(m_Models[i]->m_Position[1]);
		int az = round(m_Models[i]->m_Position[2]);

		int dx = ax - bx;
		int dy = ay - by;
		int dz = az - bz;
		
		if (bx != ax || by != ay || bz != az) {
			//LOGV("%d delta(%d %d %d)\n", i, dx, dy, dz);
			collided_index = m_Space->at(ax, ay, az);
			if (collided_index >= 0 && collided_index != i) {
				//LOGV("%d collided (%d %d %d) => (%d %d %d)\n", i, bx, by, bz, ax, ay, az);
				m_Models[i]->SetPosition((float)bx + ((float)dx * 0.4), (float)by + ((float)dy * 0.4), (float)bz + ((float)dz * 0.4));
				m_Models[i]->SetVelocity(ax - dx, ay - dy, az - dz);
				m_Models[i]->m_IsMoving = true;
			} else {
				//LOGV("%d moved (%d %d %d) => (%d %d %d)\n", i, bx, by, bz, ax, ay, az);
				m_Space->erase(bx, by, bz);
				m_Space->set(ax, ay, az, i);
			}
		}
	}

	/*
	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];

	float m_CameraDiameter = 20.0;
	float cx = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
	float cz = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];

	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	
	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];
	
	m_CameraPosition[0] = cx;
	m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
	m_CameraPosition[2] = cz;
	*/
	
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	
	m_CameraPosition[0] = 27.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;
	
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
					
					m_Models[m_TerrainEndIndex]->SetScale(0.99, 0.99, 0.99);

					break;

				case 8:
					m_PlayerIndex = m_TerrainEndIndex;
					t = 1;
					m_Models[m_TerrainEndIndex]->m_IsPlayer = true;
					m_SimulatedModels.push_back(m_TerrainEndIndex);
					
					m_Models[m_TerrainEndIndex]->SetScale(0.6, 0.6, 0.6);

					break;

				case 0:
					m_TargetIndex = m_TerrainEndIndex;
					t = 2;
					m_Models[m_TerrainEndIndex]->m_IsStuck = false;
					m_SimulatedModels.push_back(m_TerrainEndIndex);
					
					m_Models[m_TerrainEndIndex]->SetScale(0.75, 0.75, 0.75);

					break;
					
				default:
					break;
			}

			m_Models[m_TerrainEndIndex]->SetPosition(current[0] + 10, current[1], current[2] + 10);
			m_Space->set((int)m_Models[m_TerrainEndIndex]->m_Position[0], (int)m_Models[m_TerrainEndIndex]->m_Position[1], (int)m_Models[m_TerrainEndIndex]->m_Position[2], m_TerrainEndIndex);
			m_Models[m_TerrainEndIndex]->SetTexture(m_Textures->at(t));
			m_Models[m_TerrainEndIndex]->SetFrame(0);
		}
		m_TerrainEndIndex++;
	}
	
	free(level);
	free(data);
}
