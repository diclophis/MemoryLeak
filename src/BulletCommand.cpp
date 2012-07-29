// GPL Jon Bardin

#include "MemoryLeak.h"

//BulletCommand::BulletCommand(BulletMLParser* bp, SpriteGun *b) : BulletMLRunner(bp) {
//  LOGV("BulletCommand::BulletCommand just parser, with sprite\n");
//  turn = 0;
//}

BulletCommand::BulletCommand(BulletMLParser* bp, SpriteGun* b) : BulletMLRunner(bp), bullet(b) {
  LOGV("BulletCommand::BulletCommand\n");
  turn = 0;
  m_LastUsedBullet = -1;
  m_UseThisBullet = -1;
  m_FollowBullet = NULL;
}


BulletCommand::BulletCommand(BulletMLState* bs, SpriteGun* b) : BulletMLRunner(bs), bullet(b) {
  LOGV("BulletCommand::BulletCommand with state\n");
  turn = 0;
  m_LastUsedBullet = -1;
  m_UseThisBullet = -1;
  m_FollowBullet = NULL;
}


BulletCommand::~BulletCommand() {
  LOGV("dealloc bullet\n");
}

void BulletCommand::createSimpleBullet(double direction, double speed) {
  Shoot(direction, speed);
}


void BulletCommand::createBullet(BulletMLState* state, double direction, double speed) {
  LOGV("createBullet with state\n");
  BulletCommand *bc = new BulletCommand(state, bullet);
  m_SubBulletCommands.push_back(bc);
  bc->m_FollowBullet = bullet->m_AtlasSprites[bullet->m_LastUsedBullet];
  bc->Shoot(direction, speed);
}

void BulletCommand::Consume() {
  bullet->m_LastUsedBullet++;

  if (bullet->m_LastUsedBullet >= bullet->m_AtlasSprites.size()) {
    bullet->m_LastUsedBullet = 0;
  }

  //for (std::vector<BulletCommand *>::iterator i = m_SubBulletCommands.begin(); i != m_SubBulletCommands.end(); ++i) {
  //  (*i)->m_LastUsedBullet = m_LastUsedBullet;
  //}
}

void BulletCommand::Shoot(double direction, double speed) {
  
  /*
  int i = bullet->m_LastUsedBullet;

  if (m_UseThisBullet != -1) {
    i = m_UseThisBullet;
  } else {
    i = m_LastUsedBullet;
  }
  */

  //LOGV("Shoot %x  --  %d %d %d  --  %f %f\n", this, m_LastUsedBullet, m_UseThisBullet, bullet->m_LastUsedBullet, direction, speed);

  AtlasSprite *sprite = bullet->m_AtlasSprites[bullet->m_LastUsedBullet];
  b2Body *body = (b2Body *)sprite->m_UserData;

  sprite->m_Scale[0] = 10.0;
  sprite->m_Scale[1] = 10.0;
  body->SetAwake(false);
  if (m_FollowBullet) {
    //LOGV("following\n");
    body->SetTransform(b2Vec2(m_FollowBullet->m_Position[0] / PTM_RATIO, m_FollowBullet->m_Position[1] / PTM_RATIO), 0.0);
  } else {
    body->SetTransform(b2Vec2(bullet->m_Position[0] / PTM_RATIO, bullet->m_Position[1] / PTM_RATIO), 0.0);
  }
  // x = r cos theta,
  // y = r sin theta, 
  float fx = speed * fastSinf((M_PI / 2.0) - DEGREES_TO_RADIANS(direction));
  float fy = speed * fastSinf(DEGREES_TO_RADIANS(direction));
  body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
  sprite->m_IsAlive = true;
  sprite->m_Life = 0.0;
  sprite->m_Frame = 0;

  Consume();

}

double BulletCommand::getBulletDirection() {
  LOGV("getBulletDirection\n");
  return 0.0;
}


double BulletCommand::getAimDirection() {
  //LOGV("getAimDirection\n");
  return -90.0;
}


double BulletCommand::getBulletSpeed() {
  LOGV("getBulletSpeed\n");
  return 10.0;
}


double BulletCommand::getDefaultSpeed() {
  //LOGV("getDefaultSpeed\n");
  return 2.0;
}


double BulletCommand::getRank() {
  //LOGV("getRank\n");
  return 0.5;
}


int BulletCommand::getTurn() {
  //LOGV("getTurn %x %d\n", this, turn);
  return turn;
}


void BulletCommand::doVanish() {
  LOGV("doVanish %d\n", bullet->m_LastUsedBullet);
  if (m_FollowBullet) {
    m_FollowBullet->m_IsAlive = false;
  }
}

void BulletCommand::doChangeDirection(double direction) {
  LOGV("do change direction %f\n", direction);
  float speed = getDefaultSpeed();
  float fx = speed * fastSinf((M_PI / 2.0) - DEGREES_TO_RADIANS(direction));
  float fy = speed * fastSinf(DEGREES_TO_RADIANS(direction));
  if (m_FollowBullet) {
    AtlasSprite *sprite = m_FollowBullet; //bullet->m_AtlasSprites[m_LastUsedBullet - 1];
    b2Body *body = (b2Body *)sprite->m_UserData;
    body->SetAwake(false);
    body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
  }
}


void BulletCommand::doChangeSpeed(double) {
  LOGV("doChangeSpeed\n");
}


void BulletCommand::doAccelX(double) {
  LOGV("doAccelX\n");
}


void BulletCommand::doAccelY(double) {
  LOGV("doAccelY\n");
}


double BulletCommand::getBulletSpeedX() {
  LOGV("getBulletSpeedX\n");
  return 0.0;
}


double BulletCommand::getBulletSpeedY() {
  LOGV("getBulletSpeedY\n");
  return 0.0;
}

void BulletCommand::run(int t) {
  turn = t;
  BulletMLRunner::run();
  for (std::vector<BulletCommand *>::iterator i = m_SubBulletCommands.begin(); i != m_SubBulletCommands.end(); ++i) {
    (*i)->run(t);
  }
}
