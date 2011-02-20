// Jon Bardin GPL

#include "MemoryLeak.h"
#include "octree.h"
#include "micropather.h"
#include "AtlasSprite.h"
#include "SpriteGun.h"
#include "Model.h"
#include "ModelOctree.h"
#include "Engine.h"

#include "MainMenu.h"

#define kMaxTankSpeed 1.0
#define kTurnRate 1.0
#define kTankAcceleration 0.25


#define WATER_VERTICES      4
#define RIPPLE_SEGMENTS     64

/** Vertices for the water object. */
static const GLfloat objVertexDataWater[]=
{
-10000.f, 0.f, -10000.f,
10000.f, 0.f, -10000.f,
10000.f, 0.f,  10000.f,
-10000.f, 0.f,  10000.f
};

/** Indices to the water object vertices. */
static const GLubyte objIndicesWater[]=
{ 1, 0, 2, 3 };

/** Materials for the duck object. */
static const GLfloat objDiffuseDuck[4]     = { 0.6, 0.6, 0.10, 1.0 };
static const GLfloat objAmbientDuck[4]     = { 0.5, 0.4, 0.05, 1.0 };
static const GLfloat objSpecularDuck[4]    = { 0.8, 0.8, 0.20, 1.0 };
static const GLfloat objEmissionDuck[4]    = { 0.2, 0.2, 0.00, 1.0 };

/** Materials for the underwater duck object. */
static const GLfloat objDiffuseSunkenDuck[4]     = { 0.4, 0.4, 0.60, 1.0 };
static const GLfloat objAmbientSunkenDuck[4]     = { 0.4, 0.3, 0.65, 1.0 };
static const GLfloat objSpecularSunkenDuck[4]    = { 0.6, 0.6, 0.80, 1.0 };
static const GLfloat objEmissionSunkenDuck[4]    = { 0.0, 0.0, 0.20, 1.0 };


static const GLfloat lightPositionLamp[4]  = { 0.0, 50.0, 50.0, 0.0 };


MainMenu::MainMenu(int w, int h, std::vector<GLuint> &t, std::vector<foo*> &m, std::vector<foo*> &l, std::vector<foo*> &s, int bs, int sd) : Engine(w, h, t, m, l, s, bs, sd) {

	m_CameraIndex = 0;
	
	leftSliderValue = 0.0;
	rightSliderValue = 0.0;
	
	m_CameraRotation = -10.0;
	m_CameraRotationSpeed = 0.0;
	m_CameraHeight = 10.0;
	m_CameraClimbSpeed = 0.0;
	m_CameraTarget[0] = 0.0;
	m_CameraTarget[1] = 0.0;
	m_CameraTarget[2] = 0.0;
	m_CameraPosition[0] = 0.0;
	m_CameraPosition[1] = 0.0;
	m_CameraPosition[2] = 0.0;

	int m_PostProcessFlags = aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph | aiProcess_ImproveCacheLocality;
	char path[128];
	snprintf(path, sizeof(s), "%d", 2);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();	
	
	snprintf(path, sizeof(s), "%d", 1);
	m_Importer.ReadFile(path, m_PostProcessFlags);	
	m_FooFoos.push_back(Model::GetFoo(m_Importer.GetScene(), 0, 1));
	m_Importer.FreeScene();	
	
	m_Models.push_back(new Model(m_FooFoos.at(0)));
	m_Models[0]->SetTexture(m_Textures->at(2));
	m_Models[0]->SetFrame(0);
	m_Models[0]->SetPosition(0.0, 1.0, 0.0);
	m_Models[0]->SetScale(4.0, 4.0, 4.0);

	m_Models.push_back(new Model(m_FooFoos.at(1)));
	m_Models[1]->SetTexture(m_Textures->at(1));
	m_Models[1]->SetFrame(0);
	m_Models[1]->SetPosition(0.0, -0.75, 0.0);
	m_Models[1]->SetScale(256.0, 0.25, 256.0);
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 4, 4, "", 0, 16, 1.25, "", 0, 16, 0.25));
	m_AtlasSprites[0]->SetPosition(0.0 - (0.4 * m_ScreenWidth), 0.0);
	m_AtlasSprites[0]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[0]->m_IsAlive = true;
	m_AtlasSprites[0]->m_IsReady = true;
	m_AtlasSprites[0]->SetEmitVelocity(0.0, 0.0);
	m_AtlasSprites[0]->Build(10);
	
	m_AtlasSprites.push_back(new SpriteGun(m_Textures->at(0), 4, 4, "", 0, 16, 1.25, "", 0, 16, 2.25));
	m_AtlasSprites[1]->SetPosition(0.0 + (0.4 * m_ScreenWidth), 0.0);
	m_AtlasSprites[1]->SetVelocity(0.0, 0.0);
	m_AtlasSprites[1]->m_IsAlive = false;
	m_AtlasSprites[1]->m_IsReady = true;
	m_AtlasSprites[1]->Build(0);
	
	BuildParticles(10);
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
iRippleVertices = new GLfloat[(RIPPLE_SEGMENTS * 2) * 3];
iRippleIndices = new GLushort[RIPPLE_SEGMENTS * 2 + 2];

	
// Calculate the ripple triangle strip indices now, since they won't change.
// The ripples are drawn as a triangle strip, which goes around a full circle.
for (GLint i = 0; i < RIPPLE_SEGMENTS * 2 + 2; i++)
{
iRippleIndices[i] = i % (RIPPLE_SEGMENTS * 2);
}

// Create the needed vertex buffers object names
//glGenBuffers( 1, &iVertexDataBufferId );
//glGenBuffers( 1, &iNormalDataBufferId );
//glGenBuffers( 1, &iIndexDataBufferId );

/*
// Create the vertex buffer object for duck vertices
glBindBuffer( GL_ARRAY_BUFFER, iVertexDataBufferId );
glBufferData( GL_ARRAY_BUFFER,
				   ( sizeof(GLfloat) * NUM_DUCK_VERTICES * 3 ),  // Size of the data array
				   objVertexdataDuck,      // Vertices
				   GL_STATIC_DRAW );           // Hint that the object data is static (non-changing)

// Create the vertex buffer object for duck normals
glBindBuffer( GL_ARRAY_BUFFER, iNormalDataBufferId );
glBufferData( GL_ARRAY_BUFFER,
				   ( sizeof(GLbyte) * NUM_DUCK_VERTICES * 3 ),// Size of the data array
				   objNormaldataDuck,    // Normals
				   GL_STATIC_DRAW );             // Hint that the object data is static (non-changing)

// Create the vertex buffer object for duck indices
glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, iIndexDataBufferId );
glBufferData( GL_ELEMENT_ARRAY_BUFFER,
				   ( sizeof(GLubyte) * NUM_DUCK_FACES * 3 ), // Size of the index array
				   objFacedataDuck,              // No data pointer given (creates empty buffer)
				   GL_STATIC_DRAW );             // Hint that the object data is static (non-changing)
*/






	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
}

MainMenu::~MainMenu() {
}

void MainMenu::Hit(float x, float y, int hitState) {
	float xx = x - (0.5 * m_ScreenWidth);
	float yy = 0.5 * m_ScreenHeight - y;

	float dpx;
	float dpy;
	
	if (y < (0.15 * m_ScreenHeight)) {
		if (hitState == 2) {
			m_CameraIndex++;
			if (m_CameraIndex > 3) {
				m_CameraIndex = 0;
			}
		}
	} else {
		if (xx < 0) {
			dpx = m_AtlasSprites[0]->m_Position[0] - xx;
			dpy = m_AtlasSprites[0]->m_Position[1] - yy;
			xx = m_AtlasSprites[0]->m_Position[0] = -
			(0.4 * m_ScreenWidth);
			yy = m_AtlasSprites[0]->m_Position[1] - dpy;
			m_AtlasSprites[0]->SetPosition(xx, yy);
			rightSliderValue = yy;
		} else {
			dpx = m_AtlasSprites[1]->m_Position[0] - xx;
			dpy = m_AtlasSprites[1]->m_Position[1] - yy;
			xx = m_AtlasSprites[1]->m_Position[0] = (0.4 * m_ScreenWidth);
			yy = m_AtlasSprites[1]->m_Position[1] - dpy;
			m_AtlasSprites[1]->SetPosition(xx, yy);
			leftSliderValue = yy;
		}
	}
}

void MainMenu::Build() {
  m_IsPushingAudio = true;
}

int MainMenu::Simulate() {
	
	
	

	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	int shot_this_tick = 0;
	int not_shot_this_tick = 0;
	float m_ShotMaxLife = 0.33;

	float g = 40.0;
	float x = 0.0;
	float y = 0.0;
	
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		int o = m_ParticlesOffset + idx;

		if ((shot_this_tick < 2) && ((m_Models[o]->m_Life > m_ShotMaxLife) || !m_Models[o]->m_IsAlive)) {
			ShootParticle(o);
			shot_this_tick++;
		} else {
			not_shot_this_tick++;
			if ((m_Models[o]->m_Life > m_ShotMaxLife)) {
				ResetParticle(o);
			}
		}
		

		float theta = m_Models[o]->m_Theta;
		float v = m_Models[o]->m_Velocity[0];
		m_Models[o]->m_Life += m_DeltaTime;
		
		x = v * cos(theta) * m_Models[o]->m_Life;
		y = (v * fastSinf(theta) * m_Models[o]->m_Life) - (0.5 * g * (m_Models[o]->m_Life * m_Models[o]->m_Life));
		
		float tx = -sin(DEGREES_TO_RADIANS(m_Models[o]->m_Rotation[1]));
		float tz = cos(DEGREES_TO_RADIANS(m_Models[o]->m_Rotation[1]));
		
		m_Models[o]->m_Position[0] = (x * -(tx)) + m_Models[0]->m_Position[0] - (tx * 2.0);
		m_Models[o]->m_Position[1] = y - 0.5;
		m_Models[o]->m_Position[2] = (x * -(tz)) + m_Models[0]->m_Position[2] - (tz * 2.0);
		
	}

	
	m_AtlasSprites[0]->Simulate(m_DeltaTime);
	
	bool moveForward = false;
	bool moveBackward = false;
	bool turnLeft = false;
	bool turnRight = false;
	float sliderDelta;
	
	sliderDelta = leftSliderValue - rightSliderValue;
	
	float absDelta = fastAbs(sliderDelta);
		
	if (absDelta > 0.1) {
		if (sliderDelta < 0.0) {
			turnLeft = true;
		} else {
			turnRight = true;
		}
	}
	
	
	//TODO: figure this out
	if (rightSliderValue > 0.95 && leftSliderValue > 0.95) {
		moveForward = true;
	} else if (rightSliderValue < 0.04 && leftSliderValue < 0.04) {
		moveBackward = true;
	}
	
	moveForward = true;
	
	if (turnLeft) {
		m_Models[0]->m_Rotation[1] += kTurnRate * absDelta * m_DeltaTime;
	} else if (turnRight) {
		m_Models[0]->m_Rotation[1] -= kTurnRate * absDelta * m_DeltaTime;
	}
		
	if (moveForward) {
		m_Models[0]->m_Velocity[0] += kTankAcceleration;
		if (m_Models[0]->m_Velocity[0] > kMaxTankSpeed) {
			m_Models[0]->m_Velocity[0] = kMaxTankSpeed;
		}
	} else if (moveBackward) {
		m_Models[0]->m_Velocity[0] -= kTankAcceleration;
		if (m_Models[0]->m_Velocity[0] < -kMaxTankSpeed) {
			m_Models[0]->m_Velocity[0] = -kMaxTankSpeed;
		}
	}
	
	m_Models[0]->Simulate(m_DeltaTime, false);

	if (m_CameraIndex == 0) {
		m_CameraTarget[0] = m_Models[0]->m_Position[0];
		m_CameraTarget[1] = m_Models[0]->m_Position[1];
		m_CameraTarget[2] = m_Models[0]->m_Position[2];

		m_CameraRotation += DEGREES_TO_RADIANS(0.5);
		m_CameraHeight = 0.5; // + (fastSinf(m_SimulationTime * 0.5) * 5.0);
		float m_CameraDiameter = 20.0; // + fastAbs(fastSinf(m_SimulationTime * 0.1) * 25.0);
		float cx = (cos(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[0];
		float cz = (fastSinf(m_CameraRotation) * m_CameraDiameter) + m_CameraTarget[2];
		
		m_CameraPosition[0] = cx;
		m_CameraPosition[1] = m_CameraTarget[1] + m_CameraHeight;
		m_CameraPosition[2] = cz;
	} else if (m_CameraIndex == 1) {
		float tx = -sin(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1]));
		float tz = cos(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1]));
		
		float txx = -sin(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1] + 90.0));
		float tzz = cos(DEGREES_TO_RADIANS(m_Models[0]->m_Rotation[1] + 90.0));
		
		/*
		m_CameraTarget[0] = m_Models[0]->m_Position[0] + (tx * 60.0);
		m_CameraTarget[1] = 0.0125;
		m_CameraTarget[2] = m_Models[0]->m_Position[2] + (tz * 60.0);
		m_CameraPosition[0] = m_Models[0]->m_Position[0] - (tx * 0.25) - (txx * 0.4);
		m_CameraPosition[1] = 0.125;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] - (tz * 0.25) - (tzz * 0.4);
		*/
		
		m_CameraTarget[0] = m_Models[0]->m_Position[0] + (tx * 60.0);
		m_CameraTarget[1] = 0.0125;
		m_CameraTarget[2] = m_Models[0]->m_Position[2] + (tz * 60.0);
		m_CameraPosition[0] = m_Models[0]->m_Position[0] - (tx * 10.0) - (txx * 0.0);
		m_CameraPosition[1] = 1.0;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] - (tz * 10.0) - (tzz * 0.0);
	} else if (m_CameraIndex == 2) {
		m_CameraTarget[0] = 0.0;
		m_CameraTarget[1] = 0.0;
		m_CameraTarget[2] = 0.0;
		m_CameraPosition[0] = 1.0;
		m_CameraPosition[1] = 300.0;
		m_CameraPosition[2] = 1.0;
	} else if (m_CameraIndex == 3) {
		m_CameraTarget[0] = m_Models[0]->m_Position[0];
		m_CameraTarget[1] = m_Models[0]->m_Position[1];
		m_CameraTarget[2] = m_Models[0]->m_Position[2];
		m_CameraPosition[0] = m_Models[0]->m_Position[0] + 1;
		m_CameraPosition[1] = m_Models[0]->m_Position[1] + 60.0;
		m_CameraPosition[2] = m_Models[0]->m_Position[2] + 1;
	}
	
	return 1;
}

void MainMenu::RenderModelPhase() {

	
	
	
	glMaterialfv(   GL_FRONT_AND_BACK, GL_AMBIENT,   objAmbientSunkenDuck  );
	glMaterialfv(   GL_FRONT_AND_BACK, GL_DIFFUSE,   objDiffuseSunkenDuck  );
	glMaterialfv(   GL_FRONT_AND_BACK, GL_SPECULAR,  objSpecularSunkenDuck );
	glMaterialfv(   GL_FRONT_AND_BACK, GL_EMISSION,  objEmissionSunkenDuck );
	glMaterialx(    GL_FRONT_AND_BACK, GL_SHININESS,     5 << 16     );
	
	// Draw duck reflection by using y-scale of -1.0
	//DrawDuck(  aFrame, aTimeSecs, -1.0f );
	

	DrawPlayer(-1.0);
	
	
	// Draw semitransparent water surface
	DrawWater();
	
	Model::ReleaseBuffers();
	
	DrawRipples();

		
	// Set duck material
	glMaterialfv(   GL_FRONT_AND_BACK, GL_AMBIENT,   objAmbientDuck  );
	glMaterialfv(   GL_FRONT_AND_BACK, GL_DIFFUSE,   objDiffuseDuck  );
	glMaterialfv(   GL_FRONT_AND_BACK, GL_SPECULAR,  objSpecularDuck );
	glMaterialfv(   GL_FRONT_AND_BACK, GL_EMISSION,  objEmissionDuck );
	glMaterialx(    GL_FRONT_AND_BACK, GL_SHININESS,     5 << 16     );
	
	DrawPlayer(1.0);	
}

void MainMenu::DrawWater() {
	glDisable( GL_LIGHTING );
	glDisableClientState( GL_NORMAL_ARRAY );

	
	// Set array pointers for water model.
	glVertexPointer( 3, GL_FLOAT, 0, objVertexDataWater );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glNormal3f( 0.f, 1.f, 0.f );
	//glColor4f(0.204, 0.796, 0.988, 0.600 );
	glDrawElements( GL_TRIANGLE_STRIP, WATER_VERTICES, GL_UNSIGNED_BYTE, objIndicesWater );
	//glColor4f(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_BLEND);	
	
	// Enable lighting and normal arrays
	glEnable( GL_LIGHTING );
	glEnableClientState( GL_NORMAL_ARRAY );
}


void MainMenu::DrawRipples() {

// Update the ripple vertex positions
GLint i, n;
GLfloat angle;
const GLfloat maxRadius = 100.0;

// Disable lighting and normals
//glDisable( GL_LIGHTING );
glDisableClientState( GL_NORMAL_ARRAY );

// Set array pointers for the ripple model.
glVertexPointer( 3, GL_FLOAT, 0, iRippleVertices );

// Speed up the animation
float aTimeSecs = m_SimulationTime * 3;

glDisable( GL_DEPTH_TEST );
glEnable( GL_BLEND );
glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
glNormal3f( 0.f, 1.f, 0.f );

for (n = 0; n < 3; n++)
    {
    GLfloat radius = (6 * aTimeSecs + 30 * n) - maxRadius * (GLint)((6 * aTimeSecs + 30 * n) / maxRadius);

    for (i = 0, angle = 0; i < RIPPLE_SEGMENTS; i++, angle += 2 * M_PI / RIPPLE_SEGMENTS)
        {
        // inner ring
        iRippleVertices[(i * 2 + 0) * 3 + 0] = sin(angle) * (radius + 4 * cos(angle * 8 + aTimeSecs));
        iRippleVertices[(i * 2 + 0) * 3 + 1] = 0;
        iRippleVertices[(i * 2 + 0) * 3 + 2] = cos(angle) * (radius + 4 * cos(angle * 6 + aTimeSecs * 4));
        // outer ring
        iRippleVertices[(i * 2 + 1) * 3 + 0] = sin(angle) * (radius + 15 + 4 * sin(angle * 5 - aTimeSecs * 3));
        iRippleVertices[(i * 2 + 1) * 3 + 1] = 0;
        iRippleVertices[(i * 2 + 1) * 3 + 2] = cos(angle) * (radius + 15 + 4 * sin(angle * 7 - aTimeSecs * 2));
        }

    glColor4f(1, 1, 1, .6 * (1.0 - radius / maxRadius));
    glDrawElements( GL_TRIANGLE_STRIP, RIPPLE_SEGMENTS * 2 + 2, GL_UNSIGNED_SHORT, iRippleIndices );
    }

glDisable( GL_BLEND );
glEnable( GL_DEPTH_TEST );

// Enable lighting and normal arrays
glEnable( GL_LIGHTING );
glEnableClientState( GL_NORMAL_ARRAY );

	

}


void MainMenu::DrawPlayer(float yScale) {
	// Store the model view matrix
	glPushMatrix();
	
	// Reflection and the light position must be mirrored
	glScalef( 1.f, yScale, 1.f );
	glLightfv(  GL_LIGHT0, GL_POSITION, lightPositionLamp );
	
    // Define the water clip plane that prevents the duck reflection
    // from being drawn above the water
	GLfloat coeff[4] =  { 0.f, 1.f, 0.f, 0.f };
	glClipPlanef( GL_CLIP_PLANE0, coeff );
	glEnable( GL_CLIP_PLANE0 );
	
	/*
	// Make the duck bob up and down
	glTranslatef( 0.f, 7.f + cos( aTimeSecs * 2.f ) * 7.f, 0.f );
	
	// Rotate the duck around the y-axis
	glRotatef( aTimeSecs * 3.f, 0.f, 1.f, 0.f );
	
	// Make the duck swing back and forth
	glRotatef( sin( aTimeSecs * 3.f) * -5.f, 0.f, 0.f, 1.f );
	glRotatef( cos( aTimeSecs ) * -5.f, 1.f, 0.f, 0.f );
	*/
	
	RenderModelRange(0, 1);

    // Disable the clipping plane
	glDisable( GL_CLIP_PLANE0 );
	
    // Restore model view matrix
	glPopMatrix();
}

void MainMenu::RenderSpritePhase() {
	RenderSpriteRange(0, 2);
}

void MainMenu::BuildParticles(int n) {
	m_NumParticles = n;
	m_ShootInterval = 1.0;
	m_ParticleStreamIndex = 0;
	m_ParticlesOffset = m_Models.size();
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		int o = m_ParticlesOffset + idx;  
		m_Models.push_back(new Model(m_FooFoos.at(1)));
		m_Models[o]->SetTexture(m_Textures->at(3));
		m_Models[o]->SetFrame(0);
		ResetParticle(o);
	}
}


void MainMenu::ResetParticles() {
	for (unsigned int idx=0; idx<m_NumParticles; idx++) {
		ResetParticle(idx);
	}
}


void MainMenu::ResetParticle(int idx) {	
	m_Models[idx]->SetPosition(m_Models[0]->m_Position[0], m_Models[0]->m_Position[1], m_Models[0]->m_Position[2]);
	m_Models[idx]->m_Life = 0.0 - (randf() * 20);
	m_Models[idx]->SetScale(0.25, 0.25, 0.25);
	m_Models[idx]->m_Theta = DEGREES_TO_RADIANS(45);
	m_Models[idx]->m_Velocity[0] = 12.0 - (randf() * 8.0);
	m_Models[idx]->m_IsAlive = false;
}


void MainMenu::ShootParticle(int idx) {
	m_Models[idx]->m_Theta = DEGREES_TO_RADIANS((fastAbs(randf()) * 30.0) + 20.0);
	//DEGREES_TO_RADIANS(20.0 + (fastSinf(m_SimulationTime) * 10));
	m_Models[idx]->SetPosition(m_Models[0]->m_Position[0], m_Models[0]->m_Position[1], m_Models[0]->m_Position[2]);
	
	switch (m_ParticleStreamIndex++ % 3) {
		case 0:
			m_Models[idx]->m_Rotation[1] = (m_Models[0]->m_Rotation[1] + 25) + (randf() * 7.0);
			break;
		case 1:
			m_Models[idx]->m_Rotation[1] = (m_Models[0]->m_Rotation[1]) + (randf() * 2.0);
			break;
		case 2:
			m_Models[idx]->m_Rotation[1] = (m_Models[0]->m_Rotation[1] - 25) + (randf() * 7.0);
			break;
	};
	
	m_Models[idx]->m_Life = 0.0;
	m_Models[idx]->m_IsAlive = true;
}