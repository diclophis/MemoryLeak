// contact listenenr for SpaceShipDown

struct MLContact {
  b2Fixture *fixtureA;
  b2Fixture *fixtureB;
  bool operator==(const MLContact& other) const {
    return (fixtureA == other.fixtureA) && (fixtureB == other.fixtureB);
  }
};

class SpaceShipDownContactListener : public b2ContactListener {

public:

  std::vector<MLContact> m_Contacts;
    
  SpaceShipDownContactListener();
  ~SpaceShipDownContactListener();
    
	virtual void BeginContact(b2Contact* contact);
	virtual void EndContact(b2Contact* contact);
	virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);    
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse);
};
