
// Jon Bardin GPL


class Hero {


public:
	Hero(b2World *w, GLuint t);
	~Hero();

  SpriteGun *sprite;
  b2World *world;
  b2Body *body;
  float radius;
  bool awake;

  void Sleep();
  void Wake();
  void Dive();
  void LimitVelocity();
  void UpdateNodePosition();
  void Reset();
  void CreateBox2DBody();

  float rotation;
  MLPoint position;

  void Render();

};
