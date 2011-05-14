// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"
#include "BlockBuster.h"


BlockBuster::BlockBuster(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_CameraOffsetX = 0;
	m_LastSwept = 0;
	m_SweepTimeout = 0.0;
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, 10, 11, 1.0, "", 10, 11, 0.5));
	m_AtlasSprites[0]->SetPosition(128 * 50.0, 100.0);
	m_AtlasSprites[0]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[0]->m_IsAlive = true;
	m_AtlasSprites[0]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[0]->SetScale(1.0, 1.0);
	m_AtlasSprites[0]->Build(20);
	
	m_PlayerIsFalling = true;
	
	m_Space = new Octree<int>(512 * 512, -1);

	m_FloorSize = 50.0;
	
	m_FloorBufferCount = 64;
	m_FloorBufferStartIndex = m_AtlasSprites.size();
	
	m_LastFloorX = m_AtlasSprites[0]->m_Position[0] - ((m_FloorBufferCount / 2) * m_FloorSize);
	
	for (int i=m_FloorBufferStartIndex; i<m_FloorBufferStartIndex + m_FloorBufferCount; i++) {
		m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(7), m_FloorBufferCount, 1, 0, m_FloorBufferCount, 0.25, "", 0, m_FloorBufferCount, 0.25));
		//m_AtlasSprites[i]->SetPosition(m_LastFloorX, m_LastFloorY);
		//m_Space->set((int)(m_LastFloorX / m_FloorSize), (int)(m_LastFloorY / m_FloorSize) + 256, 0, i);
		

		m_AtlasSprites[i]->Build(2);
		
		SweepFloorAt(i);		
	}
	
	int f = m_AtlasSprites.size();
	
	for (unsigned int i=0; i<31; i++) {
		m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 8, 8, i%8, (i%8)+1 , 0.5, "", 0, 1, 1.0));
		m_AtlasSprites[f+i]->SetPosition(0.0, 0.0);
		m_AtlasSprites[f+i]->SetVelocity(0.0, 0.0);
		m_AtlasSprites[f+i]->m_IsAlive = false;
		m_AtlasSprites[f+i]->SetEmitVelocity(0.0, 0.0);
		m_AtlasSprites[f+i]->Build(0);
	}
	
	colliding_indexes = (int *)malloc(sizeof(int) * 31);
}

BlockBuster::~BlockBuster() {
}

void BlockBuster::Hit(float x, float y, int hitState) {
	//m_AtlasSprites[0]->m_Velocity[0] += 50.0;
	//m_PlayerIsFalling = true;
	
	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;
	
	//m_AtlasSprites[0]->m_Position[1] = yy;
	
	/*
	if (xx > 0) {
		m_AtlasSprites[0]->m_Velocity[0] += 500.0;
	} else {
		m_AtlasSprites[0]->m_Velocity[0] -= 500.0;
	}
	
	if (yy > 0) {
		m_AtlasSprites[0]->m_Velocity[1] = 3000.0;
	} else {
		//m_AtlasSprites[0]->m_Velocity[1] = 150.0;
	}
	*/
	m_AtlasSprites[0]->m_Position[1] = yy * m_SimulationTime;
}

void BlockBuster::Build() {
  m_IsPushingAudio = true;
}

inline float clamp(float x, float a, float b){    return x < a ? a : (x > b ? b : x);};

int BlockBuster::Simulate() {
	
	if (m_AtlasSprites[0]->m_Velocity[0] > 8000.0) {
		m_AtlasSprites[0]->m_Velocity[0] = 8000.0;
	}
	
	if (m_AtlasSprites[0]->m_Velocity[0] < 0.0) {
		m_AtlasSprites[0]->m_Velocity[0] = 0.0;
	}

	float check_x = (int)((m_AtlasSprites[0]->m_Position[0] + 25.0) / m_FloorSize);
	float check_y = ((m_AtlasSprites[0]->m_Position[1]) / m_FloorSize) + 256.0;
	
	for (unsigned int i=0; i<31; i++) {
		int y = ((int)check_y + i - 15);
		//LOGV("the fuck: %f, %d\n", m_AtlasSprites[0]->m_Position[1], (int)(check_y + (i - 5)));
		colliding_indexes[i] = m_Space->at(check_x, y, 0);
		
		
		m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + i]->m_Position[0] = (check_x) * 50.0;
		m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + i]->m_Position[1] = (y - 256.0) * 50.0;
	}
	
	/*
	colliding_indexes[0] = m_Space->at(check_x + 1, check_y + 0, 0);
	colliding_indexes[1] = m_Space->at(check_x + 1, check_y - 1, 0);
	colliding_indexes[2] = m_Space->at(check_x + 0, check_y - 1, 0);
	colliding_indexes[3] = m_Space->at(check_x - 1, check_y - 1, 0);
	colliding_indexes[4] = m_Space->at(check_x - 1, check_y + 0, 0);
	colliding_indexes[5] = m_Space->at(check_x - 1, check_y + 1, 0);
	colliding_indexes[6] = m_Space->at(check_x + 0, check_y + 1, 0);
	colliding_indexes[7] = m_Space->at(check_x + 1, check_y + 1, 0);
	colliding_indexes[8] = m_Space->at(check_x, check_y, 0);
	*/
	
	m_PlayerIsFalling = true;
	
	float avg_y = 0;
	int avg_y_count = 0;
	
	//bool would_get_stuck = false;
	
	//float old_speed = m_AtlasSprites[0]->m_Velocity[0];
	
	bool needs_y = true;
	
	float right_y = -FLT_MAX;
	float left_y = -FLT_MAX;
	
	int right_win = -1;
	int left_win = -1;
	
	for (unsigned int i=0; i<31; i++) {
		if (colliding_indexes[i] != -1) {
			needs_y = false;
			m_AtlasSprites[colliding_indexes[i]]->m_IsAlive = true;

			/*
			if (i == 7 || i == 0 || i == 1) {
				if (m_AtlasSprites[colliding_indexes[i]]->m_Position[1] > right_y) {
					right_win = i;
					m_AtlasSprites[colliding_indexes[i]]->m_IsAlive = true;
					right_y = m_AtlasSprites[colliding_indexes[i]]->m_Position[1];
				}
			}
			
			if (i == 3 || i == 4 || i == 5) {
				if (m_AtlasSprites[colliding_indexes[i]]->m_Position[1] > left_y) {
					left_win = i;
					left_y = m_AtlasSprites[colliding_indexes[i]]->m_Position[1];
				}
			}	
			*/
			
			/*
			float delta_collide_x = fastAbs(m_AtlasSprites[0]->m_Position[0] - m_AtlasSprites[colliding_indexes[i]]->m_Position[0]);
			float delta_collide_future_x = fastAbs(m_AtlasSprites[0]->m_Position[0] + (m_AtlasSprites[0]->m_Velocity[0] * m_DeltaTime * 2.0) - m_AtlasSprites[colliding_indexes[i]]->m_Position[0]);
			float delta_collide_y = fastAbs(m_AtlasSprites[0]->m_Position[1] - m_AtlasSprites[colliding_indexes[i]]->m_Position[1]);
			float delta_collide_future_y = fastAbs(m_AtlasSprites[0]->m_Position[1] + (m_AtlasSprites[0]->m_Velocity[1] * m_DeltaTime * 2.0) - m_AtlasSprites[colliding_indexes[i]]->m_Position[1]);


			if (needs_y && delta_collide_future_y < 49.0) {
				if (m_AtlasSprites[0]->m_Velocity[0] > 0) {
					if (i == 0) {
						LOGV("collide: %d\n", i);
						avg_y = m_AtlasSprites[colliding_indexes[i]]->m_Position[1];
						needs_y = false;

					}
				} else {
					if (i == 4) {
						avg_y = m_AtlasSprites[colliding_indexes[i]]->m_Position[1];
						needs_y = false;

					}
				}
			}
			*/
		}
	}
	
	/*
	if (needs_y == false) {
		if (m_AtlasSprites[0]->m_Velocity[1] <= 0 || avg_y > m_AtlasSprites[0]->m_Position[1]) {
			LOGV("avg_y: %f, y: %f\n", avg_y, m_AtlasSprites[0]->m_Position[1]);
			m_AtlasSprites[0]->m_Velocity[1] = 0.0;
			m_AtlasSprites[0]->m_Position[1] = avg_y;
			m_PlayerIsFalling = false;
		}
	}
	*/
	
	if (!needs_y) {
		
		m_AtlasSprites[0]->m_Velocity[0] += 10.0;
	} else {
		m_SimulationTime = 1.0;
		m_AtlasSprites[0]->m_Velocity[0] = 100.0;
	}
	
	/*
	if (right_win != -1 && left_win != -1) {
		//LOGV("win: %d %d %f %f\n", right_win, left_win, right_y, left_y);

		if (m_AtlasSprites[0]->m_Velocity[1] <= 0) {			
			//m_AtlasSprites[0]->m_Velocity[1] = 0.0;
			//m_AtlasSprites[0]->m_Position[1] = (right_y + left_y) * 0.5;
			m_PlayerIsFalling = false;

		}
	}
	*/
	
	//if (m_PlayerIsFalling) {
	//	m_AtlasSprites[0]->m_Velocity[1] -= (4000.0 * m_DeltaTime);
	//}
		
	/*
	if (would_get_stuck && avg_delta_y_count > 1) {
		float avg = (avg_delta_y / (float)avg_delta_y_count);
		if (avg < 10.0) {
			m_AtlasSprites[0]->m_Velocity[0] = 0.0;
		} else {
			m_AtlasSprites[0]->m_Velocity[0] = old_speed;
			if (colliding_indexes[0] != -1) {
				m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[colliding_indexes[0]]->m_Position[1] + m_FloorSize;
			} else {
				m_AtlasSprites[0]->m_Position[1] = m_AtlasSprites[colliding_indexes[1]]->m_Position[1] + m_FloorSize;
			}
		}
	}
	*/
	
	/*	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 0]->m_Position[0] = (check_x + 1.0) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 0]->m_Position[1] = (check_y - 256.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 1]->m_Position[0] = (check_x + 1.0) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 1]->m_Position[1] = (check_y - 256.0 - 1.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 2]->m_Position[0] = (check_x + 0.0) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 2]->m_Position[1] = (check_y - 256.0 - 1.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 3]->m_Position[0] = (check_x - 1) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 3]->m_Position[1] = (check_y - 256.0 - 1.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 4]->m_Position[0] = (check_x -1) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 4]->m_Position[1] = (check_y - 256.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 5]->m_Position[0] = (check_x - 1) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 5]->m_Position[1] = (check_y - 256.0 + 1.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 6]->m_Position[0] = (check_x) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 6]->m_Position[1] = (check_y - 256.0 + 1.0) * 50.0;
	
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 7]->m_Position[0] = (check_x + 1) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 7]->m_Position[1] = (check_y - 256.0 + 1.0) * 50.0;

	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 8]->m_Position[0] = (check_x) * 50.0;
	m_AtlasSprites[m_FloorBufferStartIndex + m_FloorBufferCount + 8]->m_Position[1] = (check_y - 256.0) * 50.0;
	
	*/
	
	//if (m_AtlasSprites[0]->m_Position[1] < -600) {
	//	m_AtlasSprites[0]->SetVelocity(0.0, 0.0);
	//}
	
	m_AtlasSprites[0]->Simulate(m_DeltaTime);

	for (unsigned int i=0; i<m_FloorBufferCount; i++) {
		if (needs_y) {
			m_AtlasSprites[m_FloorBufferStartIndex + i]->m_IsAlive = false;
		}
		
		m_AtlasSprites[m_FloorBufferStartIndex + i]->Simulate(m_DeltaTime);
		if (m_AtlasSprites[m_FloorBufferStartIndex + i]->m_Position[0] < m_AtlasSprites[0]->m_Position[0] - ((m_FloorBufferCount / 2) * m_FloorSize)) {
			SweepFloorAt(m_FloorBufferStartIndex + i);
		}


		//m_AtlasSprites[m_FloorBufferStartIndex + i]->SetScale(1.0, 200.0 + (fastSinf((m_SimulationTime + (i * 0.1))) * 25.0));
	}


	m_SweepTimeout += m_DeltaTime;

	float camera_delta = (m_CameraOffsetX - m_AtlasSprites[0]->m_Position[0]);
	
	if (camera_delta < -75.0) {
		m_CameraSpeed = (m_AtlasSprites[0]->m_Velocity[0] * 1.01);
	} else if (camera_delta > 75.0) {
		m_CameraSpeed = m_AtlasSprites[0]->m_Velocity[0] * 0.98;
	} else {
		//m_CameraSpeed = m_AtlasSprites[0]->m_Velocity[0];
	}
	
	m_CameraOffsetX = m_AtlasSprites[0]->m_Position[0];
	
	return 1;
}


void BlockBuster::SweepFloorAt(int i) {
	//LOGV("sweep %d\n", i);
	float sweep_x = 0;
	float sweep_y = 0;
	sweep_x = (int)((m_AtlasSprites[i]->m_Position[0]) / m_FloorSize);
	sweep_y = (int)((m_AtlasSprites[i]->m_Position[1]) / m_FloorSize) + 256.0;
	m_Space->set(sweep_x, sweep_y, 0, -1);
	m_LastFloorX += 50;
	float possible_floor_y = fastSinf(m_LastFloorX * (0.25)) * (1000.0 + (fastSinf(m_SimulationTime) * 300.0));
	float delta = 0;
	delta = (possible_floor_y - m_LastFloorY);				
	m_LastFloorY = m_LastFloorY + delta;
	m_AtlasSprites[i]->SetPosition(m_LastFloorX, m_LastFloorY);
	int set_x = (int)(m_LastFloorX / m_FloorSize);
	int set_y = (int)(m_LastFloorY / m_FloorSize) + 256;
	m_Space->set(set_x, set_y, 0, i);
	
	m_AtlasSprites[i]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[i]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[i]->SetScale(1.0, 150.0);
	m_AtlasSprites[i]->m_IsAlive = false;
	m_AtlasSprites[i]->m_Frame = 0;
	m_AtlasSprites[i]->Reset();
}


void BlockBuster::RenderModelPhase() {
}


void BlockBuster::RenderSpritePhase() {
	glEnable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_ONE, GL_ONE);
	glTranslatef(-m_CameraOffsetX, 0.0, 0.0);
	RenderSpriteRange(1, 1 + m_FloorBufferCount);
	RenderSpriteRange(0, 1);
	RenderSpriteRange(1 + m_FloorBufferCount, 1 + m_FloorBufferCount + 31);
	glDisable(GL_BLEND);
}
