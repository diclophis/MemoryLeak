//
//  RaptorIsland.cpp
//  MemoryLeak
//
//  Created by Jon Bardin on 9/11/10.
//


#include "OpenSteer/SimpleVehicle.h"
#include "OpenSteer/Color.h"
#include "CaptureTheFlag.h"
#include "RaptorIsland.h"


CtfSeeker* gSeeker = NULL;
std::vector<CtfEnemy*> ctfEnemies;
extern std::vector<CtfEnemy*> ctfEnemies;
SOG CtfBase::allObstacles;


RaptorIsland::~RaptorIsland() {
	LOGV("dealloc RaptorIsland!!@!@!\n");
	myRaptors.clear();
	myRaptorManager.Release();
	
	myBarrels.clear();
	myBarrelManager.Release();
}


void RaptorIsland::build(int width, int height, std::vector<GLuint> textures, std::vector<foo*> models) {
	//Screen
	screenWidth = width;
	screenHeight = height;

	//World
	myGravity = 0.0;
	mySimulationTime = 0.0;
	myGameStarted = false;
	myGameSpeed = 1;
	myDeltaTime = 1.0 / 60.0;

	myTextures = textures;


	buildCamera();


	ctfSeeker = new CtfSeeker;
	all.push_back(ctfSeeker);


	for (int i = 0; i<10; i++) {
		CtfEnemy *enemy = new CtfEnemy;
		ctfEnemies.push_back(enemy);
		all.push_back (ctfEnemies[i]);
	}

	
	CtfBase::initializeObstacles();

	myRaptorHeight = 5.0;	
	myRaptorManager.SetStagger(4.0);
	for (unsigned int i=0; i<ctfEnemies.size(); i++) {
		myRaptors.push_back(myRaptorManager.Load(models[0], 30, myTextures[0]));
		myRaptors[i]->SetCycle(1);
		myRaptors[i]->SetPosition(-25.0, myRaptorHeight, (randf() * 50.0) - 25.0);
		myRaptors[i]->SetScale(0.2, 0.2, 0.2);
	}


	for (unsigned int cycle = 0; cycle < myRaptors[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myRaptors[0]->GetNumCycles(), cycle, myRaptors[0]->GetCycleName(cycle));
	}


	myBarrelHeight = 0.0;
	for (int i=0; i<CtfBase::obstacleCount; i++) {
		Md2Instance *barrel;
		barrel = myBarrelManager.Load(models[1], 1, myTextures[2]);
		myBarrels.push_back(barrel);
		barrel->SetScale(0.05, 0.05, 0.05);
		barrel->SetPosition(0.0, myBarrelHeight, 0.0);
		barrel->SetRotation(90.0);
	}
	 
	
	
	mySkyBoxHeight = 12.5;
	mySkyBox = mySkyBoxManager.Load(models[2], 1, myTextures[4]);
	mySkyBox->SetPosition(0.0, mySkyBoxHeight, 0.0);
	mySkyBox->SetRotation(90.0);
	mySkyBox->SetScale(0.5, 0.25, 0.5);

	
	myPlayerHeight = 0.0;
	myPlayer = myPlayerManager.Load(models[2], 1, myTextures[3]);
	myPlayer->SetPosition(0.0, myPlayerHeight, 0.0);
	myPlayer->SetRotation(90.0);
	myPlayer->SetScale(0.1, 0.1, 0.1);
	
	
	/*
	for (int cycle = 0; cycle < myBarrels[0]->GetNumCycles(); cycle++) {
		LOGV("%d %d %s\n", myBarrels[0]->GetNumCycles(), cycle, myBarrels[0]->GetCycleName(cycle));
	}
	 */
	
	
	buildFont();
	
	/*
	myPlatformCount = 1;
	int i = 0;
	
	myPlatforms = (Platform *)malloc(myPlatformCount * sizeof(Platform));

	myPlatforms[i].position = Vector3DMake(0.0, 0.0, 0.0);
	myPlatforms[i].length = 100.0;
	myPlatforms[i].amplitude = 0.0; //randf();
	myPlatforms[i].step = 100.0;
	myPlatforms[i].angular_frequency = 0.0;
	myPlatforms[i].phase = 0.0;
	
	buildSkyBox();
	*/
	
	
	

	
	
	
	
	
	
	
	
	
	
	int i=0;
	for (SOI so = CtfBase::allObstacles.begin(); so != CtfBase::allObstacles.end(); so++)
	{
		OpenSteer::Vec3 a = (**so).center;
		
		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		
		
		i++;
		
		//Ogre::Vector3 b = Vector3(a.x, a.y, a.z);
		
		/*
		if (frandom2(1.0, 2.0) >= 1.5) {
			obstacle = m_pSceneMgr->createEntity(gen->generate(), "rock.05.mesh");
			scale = Vector3(0.1, 0.1, 0.1);
		} else {
			obstacle = m_pSceneMgr->createEntity(gen->generate(), "tree.05.mesh");
			scale = Vector3(1.0, 1.0, 1.0);
		}
		 */
		
		/*
		node1 = m_pSceneMgr->getRootSceneNode()->createChildSceneNode(gen->generate());
		node1->attachObject(obstacle);
		node1->setScale(scale);
		node1->setPosition(b);
		*/
		/*
		 CONICAL = 0, ///< Conical shape
		 SPHERICAL, ///< Spherical shape
		 HEMISPHERICAL, ///< Hemispherical shape
		 CYLINDRICAL, ///< Cylindrical shape
		 TAPERED_CYLINDRICAL, ///< Tapered cylindrical shape
		 FLAME, ///< Flame shape
		 INVERSE_CONICAL, ///< Inverse conical shape
		 TEND_FLAME, ///< Tend flame shape
		 */
		
		/*
		 obstacle = m_pSceneMgr->createEntity(gen->generate(), "TreeMesh");
		 treeNode = m_pSceneMgr->getRootSceneNode()->createChildSceneNode();
		 treeNode->attachObject(obstacle);
		 treeNode->setPosition(b);
		 */
	}
	
	
	
	
	
	
	
	
	
	buildFountain();	
	
	
	
	
	
	
	myLineVertices[0] = -50.0;	
	myLineVertices[1] = 1.0;	
	myLineVertices[2] = 0.0;	

	myLineVertices[3] = 50.0;	
	myLineVertices[4] = 1.0;	
	myLineVertices[5] = 0.0;	
	
	mySimulationTime = 0.0;
		
	mySceneBuilt = true;
	
	simulate();
	go();
	
}

void RaptorIsland::render() {
	drawCamera();
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	myRaptorManager.Render();
	myBarrelManager.Render();
	//myPlayerManager.Render();
	//mySkyBoxManager.Render();

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);
	//glPointSize(2.0);
	glVertexPointer(3, GL_FLOAT, 0, myLineVertices);
	glDrawArrays(GL_LINES, 0, 2);
	glDisableClientState(GL_VERTEX_ARRAY);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	drawFountain();
	drawFont();



	glDisable(GL_TEXTURE_2D);
}


int RaptorIsland::simulate() {
	
	tickCamera();

	
	myRaptorManager.Update(myDeltaTime);
	myBarrelManager.Update(myDeltaTime);
	mySkyBoxManager.Update(myDeltaTime);
	myPlayerManager.Update(myDeltaTime);

	OpenSteer::Vec3 pos1a, vel1a, pos2a, vel2a;
	
	float rot1a;
	
	ctfSeeker->updateX(mySimulationTime, myDeltaTime, steeringFromInput);
	
	// update each enemy
	for (unsigned int i = 0; i < ctfEnemies.size(); i++) {
		ctfEnemies[i]->update(mySimulationTime, myDeltaTime);
		bool hit = false;
		pos1a = ctfEnemies[i]->position();
		vel1a = ctfEnemies[i]->velocity();
		if (vel1a.x != 0.0) {
			rot1a = atan2(vel1a.z, vel1a.x);
		}
		
		myRaptors[i]->SetRotation(-RadiansToDegrees(rot1a));
		myRaptors[i]->SetPosition(pos1a.x, myRaptorHeight, pos1a.z);
		
		if (hit) {
			myRaptors[i]->SwitchCycle(6, 0.0, true, -1, 0);
		}
	}
	
	int i=0;
	for (SOI so = CtfBase::allObstacles.begin(); so != CtfBase::allObstacles.end(); so++)
	{
		OpenSteer::Vec3 a = (**so).center;
		
		if (randf() < 0.1) {
			myFountainPosition = Vector3DMake(a.x, a.y, a.z);
		}
		
		if (a.x == -25.0) {
			if (a.z < -60.0) {
				a.z = 60.0;
			} else {
				a.z -= 0.2;
			}
		} else if (a.x == -15.0) {
			if (a.z > 60.0) {
				a.z = -60.0;
			} else {
				a.z += 0.2;
			}
		}
		
		(**so).setCenter(a);

		myBarrels[i]->SetPosition(a.x, a.y, a.z);
		i++;
	}
	
	
	pos1a = ctfSeeker->position();
	vel1a = ctfSeeker->velocity();
	myPlayer->SetPosition(pos1a.x, pos1a.y, pos1a.z);
	

	tickFountain();

	
	//if (mySimulationTime > 30.0) {
	//	return 0;
	//} else {
		return 1;
	//}
}


void RaptorIsland::buildCamera() {
	//Camera
	myCameraTarget = Vector3DMake(0.0, 0.0, 0.0);
	myCameraPosition = Vector3DMake(-10.0, 0.0, 0.0);
	myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
}

void RaptorIsland::tickCamera() {
	Vector3D desiredTarget;
	Vector3D desiredPosition;

	//desiredTarget = Vector3DMake(1000.0, 8.0, 0.0);
	desiredTarget = Vector3DMake(0.0, 0.0, 0.0);
	//Vector3D desiredPosition = Vector3DMake(-49.0, 5.0, 0.0);
#ifdef DESKTOP
	//desiredPosition = Vector3DMake(-75.0, 75.0, 0.0);
	//desiredPosition = Vector3DMake(-49.0, 8.0, 0.0);
	//desiredPosition = Vector3DMake(-11.0, 50.0, 0.0);
	desiredPosition = Vector3DMake(-49.0, 50.0 - (mySimulationTime * 10.0), 0.0);
#else
	desiredPosition = Vector3DMake(-49.0, 8.0, 0.0);
#endif
	
	myCameraTarget = desiredTarget;

	if (desiredPosition.y > 5.0) {
		myCameraPosition = desiredPosition;
	}
}

void RaptorIsland::hitTest(float x, float y) {

/*
// assuming you created your own frustum setting function with a
// similar signature to this for the camera position.
//void Set(float fFov, float fAspect, float fNear, float fFar)
// and you are familiar with the following camera settings.
//#define SCREEN_WIDTH 320
//#define SCREEN_HEIGHT 480
#define NEAR 0.1
#define FAR 200
//#define FOV SCREEN_HEIGHT
//#define ASPECT  float(SCREEN_WIDTH)/float(SCREEN_HEIGHT)
float aspect = (float)screenWidth / (float)screenHeight;


float centered_y = (screenHeight - y) - screenHeight / 2;
float centered_x = x - screenWidth / 2;
float unit_x = centered_x / (screenWidth / 2);
float unit_y = centered_y / (screenWidth / 2);

float near_height = NEAR * float(tan(screenHeight * M_PI / 360.0 ));
float ray[4] ={ unit_x * near_height * aspect, unit_y * near_height, 1, 0 };
float ray_start_point[4] = {0.f, 0.f, 0.f, 1.f};

GLfloat the_modelview[16];
//Read the current modelview matrix into the array the_modelview
glGetFloatv(GL_MODELVIEW_MATRIX, the_modelview);

M = {
{R11, R12, R13, 0},
{R21, R22, R23, 0},
{R31, R32, R33, 0},
{tx, ty, tz, 1},
};

Rt = {
{R11, R21, R31},
{R12, R22, R32},
{R13, R23, R33}
}

Rt*t = t'

M-1 = {
{R11, R21, R31, 0},
{R12, R22, R32, 0},
{R13, R23, R33, 0},
{-t'x, -t'y, -t'z, 1},
};
*/


	const Vec3 direction = directionFromCameraToScreenPosition(x, y, screenHeight);

	LOGV("ray: %f, %f, %f\n", direction.x, direction.y, direction.z);
	
	float minDistance = FLT_MAX;       // smallest distance found so far
	float d = FLT_MAX;

	//CtfBase *nearest;

	int nearestIndex = -1;

	Vec3 cameraPosition;
	cameraPosition.x = myCameraPosition.x;
	cameraPosition.y = myCameraPosition.y;
	cameraPosition.z = myCameraPosition.z;

	
    
	for (unsigned int i = 0; i < ctfEnemies.size(); i++) {
		if (myRaptors[i]->GetVisible()) {
			ctfEnemies[i]->update(mySimulationTime, myDeltaTime);

			d = distanceFromLine(ctfEnemies[i]->position(), cameraPosition, direction);

			//LOGV("wtf: %f\n", d);

			if (d < 75.0) {
				//if (myRaptors[i]->GetCycle() == 1) {
				//	myRaptors[i]->SwitchCycle(19, 0.03, false, -1, 1);
				//	break;
				//}
				myRaptors[i]->SetVisible(false);
			}
		}

		//if (d < minDistance) {
		//	minDistance = d;
		//	nearestIndex = i;
		//}
	}

	//myLineVertices[0] = 0.0 + (direction.y * 100.0);	
	//myLineVertices[1] = 5.0; //0.0 + (direction.z * 100.0);	
	//myLineVertices[2] = 0.0 + (direction.x * 100.0);

	float zzz = x - (screenWidth / 2);


	myLineVertices[0] = 200.0; //0.0 + (direction.x * 100.0);	
	myLineVertices[1] = 5.0; //0.0 + (direction.z * 100.0);	
	myLineVertices[2] = zzz;

	myLineVertices[3] = -48.0;
	myLineVertices[4] = 5.0;
	myLineVertices[5] = 0.0;

	LOGV("zzz: %f", zzz);
	LOGV("one: %f %f %f\n", myLineVertices[0], myLineVertices[1], myLineVertices[2]);
	LOGV("two: %f %f %f\n", myLineVertices[3], myLineVertices[4], myLineVertices[5]);

	//myLineVertices[3] = myCameraPosition.x + (direction.x * 10.0);	
	//myLineVertices[4] = 0.0; direction.y;	
	//myLineVertices[5] = 0.0; direction.z;	

	//LOGV("nearest: %d\n", nearestIndex);

	//if (nearestIndex != -1) {
	//	//myRaptors[nearestIndex]->SetVisible(false);
	//	myRaptors[nearestIndex]->SwitchCycle(6, 0.001, false, 1, 1);
	//}
	
	
	//d = distanceFromLine([mySeeker myVehicle]->position(), myCameraPosition, direction);
	//if (d < minDistance) {
	//	myNearest = mySeeker;
	//}
	
	//NSLog(@"nearest: %@", myNearest);

	//const AVGroup& vehicles = allVehiclesOfSelectedPlugIn();
    //for (AVIterator i = vehicles.begin(); i != vehicles.end(); i++)
    //{
        // distance from this vehicle's center to the selection line:
        //d = distanceFromLine ((**i).position(), camera.position(), direction);
		
        // if this vehicle-to-line distance is the smallest so far,
        // store it and this vehicle in the selection registers.
        //if (d < minDistance)
        //{
        //    minDistance = d;
        //    nearest = *i;
        //}
    //}
	
	//NSLog(@"%i", ((SimpleVehicle*)i)->serialNumber);

	
}

