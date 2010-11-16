//


#include "MemoryLeak.h"
#include "Model.h"
#include "Engine.h"
#include "PixelPusher.h"


PixelPusher::PixelPusher(int width, int height, std::vector<GLuint> &x_textures, std::vector<foo*> &x_models) : Engine(width, height, x_textures, x_models) {
	LOGV("PixelPusher::PixelPusher\n");
	
	m_SwipeState = 0;
	
	m_Touches = (float *)malloc(sizeof(float) * 4);
	m_Touches[0] = m_Touches[1] = m_Touches[2] = m_Touches[3] = 0;

}

void PixelPusher::Build() {
	
	m_CameraRotation = 90.0;
	m_CameraRotationSpeed = 0.0;
	
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	
	m_CameraPosition[0] = 0.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;
	
	unsigned int t = 0;

	
	m_TerrainStartIndex = 0;
	m_TerrainEndIndex = m_TerrainStartIndex + 256;
	unsigned int grid = 16;
	t=0;
	for (unsigned int i=m_TerrainStartIndex; i<m_TerrainEndIndex; i++) {
		int x = (t / grid) - (grid / 2);
		int z = (t % grid) - (grid / 2); 
		m_Models.push_back(new Model(m_FooFoos.at(0)));
		m_Models[i]->SetTexture(m_Textures->at(3));
		m_Models[i]->SetFrame(0);
		m_Models[i]->m_IsPlayer = false;
		m_Models[i]->m_IsHarmfulToPlayers = false;
		m_Models[i]->m_IsStuck = true;
		m_Models[i]->SetPosition(x, -1.0, z);
		//if (x == 0.0 && z == 0.0) {
		//	m_Models[i]->SetPosition(x, -1.5, z);
		//} else {
		m_Models[i]->SetPosition(x, -1.0, z);
		//}
		
		t++;
	}
	
	m_EnemyStartIndex = m_TerrainEndIndex;
	m_EnemyEndIndex = m_EnemyStartIndex + 8;
	t=0;

	for (unsigned int i=m_EnemyStartIndex; i<m_EnemyEndIndex; i++) {
		m_Models.push_back(new Model(m_FooFoos.at(0)));
		m_Models[i]->SetTexture(m_Textures->at(0));
		m_Models[i]->SetFrame(0);
		m_Models[i]->m_IsPlayer = false;
		m_Models[i]->m_IsHarmfulToPlayers = false;
		m_Models[i]->m_IsStuck = (i % 3) == 0;
		m_Models[i]->SetPosition((t++ * 2) - 8, 0.0, 4.0);
	}

	m_PlayerIndex = m_EnemyEndIndex;
	m_Models.push_back(new Model(m_FooFoos.at(0)));
	m_Models[m_PlayerIndex]->SetTexture(m_Textures->at(1));
	m_Models[m_PlayerIndex]->SetFrame(0);
	m_Models[m_PlayerIndex]->m_IsPlayer = true;
	m_Models[m_PlayerIndex]->SetPosition(1.0, 0.0, 1.0);
	m_Models[m_PlayerIndex]->SetScale(0.98, 0.98, 0.98);
}


PixelPusher::~PixelPusher() {
	LOGV("dealloc GameController\n");
}


void PixelPusher::Hit(float x, float y, int hitState) {
	//LOGV("%f %f %d\n", x, y, hitState);
	float dx;
	float dy;
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
			
			if (fabs(y) > (m_ScreenHeight * 0.5)) {
			} else {
				//if (dx > 0) {
					//right
					m_CameraRotationSpeed += (0.0033 * dx);
				//} else {
					//left
				//	m_CameraRotation -= 0.01 ;
				//}
			}
			
			m_Touches[2] = x;
			m_Touches[3] = y;

			
			break;
			
		case 2:
			m_Touches[2] = x;
			m_Touches[3] = y;
			
			dx = m_Touches[2] - m_Touches[0];
			dy = m_Touches[3] - m_Touches[1];
			
			//LOGV("%f %f", m_Touches[2], m_Touches[3]);
			
			if (fabs(dx) > fabs(dy)) {
				if (fabs(m_Touches[3]) > (m_ScreenHeight * 0.5)) {
					if (dx > 0) {
						//right
						m_Models[m_PlayerIndex]->Move(1);
					} else {
						//left
						m_Models[m_PlayerIndex]->Move(3);
					}
				} else {
					if (dx > 0) {
						//right
						//m_CameraRotation += 1;
					} else {
						//left
						//m_CameraRotation -= 1;
					}
				}
			} else {
				if (dy > 0) {
					//up
					m_Models[m_PlayerIndex]->Move(0);

				} else {
					//down
					m_Models[m_PlayerIndex]->Move(2);
				}
			}

				
			
			break;
			
		default:
			break;
	}
}


int PixelPusher::Simulate() {

	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];

	for (unsigned int i=m_EnemyStartIndex; i<m_Models.size(); i++) {
    bool collided = false;

		//LOGV("%d %f %f %f\n", i, m_Models[i]->m_Position[0], m_Models[i]->m_Position[1], m_Models[i]->m_Position[2]);
    if (m_Models[i]->m_IsStuck) {
      //no point
    } else {
      m_Models[i]->Simulate(m_DeltaTime);
      for (unsigned int j=0; j<m_Models.size(); j++) {
        if (i != j) {
          if (m_Models[i]->IsCollidedWith(m_Models[j])) {
            collided = true;
            if (m_Models[i]->m_IsPlayer && m_Models[j]->m_IsHarmfulToPlayers) {
              m_Models[j]->Harm(m_Models[i]);
            } else if (m_Models[j]->IsMovable()) {
              //m_Models[j]->SetVelocity(m_Models[i]->m_Velocity[0] + m_Models[j]->m_Position[0], m_Models[i]->m_Velocity[1] +  + m_Models[j]->m_Position[0], m_Models[i]->m_Velocity[2]);
				//if (m_Models[i]->m_IsPlayer) {
					m_Models[j]->Move(m_Models[i]->m_Direction);
				//}
			} else if (m_Models[j]->IsClimbable(m_Models[i])) {
              m_Models[i]->Climb(m_Models[j]);
            } else {
              //m_Models[i]->SetVelocity(0.0, 0.0, 0.0);
              m_Models[i]->Stand();
            }
          } else {
            //m_Models[i]->SetVelocity(m_Models[i]->m_Velocity[0], -1.0, m_Models[i]->m_Velocity[2]);
          }
        }
      }
      if (!collided) {
        m_Models[i]->Fall();
        //m_Models[i]->SetVelocity(0.0, -1.0, 0.0);
      }
    }
	}
	
	
	
	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];
	
	m_CameraRotation = m_CameraRotation + (m_CameraRotationSpeed * m_DeltaTime);
	float m_CameraDiameter = 20.0;
	float x = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
	float z = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];

	if (m_CameraRotationSpeed > 0.1) {
		m_CameraRotationSpeed -= (0.05 * m_DeltaTime);
	} else if (m_CameraRotationSpeed < -0.1) {
		m_CameraRotationSpeed += (0.05 * m_DeltaTime);
	} else {
		m_CameraRotationSpeed = 0.0;
	}

	m_CameraPosition[0] = x;
	m_CameraPosition[1] = m_CameraTarget[1] + 15.0;
	m_CameraPosition[2] = z;
	
	
	return 1;
}
