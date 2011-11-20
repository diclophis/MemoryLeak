// Jon Bardin GPL


#include "MemoryLeak.h"
#include "SpaceShipDown.h"

#define GRAVITY -100.0
#define PART_DENSITY 5.25
#define PART_FRICTION 0.5
#define PLAYER_DENSITY 6.5
#define PLAYER_FRICTION 0.25
//#define PLAYER_HORIZONTAL_THRUST 1000.0
//#define PLAYER_VERTICAL_THRUST -GRAVITY * 5.0
#define PLAYER_MAX_VELOCITY_X 5.0
#define PLAYER_MAX_VELOCITY_Y 5.0
#define BLOCK_WIDTH 64.0

SpaceShipDown::SpaceShipDown(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s) : Engine(w, h, t, m, l, s) {
  LoadSound(0);
  m_IsPushingAudio = true;
  m_SpaceShipPartsStartIndex = -1;
  m_SpaceShipPartsStopIndex = -1;
  m_PlatformsStartIndex = -1;
  m_PlatformsStopIndex = -1;
  m_DropZonesStartIndex = -1;
  m_DropZonesStopIndex = -1;
  m_PickedUpPartIndex = -1;
  m_PickupTimeout = -1;
  m_ThrustLevel = 0.0;
  m_WorldWidth = 0.0;
  m_WorldHeight = 0.0;
  m_DebugDrawToggle = true;
  m_TouchedLeft = false;
  m_TouchedRight = false;

  CreateWorld();
  CreatePickupJoints();
  CreateDebugDraw();
  LoadLevel(1, 2);
  LoadLevel(1, 1);
  LoadLevel(1, 0);
  LoadLevel(1, 3);
  CreateBorder(m_WorldWidth, m_WorldHeight);


  float m_WorldWidthInPixels = m_WorldWidth * 2.0;
  float m_WorldHeightInPixels = m_WorldHeight * 2.0;
 

  m_LandscapeIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(2), 1, 1, 0, 1, 1.0, "", 0, 1, 0.0, m_WorldWidthInPixels, m_WorldHeightInPixels));
  m_AtlasSprites[m_LandscapeIndex]->Build(0);
  m_AtlasSprites[m_LandscapeIndex]->SetPosition(0.0, 0.0);
  m_SpriteCount++;


}


SpaceShipDown::~SpaceShipDown() {
  delete m_DebugDraw;
  delete m_ContactListener;
  delete world;
}


void SpaceShipDown::CreatePlayer(float x, float y) {
  float radius = 30.0;

  m_PlayerIndex = m_SpriteCount;
  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 3, 3, 5, 6, 1.0, "", 5, 6, 0.125, BLOCK_WIDTH, BLOCK_WIDTH));
  m_AtlasSprites[m_PlayerIndex]->m_Fps = 15.0;
  m_AtlasSprites[m_PlayerIndex]->SetPosition(x, y);
  m_AtlasSprites[m_PlayerIndex]->Build(0);
  m_SpriteCount++;

  MLPoint startPosition = MLPointMake(m_AtlasSprites[m_PlayerIndex]->m_Position[0] / PTM_RATIO, m_AtlasSprites[m_PlayerIndex]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);
  m_PlayerBody = world->CreateBody(&bd);
  m_PlayerBody->SetUserData((void *)m_PlayerIndex);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = PLAYER_DENSITY;
  fd.restitution = 0.0;
  fd.friction = PLAYER_FRICTION;
  m_PlayerBody->CreateFixture(&fd);
  m_PlayerBody->SetActive(true);
  m_FrictionJointDef->bodyB = m_PlayerBody;
}


void SpaceShipDown::CreateSpaceShipPart(float x, float y) {
  int part_index = m_SpriteCount;

  if (m_SpaceShipPartsStartIndex == -1) {
    m_SpaceShipPartsStartIndex = part_index;
  }

  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 3, 3, 7, 8, 1.0, "", 7, 8, 0.0, BLOCK_WIDTH, BLOCK_WIDTH));
  m_AtlasSprites[part_index]->Build(0);
  m_AtlasSprites[part_index]->SetPosition(x, y);
  m_SpriteCount++;
  m_SpaceShipPartsStopIndex = m_SpriteCount;

  float radius = 25.0;
  MLPoint startPosition = MLPointMake(m_AtlasSprites[part_index]->m_Position[0] / PTM_RATIO, m_AtlasSprites[part_index]->m_Position[1] / PTM_RATIO);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 1.0f;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x, startPosition.y);

  //part
  b2Body *part_body;
  part_body = world->CreateBody(&bd);
  part_body->SetUserData((void *)part_index);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = PART_DENSITY;
  fd.restitution = 0.0;
  fd.friction = PART_FRICTION;
  fd.isSensor = false;
  part_body->CreateFixture(&fd);

  //part pickup sensor
  shape.m_radius = (radius * 1.25) / PTM_RATIO;
  fd.shape = &shape;
  fd.density = 0.0;
  fd.restitution = 0.0;
  fd.friction = 0.0;
  fd.isSensor = true;
  part_body->CreateFixture(&fd);

  part_body->SetActive(true);

}


void SpaceShipDown::CreateDropZone(float x, float y, float w, float h) {
  int drop_zone_index = m_SpriteCount;

  if (m_DropZonesStartIndex == -1) {
    m_DropZonesStartIndex = drop_zone_index;
  }

  // Define the ground body.
  // bottom-left corner
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(x / PTM_RATIO, ((y - h) / PTM_RATIO));
  
  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  groundBody->SetUserData((void *)-1);
  

  // Define the ground box shape.
  b2PolygonShape groundBox;
  groundBox.SetAsBox(w / PTM_RATIO, h / PTM_RATIO);

  b2FixtureDef fd;
  fd.shape = &groundBox;
  fd.isSensor = true;
  
  groundBody->CreateFixture(&fd);

  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 3, 3, 2, 3, 1.0, "", 2, 3, 0.0, BLOCK_WIDTH, BLOCK_WIDTH));
  m_AtlasSprites[drop_zone_index]->Build(0);
  m_AtlasSprites[drop_zone_index]->SetPosition(x, y - BLOCK_WIDTH * 0.5);
  m_SpriteCount++;
  m_DropZonesStopIndex = m_SpriteCount;
}


void SpaceShipDown::CreatePlatform(float x, float y, float w, float h) {
  int platform_index = m_SpriteCount;

  if (m_PlatformsStartIndex == -1) {
    m_PlatformsStartIndex = platform_index;
  }

  // Define the ground body.
  // bottom-left corner
  b2BodyDef groundBodyDef;
  groundBodyDef.position.Set(x / PTM_RATIO, ((y - h) / PTM_RATIO));
  
  // Call the body factory which allocates memory for the ground body
  // from a pool and creates the ground box shape (also from a pool).
  // The body is also added to the world.
  b2Body* groundBody = world->CreateBody(&groundBodyDef);
  groundBody->SetActive(true);
  groundBody->SetUserData((void *)-2);
  
  // Define the ground box shape.
  b2PolygonShape groundBox;
  
  // bottom
  groundBox.SetAsBox(w / PTM_RATIO, h / PTM_RATIO);
  groundBody->CreateFixture(&groundBox, 0);

  m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(1), 3, 3, 2, 3, 1.0, "", 2, 3, 0.0, BLOCK_WIDTH, BLOCK_WIDTH));
  m_AtlasSprites[platform_index]->Build(0);
  m_AtlasSprites[platform_index]->SetPosition(x, y - BLOCK_WIDTH * 0.5);
  m_SpriteCount++;
  m_PlatformsStopIndex = m_SpriteCount;
}


void SpaceShipDown::CreateWorld() {
  b2Vec2 gravity;
  gravity.Set(0.0, GRAVITY);
  world = new b2World(gravity, false);
  m_ContactListener = new SpaceShipDownContactListener();
  world->SetContactListener(m_ContactListener);
}


void SpaceShipDown::CreateBorder(float width, float height) {
  b2BodyDef borderBodyDef;
  b2PolygonShape borderBox;
  b2Body* borderBody;

  float x = 0.0;
  float y = 0.0;
  float w = width;
  float h = height;
  float t = 10.0;
  float hs = 0.0;
  float vs = 0.0;

  x = 0;
  y = -height;
  hs = w;
  vs = t;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);
  borderBody->SetUserData((void *)-1);

  x = -width;
  y = 0;
  hs = t;
  vs = w;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);
  borderBody->SetUserData((void *)-1);

  x = 0;
  y = height;
  hs = w;
  vs = t;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);
  borderBody->SetUserData((void *)-1);

  x = width;
  y = 0;
  hs = t;
  vs = w;

  borderBodyDef.position.Set(x / PTM_RATIO, y / PTM_RATIO);
  borderBody = world->CreateBody(&borderBodyDef);
  borderBox.SetAsBox(hs / PTM_RATIO, vs / PTM_RATIO);
  borderBody->CreateFixture(&borderBox, 0);
  borderBody->SetUserData((void *)-1);
}


void SpaceShipDown::CreatePickupJoints() {
  b2RopeJointDef *rope_joint_def = new b2RopeJointDef();
  rope_joint_def->localAnchorA = b2Vec2(0.0, 0.0);
  rope_joint_def->localAnchorB = b2Vec2(0.0, 0.0);
  rope_joint_def->maxLength = 75.0 / PTM_RATIO;
  m_PickupJointDefs.push_back(rope_joint_def);

  b2DistanceJointDef *distance_joint_def = new b2DistanceJointDef();
  distance_joint_def->length = 500.0 / PTM_RATIO;
  distance_joint_def->dampingRatio = 0.0;
  m_PickupJointDefs.push_back(distance_joint_def);

  m_FrictionJointDef = new b2FrictionJointDef();
  m_FrictionJointDef->maxForce = 30.0;
  m_FrictionJointDef->maxTorque = 0.0;
}


void SpaceShipDown::Hit(float x, float y, int hitState) {
	float xx = ((x) - (0.5 * (m_ScreenWidth))) * m_Zoom;
	float yy = (0.5 * (m_ScreenHeight) - (y)) * m_Zoom;
  //LOGV("state: %d %f %f\n", hitState, x, y);
  if (hitState == 0 && x < 20.0 && y < 20.0) {
    m_DebugDrawToggle = !m_DebugDrawToggle;
  } else {
    if (hitState != 2) {
      if (xx > 0) {
        m_TouchedRight = true;
      } else {
        m_TouchedLeft = true;
      }
    } else {
      if (xx > 0) {
        m_TouchedRight = false;
      } else {
        m_TouchedLeft = false;
      }
    }
  }
}


void SpaceShipDown::AdjustZoom() {
  m_Zoom = (m_WorldWidth * 2.0) / (float)m_ScreenWidth;
}


int SpaceShipDown::Simulate() {

  AdjustZoom();

  m_PickupTimeout += m_DeltaTime;

  int velocityIterations = 32;
  int positionIterations = 32;

  world->Step(m_DeltaTime, velocityIterations, positionIterations);

  m_AtlasSprites[m_PlayerIndex]->Simulate(m_DeltaTime);

  b2Vec2 forcePosition = m_PlayerBody->GetWorldCenter();
  b2Vec2 player_velocity = m_PlayerBody->GetLinearVelocity();

  float thrust_x = 300.0;
  float thrust_y = 800.0;

  if (m_TouchedLeft) {
    //thrust_x += -PLAYER_HORIZONTAL_THRUST;
    thrust_x = -thrust_x;
  }

  if (m_TouchedLeft && m_TouchedRight) {
    //thrust_x += PLAYER_HORIZONTAL_THRUST;
    thrust_x = 0.0;
    //thrust_y *= 1.1;
  }

  if (m_TouchedLeft || m_TouchedRight) {
    thrust_y *= 2.5;
    if (m_PickedUpPartIndex != -1) {
      //thrust_x *= 2.0;
      //thrust_y = 1.0;
      thrust_y *= 2.0;
    }
    if (player_velocity.y < PLAYER_MAX_VELOCITY_Y) {
      //m_ThrustLevel += 40000.0 * thrust_y * m_DeltaTime;
    }
    m_PlayerBody->ApplyForce(b2Vec2(thrust_x, thrust_y), forcePosition);
    m_AtlasSprites[m_PlayerIndex]->Fire();
  } else {
    m_ThrustLevel = 0.0;
    if (m_PickedUpPartIndex != -1) {
      thrust_y *= 2.0;
    }
    m_PlayerBody->ApplyForce(b2Vec2(0.0, thrust_y), forcePosition);
    //m_AtlasSprites[m_PlayerIndex]->m_RenderBullets = false;
  }
  
  m_AtlasSprites[m_PlayerIndex]->m_RenderBullets = true;

  for (b2Body* b = world->GetBodyList(); b; b = b->GetNext()) {
    intptr_t body_index = (intptr_t) b->GetUserData();
    //if (body_index >= 0 || (body_index < -1)) {
      body_index = fastAbs(body_index);
      float x = b->GetPosition().x * PTM_RATIO;
      float y = b->GetPosition().y * PTM_RATIO;
      if (body_index == m_PlayerIndex) {
        if (player_velocity.x > PLAYER_MAX_VELOCITY_X) {
          player_velocity.x = PLAYER_MAX_VELOCITY_X;
        }
        if (player_velocity.x < -PLAYER_MAX_VELOCITY_X) {
          player_velocity.x = -PLAYER_MAX_VELOCITY_X;
        }
        if (player_velocity.y > PLAYER_MAX_VELOCITY_Y) {
          player_velocity.y = PLAYER_MAX_VELOCITY_Y;
        }
        if (player_velocity.y < -PLAYER_MAX_VELOCITY_Y * 2.5) {
          player_velocity.y = -PLAYER_MAX_VELOCITY_Y * 2.5;
        }
        b->SetLinearVelocity(player_velocity);
        m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[0] = fastSinf(m_SimulationTime * 100.0) * 200.0;
        m_AtlasSprites[m_PlayerIndex]->m_EmitVelocity[1] = player_velocity.y - 1000.0 ;
      }
      //m_AtlasSprites[body_index]->m_Rotation = RadiansToDegrees(b->GetAngle());
      m_AtlasSprites[body_index]->SetPosition(x, y);
    //}
  }

  std::vector<MLContact>::iterator pos;
  for (pos = m_ContactListener->m_Contacts.begin(); pos != m_ContactListener->m_Contacts.end(); ++pos) {
    MLContact contact = *pos;
    b2Body *bodyA = contact.fixtureA->GetBody();
    b2Body *bodyB = contact.fixtureB->GetBody();
    intptr_t indexA = (intptr_t)bodyA->GetUserData();
    intptr_t indexB = (intptr_t)bodyB->GetUserData();
    if (indexA == m_PlayerIndex || indexB == m_PlayerIndex) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA <= m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB <= m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex == -1) {
          if (bodyA == m_PlayerBody) {
            m_PickedUpPartIndex = indexB;
            m_FrictionJointDef->bodyA = bodyB;
          } else {
            m_PickedUpPartIndex = indexA;
            m_FrictionJointDef->bodyA = bodyA;
          }
          m_PickupTimeout = 0.0;
          b2JointDef *pickup_joint_def = m_PickupJointDefs.at(0);
          pickup_joint_def->bodyA = bodyA;
          pickup_joint_def->bodyB = bodyB;
          m_PickupJoint = (b2RopeJoint *)world->CreateJoint(pickup_joint_def);
          m_FrictionJoint = (b2FrictionJoint *)world->CreateJoint(m_FrictionJointDef);
          LOGV("player touched part\n");
        }
      }
    } else if (indexA == -1 || indexB == -1) {
      if ((indexA >= m_SpaceShipPartsStartIndex && indexA <= m_SpaceShipPartsStopIndex) || (indexB >= m_SpaceShipPartsStartIndex && indexB <= m_SpaceShipPartsStopIndex)) {
        if (m_PickedUpPartIndex != -1 && (indexA == m_PickedUpPartIndex || indexB == m_PickedUpPartIndex)) {
          //if (m_AtlasSprites[m_PlayerIndex]->m_Position[0] == m_AtlasSprites[m_]->m_Position[0]
          float x1 = bodyA->GetPosition().x;
          float x2 = bodyB->GetPosition().x;
          if ((fastAbs(x1 - x2) < 0.01) && m_PickupTimeout > 1.0) {
            b2Vec2 player_velocity;
            if (indexA == -1) {
              //player_velocity= bodyB->GetLinearVelocity();
              //player_velocity.x = 0;
              //bodyB->SetLinearVelocity(player_velocity);
              //bodyB->SetActive(false);
              bodyB->SetUserData((void *)-indexB);
            } else {
              //player_velocity= bodyA->GetLinearVelocity();
              //player_velocity.x = 0;
              bodyA->SetUserData((void *)-indexA);
              //bodyA->SetLinearVelocity(player_velocity);
              //bodyA->SetActive(false);
            }
            b2PrismaticJointDef *joint_def = new b2PrismaticJointDef();
            joint_def->bodyA = bodyA;
            joint_def->bodyB = bodyB;
            joint_def->localAxisA.Set(0.0f, 1.0f);
            //rope_joint_def->localAnchorA = b2Vec2(0.0, 0.0);
            //rope_joint_def->localAnchorB = b2Vec2(0.0, 0.0 i);
            (b2PrismaticJointDef *)world->CreateJoint(joint_def);
            m_PickedUpPartIndex = -1;
            world->DestroyJoint(m_PickupJoint);
            world->DestroyJoint(m_FrictionJoint);
            LOGV("break joint\n");
          }
        }
      }
    }
  }

  return 1;
}


const char *SpaceShipDown::byte_to_binary(int x) {
	static char b[5];
	b[0] = '\0';
	int z;
	for (z = 16; z > 0; z >>= 1) {
		strcat(b, ((x & z) == z) ? "1" : "0");
	}
	return b;
}


void SpaceShipDown::LoadLevel(int level_index, int cursor_index) {
  float limits[4];
  limits[0] = 0.0;
  limits[1] = 0.0;
  limits[2] = 0.0;
  limits[3] = 0.0;

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
      LOGV("wtf\n");
			throw 666;
		}
	}

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
			
      float world_x = current[0] * 50.0;
      float world_y = current[2] * -50.0;

      if (world_x < limits[0]) {
        limits[0] = world_x;
      }

      if (world_x > limits[2]) {
        limits[2] = world_x;
      }

      if (world_y < limits[1]) {
        limits[1] = world_y;
      }

      if (world_y > limits[3]) {
        limits[3] = world_y;
      }

      if (cursor_index == current[3]) {
        switch (current[3]) {
          case 9:
            //black
            break;

          case 8:
            //white
            break;

          case 1:
            //yellow
            //LOGV("part\n");
            CreateSpaceShipPart(world_x, world_y);
            break;
            
          case 2:
            //green
            //LOGV("player\n");
            CreatePlayer(world_x, world_y);
            break;

          case 3:
            //green
            CreateDropZone(world_x, world_y, 25.0, 25.0);
            break;

          case 0:
            //red
            //LOGV("platform\n");
            CreatePlatform(world_x, world_y, 25.0, 25.0);
            break;
            
          default:
            break;
        }
      }
		}
	}

  //LOGV("%f %f\n", limits[2] - limits[0], limits[3] - limits[1]);

  float max_width = (limits[2] - limits[0]);
  float max_height = (limits[3] - limits[1]);

  if (max_width > max_height) {
    m_WorldWidth = m_WorldHeight = max_width;
  } else {
    m_WorldWidth = m_WorldHeight = max_height;
  }

  m_WorldWidth *= 0.6;
  m_WorldHeight *= 0.6;
  m_WorldWidth += 100.0;
  m_WorldHeight += 100.0;
	
	free(level);
	free(data);
}


void SpaceShipDown::CreateDebugDraw() {
  m_DebugDraw = new GLESDebugDraw(PTM_RATIO);
  world->SetDebugDraw(m_DebugDraw);
  uint32 flags = 0;
  flags += b2Draw::e_shapeBit;
  flags += b2Draw::e_jointBit;
  //flags += b2Draw::e_aabbBit;
  flags += b2Draw::e_pairBit;
  flags += b2Draw::e_centerOfMassBit;
  m_DebugDraw->SetFlags(flags);
}


void SpaceShipDown::RenderModelPhase() {
}


void SpaceShipDown::RenderSpritePhase() {
  if (m_DebugDrawToggle) {
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    world->DrawDebugData();
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnable(GL_TEXTURE_2D);
  } else {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    RenderSpriteRange(m_LandscapeIndex, m_LandscapeIndex + 1);
    AtlasSprite::ReleaseBuffers();
    RenderSpriteRange(m_PlatformsStartIndex, m_PlatformsStopIndex);
    RenderSpriteRange(m_DropZonesStartIndex, m_DropZonesStopIndex);
    RenderSpriteRange(m_SpaceShipPartsStartIndex, m_SpaceShipPartsStopIndex);
    RenderSpriteRange(m_PlayerIndex, m_PlayerIndex + 1);
    glDisable(GL_BLEND);
    AtlasSprite::ReleaseBuffers();
  }
}
      /*
      float pl_x = m_AtlasSprites[m_PlayerIndex]->m_Position[0];
      //float pl_y = m_AtlasSprites[m_PlayerIndex]->m_Position[1];
      float pa_x = m_AtlasSprites[m_PickedUpPartIndex]->m_Position[0];
      //float pa_y = m_AtlasSprites[m_PickedUpPartIndex]->m_Position[1];
      if (m_TouchedRight) {
        if (pl_x > pa_x) {
          thrust_x *= 12;
        } else {
          thrust_x *= 0.1;
        }
      } else if (m_TouchedLeft) {
        if (pl_x < pa_x) {
          thrust_x *= 12;
        } else {
          thrust_x *= 0.1;
        }
      }
      */
