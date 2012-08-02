// GPL Jon Bardin

#include "MemoryLeak.h"
#include "BulletCommandPlayer.h"

//BulletCommandPlayer::BulletCommandPlayer(BulletMLParser* bp, SpriteGun *b) : BulletMLRunner(bp) {
//  LOGV("BulletCommandPlayer::BulletCommandPlayer just parser, with sprite\n");
//  turn = 0;
//}

BulletCommandPlayer::BulletCommandPlayer(BulletMLParser* bp, SpriteGun* b) : BulletMLRunner(bp), bullet(b) {
  //LOGV("BulletCommandPlayer::BulletCommandPlayer MAINNNN %x bullet=%x\n", this, bullet);
  turn = 0;
  m_LastUsedBullet = -1;
  m_UseThisBullet = -1;
  m_FollowBullet = NULL;
}


BulletCommandPlayer::BulletCommandPlayer(BulletMLState* bs, SpriteGun* b, AtlasSprite *c) : BulletMLRunner(bs), bullet(b), m_FollowBullet(c) {
  //LOGV("BulletCommandPlayer::BulletCommandPlayer with state SUB: %x bullet = %x != m_Follow = %x\n", this, bullet, m_FollowBullet);
  turn = 0;
  m_LastUsedBullet = -1;
  m_UseThisBullet = -1;
}


BulletCommandPlayer::~BulletCommandPlayer() {
  LOGV("dealloc bullet\n");
}


void BulletCommandPlayer::createSimpleBullet(double direction, double speed) {
  //LOGV("createSimple MAIN==?? %x with state gonna center on follow? %x\n", this, m_FollowBullet);
  AtlasSprite *going_to_be_shot = Consume();
  if (m_FollowBullet) {
    Shoot(going_to_be_shot, direction, speed, m_FollowBullet);
  } else {
    Shoot(going_to_be_shot, direction, speed, bullet);
  }
}


void BulletCommandPlayer::createBullet(BulletMLState* state, double direction, double speed) {
  //LOGV("createBullet MAIN==?? %x with state\n", this);
  AtlasSprite *going_to_be_shot = Consume();
  BulletCommandPlayer *bc = new BulletCommandPlayer(state, bullet, going_to_be_shot);
  m_SubBulletCommandPlayers.push_back(bc);
  bc->Shoot(going_to_be_shot, direction, speed, bullet);
}

AtlasSprite *BulletCommandPlayer::Consume() {
  int r = bullet->m_LastUsedBullet;

  bullet->m_LastUsedBullet++;

  if (bullet->m_LastUsedBullet >= bullet->m_AtlasSprites.size()) {
    bullet->m_LastUsedBullet = 0;
  }

  return bullet->m_AtlasSprites[r];
}

void BulletCommandPlayer::Shoot(AtlasSprite *sprite, double direction, double speed, AtlasSprite *center) {
  
  b2Body *body = (b2Body *)sprite->m_UserData;

  sprite->m_Scale[0] = 10.0;
  sprite->m_Scale[1] = 10.0;

  body->SetAwake(false);
  body->SetTransform(b2Vec2(center->m_Position[0] / PTM_RATIO, center->m_Position[1] / PTM_RATIO), 0.0);

  // x = r cos theta,
  // y = r sin theta, 
  float fx = speed * fastSinf((M_PI / 2.0) - DEGREES_TO_RADIANS(direction));
  float fy = speed * fastSinf(DEGREES_TO_RADIANS(direction));
  body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
  sprite->m_IsAlive = true;
  sprite->m_Life = 0.0;
  sprite->m_Frame = 0;
}

double BulletCommandPlayer::getBulletDirection() {
  LOGV("getBulletDirection\n");
  return 0.0;
}


double BulletCommandPlayer::getAimDirection() {
  //LOGV("getAimDirection\n");
  return -90.0;
}


double BulletCommandPlayer::getBulletSpeed() {
  LOGV("getBulletSpeed\n");
  return 10.0;
}


double BulletCommandPlayer::getDefaultSpeed() {
  //LOGV("getDefaultSpeed\n");
  return 2.0;
}


double BulletCommandPlayer::getRank() {
  //LOGV("getRank\n");
  return 0.5;
}


int BulletCommandPlayer::getTurn() {
  //LOGV("getTurn %x %d\n", this, turn);
  return turn;
}


void BulletCommandPlayer::doVanish() {
  //LOGV("doVanish %d\n", bullet->m_LastUsedBullet);
  if (m_FollowBullet) {
    m_FollowBullet->m_IsAlive = false;
  }
}

void BulletCommandPlayer::doChangeDirection(double direction) {
  LOGV("do change direction %f\n", direction);
  float speed = getDefaultSpeed();
  float fx = speed * fastSinf((M_PI / 2.0) - DEGREES_TO_RADIANS(direction));
  float fy = speed * fastSinf(DEGREES_TO_RADIANS(direction));
  //TODO this is wrong
  if (m_FollowBullet) {
    AtlasSprite *sprite = m_FollowBullet; //bullet->m_AtlasSprites[m_LastUsedBullet - 1];
    b2Body *body = (b2Body *)sprite->m_UserData;
    body->SetAwake(false);
    body->ApplyLinearImpulse(b2Vec2(fx, fy), body->GetPosition());
  }
}


void BulletCommandPlayer::doChangeSpeed(double vel) {
  //LOGV("doChangeSpeed\n");
  if (m_FollowBullet) {
    m_FollowBullet->m_IsAlive = false;
    b2Body *body = (b2Body *)m_FollowBullet->m_UserData;
    //body->SetLinearVelocity(vel);
    //body->SetTransform(b2Vec2(bullet->m_Position[0] / PTM_RATIO, bullet->m_Position[1] / PTM_RATIO), 0.0);
    //m_FollowBullet->m_Position[0] = bullet->m_Position[0];
    //m_FollowBullet->m_Position[1] = bullet->m_Position[1];
    //body->SetLinearVelocity( vel );
  }
}


void BulletCommandPlayer::doAccelX(double) {
  LOGV("doAccelX\n");
}


void BulletCommandPlayer::doAccelY(double) {
  LOGV("doAccelY\n");
}


double BulletCommandPlayer::getBulletSpeedX() {
  //LOGV("getBulletSpeedX\n");
  return 0.0;
}


double BulletCommandPlayer::getBulletSpeedY() {
  //LOGV("getBulletSpeedY\n");
  return 0.0;
}

void BulletCommandPlayer::run(int t) {
  turn = t;
  BulletMLRunner::run();
  for (std::vector<BulletCommandPlayer *>::iterator i = m_SubBulletCommandPlayers.begin(); i != m_SubBulletCommandPlayers.end(); ++i) {
    (*i)->run(t);
  }
}
