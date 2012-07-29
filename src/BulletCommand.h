// GPL Jon Bardin

class BulletCommand : public BulletMLRunner {

public:

  //b2Body* bullet;
  SpriteGun *bullet;
  int turn;
  int m_LastUsedBullet;

  void Shoot(AtlasSprite *, double direction, double speed, AtlasSprite *center);
  void createSimpleBullet(double direction, double speed);
	void createBullet(BulletMLState* state, double direction, double speed);

  //BulletCommand() : BulletMLRunner() {
  //}

  //BulletCommand(BulletMLParser *bp, SpriteGun *sprite);
  BulletCommand(BulletMLParser *bp, SpriteGun *b); // : BulletMLRunner(bp), bullet_(b);
  BulletCommand(BulletMLState *bs, SpriteGun *b, AtlasSprite *c);// : BulletMLRunner(bs), bullet_(b);
  ~BulletCommand();

  double getBulletDirection();
  double getAimDirection();
  double getBulletSpeed();
  double getDefaultSpeed();
  double getRank();
  int getTurn();
  void doVanish();

  void doChangeDirection(double);
  void doChangeSpeed(double);
  void doAccelX(double);
  void doAccelY(double);

  double getBulletSpeedX();
  double getBulletSpeedY();

  std::vector<BulletCommand *> m_SubBulletCommands;

  void run(int);

  int m_UseThisBullet;

  //double getRand() { return (double)rand() / RAND_MAX; }
  //BulletMLRunnerImpl* makeImpl(BulletMLState* state);

  AtlasSprite* Consume();
  
  AtlasSprite *m_FollowBullet;
};
