// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "PixelPusher.h"

PixelPusher::PixelPusher(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs) : Engine(w, h, t, m, l, s, bs) {
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
	
	m_ModelOctree = new micropather::ModelOctree(m_Models, *m_Space, m_PlayerIndex);
	m_Pather = new micropather::MicroPather(m_ModelOctree);
	
	m_NumComets = 24;
	m_CometStart = 300.0;

	m_LastPumpedComet = 0.0;
	m_PumpCometTimeout = 1.0 / 6.0;
	
	m_LastForcePumpedComet = 0.0;
	m_PumpCometForceTimeout = 1.0;
	
	m_AtlasSprite = new AtlasSprite(m_Textures->at(0), 10, 10, "", 0, 60, 1.0);
	m_SpriteGun = new SpriteGun(m_Textures->at(0), 10, 10, "", 0, 60, 5.0, "", 0, 60, 1.0);

	m_AtlasSprite->SetPosition(100.0, 100.0);
	m_SpriteGun->SetPosition(100.0, 100.0);
	m_SpriteGun->Build(0);
	
	for (unsigned int i=0; i<m_NumComets; i++) {
		m_IceComets.push_back(new SpriteGun(m_Textures->at(0), 10, 10, "", 0, 60, 1.25, "", 0, 60, 2.25));
		m_IceComets[i]->SetPosition(0.0, 1000.0);
		m_IceComets[i]->SetVelocity(0.0, 0.0);
		m_IceComets[i]->m_IsAlive = false;
		m_IceComets[i]->m_IsReady = true;
		m_IceComets[i]->Build(10);
	}
}


PixelPusher::~PixelPusher() {
	LOGV("dealloc GameController\n");
}


void PixelPusher::Render() {
	m_SpriteGun->Render();
	m_AtlasSprite->Render();
	for (unsigned int i=0; i<m_NumComets; i++) {
		m_IceComets[i]->Render();
	}
}


void PixelPusher::Hit(float x, float y, int hitState) {
	
	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;
	float dpx = m_AtlasSprite->m_Position[0] - xx;
	float dpy = m_AtlasSprite->m_Position[1] - yy;

	xx = m_AtlasSprite->m_Position[0] - dpx;
	yy = m_AtlasSprite->m_Position[1] - dpy;
	
	m_AtlasSprite->SetPosition(xx, yy);
	m_SpriteGun->SetPosition(xx, yy);
	
	float dx;
	float dy;
	int d1 = 0;
	int d2 = 0;
	int d3 = 0;
	int d4 = 0;
	int r = (((int)RadiansToDegrees(m_CameraRotation)) % 360);
	
	
	//HACK: disable 3d input
	hitState = -1;
	
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
				throw 453;
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
	m_IsPushingAudio = false;
	
	m_AtlasSprite->Simulate(m_DeltaTime);
	m_SpriteGun->Simulate(m_DeltaTime);
	
	float s = (m_AudioBuffer[0] + m_AudioBuffer[1]) / 2;
	int fi = 0;
	bool forced = false;
	m_SpriteGun->SetEmitVelocity(fastSinf(m_SimulationTime * 20.0) * 1000.0, 1500.0);

	for (unsigned int i=0; i<m_NumComets; i++) {
		m_IceComets[i]->Simulate(m_DeltaTime);
		
		if ((m_IceComets[i]->m_Life > m_IceComets[i]->m_MaxLife)) {
			m_IceComets[i]->m_IsAlive = false;
			m_IceComets[i]->m_IsReady = true;
			m_IceComets[i]->m_Life = 0.0;
			m_IceComets[i]->SetPosition(0.0, 1000.0);
			m_IceComets[i]->SetVelocity(0.0, 0.0);
			m_IceComets[i]->Reset();
		}
	
		if (m_IceComets[i]->m_IsAlive) {
			m_IceComets[i]->m_EmitVelocity[0] = randf() * 150.0;
			m_IceComets[i]->m_EmitVelocity[1] = randf() * 150.0;
			m_IsPushingAudio = true;
		}

		float dx = fastAbs(m_IceComets[i]->m_Position[0] - m_AtlasSprite->m_Position[0]);
		float dy = fastAbs(m_IceComets[i]->m_Position[1] - m_AtlasSprite->m_Position[1]);
		if (dy < 40 && dx < 40) {
			m_IceComets[i]->Fire();
		} else {
			bool normal_ready = ((m_SimulationTime - m_LastPumpedComet) > m_PumpCometTimeout);
			bool force_ready = (fi < 3 && (m_SimulationTime - m_LastForcePumpedComet) > m_PumpCometForceTimeout);
			if (m_IceComets[i]->m_IsReady && (normal_ready || force_ready)) {
				m_LastPumpedComet = m_SimulationTime;
				if (force_ready) {
					forced = true;
					m_IceComets[i]->SetPosition(((fi++ * 50.0) - 50.0), m_CometStart);
				} else {
					if (m_IsPushingAudio) {
						m_IceComets[i]->SetPosition((s - 125.0), m_CometStart);
					} else {
						m_IceComets[i]->SetPosition(0.0, m_CometStart);
					}
				}
				m_IceComets[i]->SetVelocity(0.0, -1000.0);
				m_IceComets[i]->m_Life = 0.0;
				m_IceComets[i]->m_IsAlive = false;
				m_IceComets[i]->m_IsReady = false;
				m_IceComets[i]->Reset();
			}
		}
	}
	
	if (forced) {
		m_LastForcePumpedComet = m_SimulationTime;
	}
	
	if (!m_IsPushingAudio) {
		ModPlug_Seek(m_Sounds[0], 0);
	}
	
	m_LastAiSolved = -1;

	int collided_index = -1;
	
	int px = round(m_Models.at(m_PlayerIndex)->m_Position[0]);
	int pz = round(m_Models.at(m_PlayerIndex)->m_Position[2]);
	
	for (unsigned int m=0; m<m_SimulatedModels.size(); m++) {
		int i = m_SimulatedModels.at(m);
		if (!m_Models[i]->m_IsPlayer) {
			int bx = round(m_Models[i]->m_Position[0]);
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
			collided_index = m_Space->at(ax, ay, az);
			if (collided_index >= 0 && collided_index != i) {
				m_Models[i]->SetPosition((float)bx + ((float)dx * 0.4), (float)by + ((float)dy * 0.4), (float)bz + ((float)dz * 0.4));
				m_Models[i]->SetVelocity(ax - dx, ay - dy, az - dz);
				m_Models[i]->m_IsMoving = true;
			} else {
				m_Space->erase(bx, by, bz);
				m_Space->set(ax, ay, az, i);
			}
		}
	}
	 

	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];

	m_CameraRotation += DEGREES_TO_RADIANS(0.5);
	
	m_CameraHeight = 4.0 + (fastSinf(m_SimulationTime * 0.5) * 5.0); 
	float m_CameraDiameter = 5.0 + fastAbs(fastSinf(m_SimulationTime * 0.1) * 25.0);
	float cx = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
	float cz = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];

	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	
	//int ct = fastAbs(randf() * 5);
	m_CameraTarget[0] = fastSinf(m_SimulationTime * 0.5) + 10.0;//m_Models[ct]->m_Position[0];
	m_CameraTarget[1] = 4.0;
	m_CameraTarget[2] = fastSinf(m_SimulationTime * 0.5) + 10.0;//m_Models[ct]->m_Position[2];
	
	m_CameraPosition[0] = cx;
	m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
	m_CameraPosition[2] = cz;
	
	
	/*
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	
	m_CameraPosition[0] = 27.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;
	*/
	
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
			
			int ii = 0;
			
			if (current[3] == 8) {
				ii = 1;
			} else if (current[3] == 1) {
				ii = 3;
			} else if (current[3] == 2) {
				ii = 4;
			} else if (current[3] == 0) {
				ii = 2;
			} else {
			}
			
			m_Models.push_back(new Model(m_FooFoos.at(ii)));
			int height = current[1];
			switch (current[3]) {
				case 9:
					//black
					//t = 3;
					m_Models[m_TerrainEndIndex]->m_IsPlayer = false;
					m_Models[m_TerrainEndIndex]->m_IsHarmfulToPlayers = false;
					m_Models[m_TerrainEndIndex]->m_IsStuck = true;
					
					m_Models[m_TerrainEndIndex]->SetScale(0.99, 0.99, 0.99);

					break;

				case 8:
					//white
					m_PlayerIndex = m_TerrainEndIndex;
					//t = 4;
					height += 1.0;
					m_Models[m_TerrainEndIndex]->m_IsPlayer = true;
					m_SimulatedModels.push_back(m_TerrainEndIndex);

					m_Models[m_TerrainEndIndex]->SetScale(0.06, 0.06, 0.06);

					break;

				case 1:
					//yellow
					//t = 6;
					height += 5;
					m_Models[m_TerrainEndIndex]->m_IsStuck = true;
					m_Models[m_TerrainEndIndex]->SetScale(0.0095, 0.0095, 0.0095);

					break;
					
				case 2:
					//green
					//t = 7;
					height += 3;
					m_Models[m_TerrainEndIndex]->m_IsStuck = true;
					m_Models[m_TerrainEndIndex]->SetScale(0.0095, 0.0095, 0.0095);
					
					break;

				case 0:
					//red
					//t = 5;
					height += 1.0;
					m_Models[m_TerrainEndIndex]->m_IsStuck = false;
					m_SimulatedModels.push_back(m_TerrainEndIndex);
					
					m_Models[m_TerrainEndIndex]->SetScale(0.06, 0.06, 0.06);

					break;
					
				default:
					break;
			}

			m_Models[m_TerrainEndIndex]->SetPosition(current[0] + 10, height, current[2] + 10);
			m_Space->set((int)m_Models[m_TerrainEndIndex]->m_Position[0], (int)m_Models[m_TerrainEndIndex]->m_Position[1], (int)m_Models[m_TerrainEndIndex]->m_Position[2], m_TerrainEndIndex);
			m_Models[m_TerrainEndIndex]->SetTexture(m_Textures->at(t));
			m_Models[m_TerrainEndIndex]->SetFrame(0);
		}
		m_TerrainEndIndex++;
	}
	
	free(level);
	free(data);
}
