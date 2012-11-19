// SpaceShipDownContactListener - Jon Bardin GPL

#include "MemoryLeak.h"
#include "SpaceShipDownContactListener.h"

SpaceShipDownContactListener::SpaceShipDownContactListener() : m_Contacts() {
}

SpaceShipDownContactListener::~SpaceShipDownContactListener() {
}

void SpaceShipDownContactListener::BeginContact(b2Contact* contact) {
    // We need to copy out the data because the b2Contact passed in is reused.
    MLContact ml_contact = { contact->GetFixtureA(), contact->GetFixtureB() };
    m_Contacts.push_back(ml_contact);
}

void SpaceShipDownContactListener::EndContact(b2Contact* contact) {
    MLContact ml_contact = { contact->GetFixtureA(), contact->GetFixtureB() };
    std::vector<MLContact>::iterator pos;
    pos = std::find(m_Contacts.begin(), m_Contacts.end(), ml_contact);
    if (pos != m_Contacts.end()) {
        m_Contacts.erase(pos);
    }
}

void SpaceShipDownContactListener::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) {
}

void SpaceShipDownContactListener::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {
}

