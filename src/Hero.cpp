// Jon Bardin GPL


#include "MemoryLeak.h"


Hero::Hero(b2World *w, GLuint t) {

  sprite = new SpriteGun(t, 8, 8, 59, 60, 1.0, "", 8, 11, 1.0, 75.0, 75.0);
  sprite->m_IsAlive = false;
  sprite->Build(0);
        
  world = w;
  radius = 14.0f;
  awake = false;

  rotation = 0;

  CreateBox2DBody();
  UpdateNodePosition();
  Sleep();

}


Hero::~Hero() {
}


void Hero::Sleep() {
  awake = false;
  body->SetActive(false);
}


void Hero::Wake() {
  awake = true;
  body->SetActive(true);
  body->ApplyLinearImpulse(b2Vec2(3, 3), body->GetPosition());
}


void Hero::Dive() {
  body->ApplyForce(b2Vec2(0,-40),body->GetPosition());
}


void Hero::LimitVelocity() {
  const float minVelocityX = 5;
  const float minVelocityY = -20;
  b2Vec2 vel = body->GetLinearVelocity();
  if (vel.x < minVelocityX) {
    vel.x = minVelocityX;
  }
  if (vel.y < minVelocityY) {
    vel.y = minVelocityY;
  }
  body->SetLinearVelocity(vel);
}


void Hero::UpdateNodePosition() {

  float x = body->GetPosition().x * PTM_RATIO;
  float y = body->GetPosition().y * PTM_RATIO;

  position = MLPointMake(x, y);
  b2Vec2 vel = body->GetLinearVelocity();
  float angle = atan2f(vel.y, vel.x);

//#ifdef DRAW_BOX2D_WORLD
//    body->SetTransform(body->GetPosition(), angle);
//#else
    
  rotation = -1 * RadiansToDegrees(angle);

  if (y < -radius && awake) {
    Sleep();
  }

  sprite->SetPosition(position.x, position.y);
}


void Hero::Reset() {
  world->DestroyBody(body);
  CreateBox2DBody();
  Sleep();
}


void Hero::CreateBox2DBody() {
  MLPoint startPosition = MLPointMake(0, 128);
  b2BodyDef bd;
  bd.type = b2_dynamicBody;
  bd.linearDamping = 0.0f;
  bd.fixedRotation = true;
  bd.position.Set(startPosition.x / PTM_RATIO, startPosition.y / PTM_RATIO);
  body = world->CreateBody(&bd);
  b2CircleShape shape;
  shape.m_radius = radius / PTM_RATIO;
  b2FixtureDef fd;
  fd.shape = &shape;
  fd.density = 1.0f;
  fd.restitution = 0.0; //bounce
  fd.friction = 0.0;
  body->CreateFixture(&fd);
}


void Hero::Render() {
  sprite->Render();
}
