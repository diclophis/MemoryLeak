

int CtfBase::obstacleCount = 0; // this value means "uninitialized"

const OpenSteer::Vec3 gHomeBaseCenter (-50.0, 0, 0.0);
const float gHomeBaseRadius = 10.0;

//const int CtfBase::maxObstacleCount = 10;
const float gObstacleRadius = 4.0;
const float gMinStartRadius = 50;
const float gMaxStartRadius = 100;

const float gBrakingRate = 0.75;

//const Color evadeColor     (0.6f, 0.6f, 0.3f); // annotation
//const Color seekColor      (0.3f, 0.6f, 0.6f); // annotation
//const Color clearPathColor (0.3f, 0.6f, 0.3f); // annotation

const float gAvoidancePredictTimeMin  = 0.5f;
const float gAvoidancePredictTimeMax  = 1.0;
float gAvoidancePredictTime = gAvoidancePredictTimeMin;

//bool enableAttackSeek  = true; // for testing (perhaps retain for UI control?)
//bool enableAttackEvade = true; // for testing (perhaps retain for UI control?)

CtfSeeker* gSeeker = NULL;

std::vector<CtfEnemy*> ctfEnemies;

#define testOneObstacleOverlap(radius, center)               \
{                                                            \
float d = Vec3::distance (c, center);                    \
float clearance = d - (r + (radius));                    \
if (minClearance > clearance) minClearance = clearance;  \
}

