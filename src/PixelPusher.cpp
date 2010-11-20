//

#include "MemoryLeak.h"
#include "Model.h"
#include "Engine.h"
#include "PixelPusher.h"


PixelPusher::PixelPusher(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l) : Engine(w, h, t, m, l) {
	LOGV("PixelPusher::PixelPusher\n");
	m_Touches = (float *)malloc(sizeof(float) * 4);
	m_Touches[0] = m_Touches[1] = m_Touches[2] = m_Touches[3] = 0;
}


void PixelPusher::Build() {
	m_CameraRotation = 0.0;
	m_CameraRotationSpeed = 0.0;
  m_CameraHeight = 0.0;
  m_CameraClimbSpeed = 0.0;
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	m_CameraPosition[0] = 0.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;
  Load(0);
}


PixelPusher::~PixelPusher() {
	LOGV("dealloc GameController\n");
}


void PixelPusher::Hit(float x, float y, int hitState) {
	float dx;
	float dy;
	int d1 = 0;
	int d2 = 0;
	int d3 = 0;
	int d4 = 0;
	int r = abs((int)RadiansToDegrees(m_CameraRotation) % 360);
	
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
			if (fabs(y) > (m_ScreenHeight * 0.25)) {
			} else {
        if (fabs(dx) > fabs(dy)) {
          if (dx > 0.0) {
            m_CameraRotationSpeed += 1;
          } else {
            m_CameraRotationSpeed += -1;
          }
        } else {
          if (dy > 0.0) {
            m_CameraClimbSpeed += 1;
          } else {
            m_CameraClimbSpeed += -1;
          }
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
			if (r >= 0 && r <= 45) {
				d1 = 3;
				d2 = 1;
				d3 = 2;
				d4 = 0;
			} else if (r >= 45 && r <= 135) {
				d1 = 2;
				d2 = 0;
				d3 = 1;
				d4 = 3;
			} else if (r >= 135 && r <= 225) {
				d1 = 1;
				d2 = 3;
				d3 = 0;
				d4 = 2;
			} else if (r >= 225 && r <= 315) {
				d2 = 2;
				d3 = 3;
				d4 = 1;
			} else if (r >= 225 && r <= 360) {
				d1 = 3;
				d2 = 1;
				d3 = 2;
				d4 = 0;
			} else {
				LOGV("the fuck\n");
			}
			if (fabs(m_Touches[3]) > (m_ScreenHeight * 0.25)) {
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

	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];

	for (unsigned int i=0; i<m_Models.size(); i++) {
    bool collided = false;
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
            } else if (m_Models[j]->IsMovable() && m_Models[i]->m_Position[1] <= m_Models[j]->m_Position[1]) {
			        m_Models[j]->Move(m_Models[i]->m_Direction);
            } else {
              m_Models[i]->Stand();
            }
          }
        }
      }
      if (!collided) {
        m_Models[i]->Fall();
      }
    }
	}
	
	m_CameraTarget[0] = m_Models[m_PlayerIndex]->m_Position[0];
	m_CameraTarget[1] = m_Models[m_PlayerIndex]->m_Position[1];
	m_CameraTarget[2] = m_Models[m_PlayerIndex]->m_Position[2];
	
	m_CameraRotation = m_CameraRotation + (m_CameraRotationSpeed * m_DeltaTime);
  m_CameraHeight = m_CameraHeight + (m_CameraClimbSpeed * m_DeltaTime);

	float m_CameraDiameter = 42.0;
	float x = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
	float z = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];

	if (m_CameraRotationSpeed > 0.1) {
		m_CameraRotationSpeed -= (1.0 * m_DeltaTime);
	} else if (m_CameraRotationSpeed < -0.1) {
		m_CameraRotationSpeed += (1.0 * m_DeltaTime);
	} else {
		m_CameraRotationSpeed = 0.0;
	}

	if (m_CameraClimbSpeed > 0.1) {
		m_CameraClimbSpeed -= (1.0 * m_DeltaTime);
	} else if (m_CameraClimbSpeed < -0.1) {
		m_CameraClimbSpeed += (1.0 * m_DeltaTime);
	} else {
		m_CameraClimbSpeed = 0.0;
	}

	m_CameraPosition[0] = x;
	m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
	m_CameraPosition[2] = z;
	
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
  fread(level, sizeof(char), m_LevelFoos->at(level_index)->len, m_LevelFoos->at(level_index)->fp);
	
  unsigned int i = 0;
  unsigned int l = strlen(level);
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
          m_Models[m_TerrainEndIndex]->SetScale(0.98, 0.98, 0.98);
          break;

        default:
          break;
      }

      m_Models[m_TerrainEndIndex]->SetPosition(current[0], current[1] - 1, current[2]);
      m_Models[m_TerrainEndIndex]->SetTexture(m_Textures->at(t));
      m_Models[m_TerrainEndIndex]->SetFrame(0);
    }
    m_TerrainEndIndex++;
  }
  free(data);
}
