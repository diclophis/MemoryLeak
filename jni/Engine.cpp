//
//  Engine.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//

#include "math.h"
#include <sstream>
#include <sys/time.h>
#include "pthread.h"

#include "importgl.h"
#include "OpenGLCommon.h"

#include "assimp.hpp"

#include "Engine.h"


#include <include/IOStream.h>
#include <include/IOSystem.h>


class ResourceIOStream : public Assimp::IOStream {
  public:
    ResourceIOStream(foo &a) : m_Foo(&a) {
	  };

	  ~ResourceIOStream()
	  {}
	
	  size_t Read(void* buffer, size_t size, size_t count) {
		  fseek(m_Foo->fp, m_Foo->off, SEEK_SET);
		  size_t r = fread(buffer, size, count, m_Foo->fp);
		  return r;
	  }
	
	  size_t Write( const void* buffer, size_t size, size_t count) {
		  return 0;
	  }
	
	  aiReturn Seek( size_t offset, aiOrigin origin)
	  {
		  int seeked;
	    switch (origin)
	    {
	    case aiOrigin_SET:
				seeked = fseek(m_Foo->fp, offset, SEEK_SET);

	      break;
	    case aiOrigin_CUR:
				seeked = fseek(m_Foo->fp, offset, SEEK_CUR);

	      break;
	    case aiOrigin_END:
				seeked = fseek(m_Foo->fp, offset, SEEK_END);

	      break;
	    default:
				LOGV("wtf");
	    }

		  if (seeked == 0) {
			  return aiReturn_SUCCESS;
		  } else {
			  return aiReturn_FAILURE;
		  }

	  }
	
	  size_t Tell() const
	  {
		  return ftell(m_Foo->fp);
	  }
	
	  size_t FileSize() const
	  {
		  return m_Foo->len;
	  }
	
	  void Flush() {
	  }
	
	private:
	
		foo *m_Foo;
	};
	
	class ResourceIOSystem : public Assimp::IOSystem
	{
	public:

		
		ResourceIOSystem(std::vector<GLuint> &a,std::vector<foo*> &b) : m_Textures(&a), m_Models(&b) {
		};
	
	  ~ResourceIOSystem()
	  {
	  }
	
	  // Check whether a specific file exists
	  bool Exists(const char* file) const
	  {
	    return true;
	  }
	
	  // Get the path delimiter character we'd like to see
	  char getOsSeparator() const
	  {
	    return '/';
	  }
	
	  // ... and finally a method to open a custom stream
	  Assimp::IOStream* Open(const char* file, const char* mode)
	  {
		int index = atoi(file);
		  return new ResourceIOStream(*m_Models->at(index));
	  }
	
	  void Close(Assimp::IOStream* stream) {
		  delete stream;
	  }
	
	private:
		std::vector<GLuint> *m_Textures;
		std::vector<foo*> *m_Models;
	};


namespace OpenSteer {
	bool updatePhaseActive = false;
	bool drawPhaseActive = false;
	bool enableAnnotation = false;
}


Engine::~Engine() {
	pthread_mutex_destroy(&m_mutex);
	textures->clear();
	models->clear();
}



inline std::string Engine::stringify(double x) {
	std::ostringstream o;
	if (!(o << x))
		throw 987;
	return o.str();
}


Engine::Engine(int width, int height, std::vector<GLuint> &x_textures, std::vector<foo*> &x_models) : screenWidth(width), screenHeight(height), textures(&x_textures), models(&x_models) {
	
	mNeedsTick = false;
	mySceneBuilt = false;
	myViewportSet = false;

	// Engine!!!
	//Screen
	screenWidth = width;
	screenHeight = height;
	
	// put my custom IO handling in place
	importer.SetIOHandler(new ResourceIOSystem(*textures, *models));
	
	buildCamera();
	buildFont();
}


void Engine::buildCamera() {
	myCameraTarget = Vector3DMake(0.0, 0.0, 0.0);
	myCameraPosition = Vector3DMake(0.0, 0.0, 0.0);
	myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
}


void Engine::go() {
	build();
	mySceneBuilt = true;
	mySimulationTime = 0.0;		
	pthread_mutex_init(&m_mutex, 0);
	pthread_create(&m_thread, 0, Engine::start_thread, this);
}


void *Engine::start_thread(void *obj) {
	reinterpret_cast<Engine *>(obj)->tick();
	return 0;
}


void Engine::pause() {
  LOGV("pausing in engine\n");
  gameState = 0;
}


int Engine::tick() {
  gameState = -1;
	timeval t1, t2;
	double elapsedTime;
  double interval = 25.0;
  int stalled = 0;
	
	gettimeofday(&t1, NULL);

  float last_waited = 0.0;
  unsigned long waited = 0;

  bool checkTime = false;

	while (gameState != 0) {
		if (mySceneBuilt) {

      if (waited > (last_waited * 0.95)) {
        checkTime = true;
      }

      if (checkTime) {
        gettimeofday(&t2, NULL);
        elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
        elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
        if (elapsedTime > interval) {
          gettimeofday(&t1, NULL);
          if (waited == 0) {
            if (stalled++ > 2) {
              return 0;
            }
          }

          if ((elapsedTime - interval) < interval) {
            myDeltaTime = ((elapsedTime / interval)) * 2.5;
            mySimulationTime += (myDeltaTime);
            pthread_mutex_lock(&m_mutex);
            gameState = simulate();
            pthread_mutex_unlock(&m_mutex);
            checkTime = false;
          } else {
            int times = (elapsedTime / interval);
            myDeltaTime = 2.5; //(interval / elapsedTime);
            for (int i=0; i<times; i++) {
              mySimulationTime += (myDeltaTime);
              pthread_mutex_lock(&m_mutex);
              gameState = simulate();
              pthread_mutex_unlock(&m_mutex);
            }
          }

          last_waited = waited;
          waited = 0;
        }
      }
      
      waited++;
		}
	}

  LOGV("exiting tick thread\n");
	
	return gameState;
}


void Engine::draw(float rotation) {
	pthread_mutex_lock(&m_mutex);
	if (mySceneBuilt) {
		if (myViewportSet) {
			glPushMatrix();
			{
				glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
				glMatrixMode(GL_MODELVIEW);
				glLoadIdentity();
				glRotatef(rotation, 0.0, 0.0, 1.0);
				drawCamera();
				render();
			}
			glPopMatrix();
		} else {
			prepareFrame(screenWidth, screenHeight);
		}
	}
	pthread_mutex_unlock(&m_mutex);

}

#ifndef DESKTOP
static void gluPerspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar) {
	GLfloat xmin, xmax, ymin, ymax;
	
	ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
	ymin = -ymax;
	xmin = ymin * aspect;
	xmax = ymax * aspect;
	
	glFrustumx(
			   (GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
			   (GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
			   (GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536)
			   );
}
#endif


void Engine::prepareFrame(int width, int height) {
	glViewport(0, 0, width, height);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0, (float) width / (float) height, 1.0, 5000.0);
	//GOOOOOD gluPerspective(45.0, (float) width / (float) height, 100.0, 10000.0);
	gluPerspective(50.0, (float) width / (float) height, 0.01, 200.0);

	//gluPerspective(90.0, (float) width / (float) height, 1.0, 1000.0);

  //glOrthof(0, 320, 0, 480, -100.0, 500.0);

	glMatrixMode(GL_MODELVIEW);

  //glEnable(GL_LIGHTING);
  //glEnable(GL_LIGHT0);
  //const GLfloat light0Ambient[] = {0.75, 0.75, 0.75, 1.0};
  //glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);


  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // Define the ambient component of the first light
  const GLfloat light0Ambient[] = {0.75, 0.75, 0.75, 1.0};
  glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);

  // Define the diffuse component of the first light
  const GLfloat light0Diffuse[] = {0.7, 0.7, 0.7, 1.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);

  // Define the specular component and shininess of the first light
  const GLfloat light0Specular[] = {0.7, 0.7, 0.7, 1.0};
  const GLfloat light0Shininess = 0.4;
  glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);

  // Define the position of the first light
  GLfloat light0Position[] = {0.0, 1.0, 1.0, 0.0}; 
  glLightfv(GL_LIGHT0, GL_POSITION, light0Position); 

  // Define a direction vector for the light, this one points right down the Z axis
  //GLfloat light0Direction[] = {-1.0, 1.0, 0.0};
  //glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0Direction);

  // Define a cutoff angle. This defines a 90° field of vision, since the cutoff
  // is number of degrees to each side of an imaginary line drawn from the light's
  // position along the vector supplied in GL_SPOT_DIRECTION above
  //glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45.0);
  //glDisable(GL_LIGHTING);

	glLoadIdentity();
	myViewportSet = true;
}


void Engine::resizeScreen(int width, int height) {
	screenWidth = width;
	screenHeight = height;
	myViewportSet = false;
}


void Engine::buildFont() {	
	
	m_charPixelWidth = m_animPixelWidth = CHAR_PIXEL_W;
	m_charPixelHeight = m_animPixelHeight = CHAR_PIXEL_H;
	
	for(int i=0; i<12; i++)
	{
		charGeomV[i] = 0.0;
	}
	
	// Pre-generate all possible texture coords
	for(int c=FONT_TEXTURE_ATLAS_WIDTH * FONT_TEXTURE_ATLAS_LINES; c>=0; c--)
	{
		int character = c * ONE_CHAR_SIZE_T;
		charTexCoords[character] = charTexCoords[character+4] = (c % FONT_TEXTURE_ATLAS_WIDTH) * CHAR_WIDTH;	
		charTexCoords[character+1] = charTexCoords[character+3] = 1.0 - (c / FONT_TEXTURE_ATLAS_WIDTH) * CHAR_HEIGHT;
		charTexCoords[character+2] = charTexCoords[character+6] = charTexCoords[character+0] + CHAR_WIDTH;
		charTexCoords[character+7] = charTexCoords[character+5] = 1.0 - (c / FONT_TEXTURE_ATLAS_WIDTH + 1) * CHAR_HEIGHT;
	}		
}


void Engine::tickFont() {
}


void Engine::drawFont() {
		
	float _scaleX = 1.0;
	float _scaleY = 1.0;

	if (m_animPixelWidth > m_charPixelWidth) {
		m_animPixelWidth /= 1.5;
	} else {
		m_animPixelWidth = m_charPixelWidth;
	}

	if (m_animPixelHeight > m_charPixelHeight) {
		m_animPixelHeight /= 1.5;
	} else {
		m_animPixelHeight = m_charPixelHeight;
	}

	m_ntextWidth = screenWidth / m_animPixelWidth;
	m_ntextHeight = screenHeight / m_animPixelHeight;
	m_fCharacterWidth = 1.0 / m_ntextWidth;
	m_fCharacterHeight = 1.0 / m_ntextHeight;

	//glBindTexture(GL_TEXTURE_2D, textures->at(1));

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();	
	glLoadIdentity();

#ifdef DESKTOP
	glOrtho(0, _scaleX, 0, _scaleY, -1, 1);
#else
	glOrthof(0, _scaleX, 0, _scaleY, -1, 1);
#endif
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float x;
	std::string fps;

	if (myGameStarted) {
		x = 0.05;
		fps = "S:" + stringify((int)myPlayerSpeed.x) + " D:" + stringify((int)myPlayerPosition.x);
	} else {
		x = 0.0;
		fps = "Escape from Raptor Island";
	}
	 
	float y = 0.875;
	
	for (unsigned int i=0; i<fps.length(); i++) {
		
		int c = fps.at(i);

		if (c == ' ') {
			if (myGameStarted) {
				x = 0.0;
			} else {
				x = -m_fCharacterWidth;
			}
			y -= 0.055;
		}
		
		c -= ' ';

		m_nCurrentChar = 0;

		// TexCoords
		int offsetT = m_nCurrentChar - (m_nCurrentChar / 3);	// 12 / 3 = 4 So 12 - 4 = 8
		memcpy(&charGeomT[offsetT], &charTexCoords[c * ONE_CHAR_SIZE_T], ONE_CHAR_SIZE_T * sizeof(GLfloat));
		
		// Vertex Xs
		charGeomV[m_nCurrentChar + 0] = charGeomV[m_nCurrentChar + 6] = x;
		charGeomV[m_nCurrentChar + 3] = charGeomV[m_nCurrentChar + 9] = x + m_fCharacterWidth;

		// Vertex Ys
		charGeomV[m_nCurrentChar + 1] = charGeomV[m_nCurrentChar + 4] = y;
		charGeomV[m_nCurrentChar + 10] = charGeomV[m_nCurrentChar + 7] = y + m_fCharacterHeight;
		charGeomV[m_nCurrentChar + 2] = charGeomV[m_nCurrentChar + 5] = charGeomV[m_nCurrentChar + 8] = charGeomV[m_nCurrentChar + 11] = 0.0;

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT,0, &charGeomT);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &charGeomV);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		x += m_fCharacterWidth;
	}
	 
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();		

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}


void Engine::drawCamera() {	
	gluLookAt(
		myCameraPosition.x, myCameraPosition.y, myCameraPosition.z,
		myCameraTarget.x, myCameraTarget.y, myCameraTarget.z,
		0.0, 1.0, 0.0
	);
}


//returns a random float between 0 and 1
float Engine::randf() {
	//#define ARC4RANDOM_MAX 0x100000000LL
	//return floorf(((double)arc4random() / ARC4RANDOM_MAX));
	return (lrand48() % 255) / 255.f;
}


void Engine::reset_vertex(int idx) {
	int i = idx * 3;
	vertices[i + 0] = generator[idx].x;
	vertices[i + 1] = generator[idx].y;
	vertices[i + 2] = generator[idx].z;
}


void Engine::random_velocity(int idx) {
	//velocity[idx].x = -0.35 + (randf() * 0.01);
	//velocity[idx].y = 0.25 - (randf() * 0.5);
	//velocity[idx].z = 0.25 - (randf() * 0.5);
	/*
	velocity[idx].x = -0.35 + (randf() * 0.01);
	velocity[idx].y = 0.25 - (randf() * 0.5);
	velocity[idx].z = 0.25 - (0.5) + (randf() * 0.002 * myPlayerSpeed.x);
	*/

	
	velocity[idx].x = 1.0 - randf() * 2.0;
	velocity[idx].y = (randf() * 1.0) + 3.0;
	velocity[idx].z = 1.0 - randf() * 2.0;
	

	//velocity[idx].x = 0.1;
	//velocity[idx].y = 0.1;
	//velocity[idx].z = 0.1;
}


void Engine::reset_particle(int idx) {
	generator[idx].x = myFountainPosition.x;
	generator[idx].y = myFountainPosition.y;
	generator[idx].z = myFountainPosition.z;

	
	reset_vertex(idx);
	random_velocity(idx);
	reset_life(idx);
	
	//LOGV("RESET: %d %f %f\n", idx, vertices[idx * 3], myPlayerPosition.x);

}

void Engine::update_vertex(int idx) {
	int i = idx * 3;
	vertices[i] += velocity[idx].x;
	vertices[i+1] += velocity[idx].y;
	vertices[i+2] += velocity[idx].z;
	velocity[idx].y -= 1.0; 
}


/*
static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{1.0f,1.0f,1.0f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},{0.9f,0.9f,0.9f},
	{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},{0.5f,0.5f,0.5f},
	{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f}
};
 */

static GLfloat ccolors[12][3]=				// Rainbow Of Colors
{
	{0.0f,1.0f,0.0f},{0.0f,0.9f,0.1f},{0.0f,0.9f,0.1f},{0.0f,0.9f,0.1f},
	{0.0f,0.5f,0.1f},{0.0f,0.5f,0.1f},{0.0f,0.5f,0.1f},{0.0f,0.5f,0.5f},
	{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f},{0.25f,0.25f,0.25f}
};


void Engine::update_color(int idx) {
	int i = idx * 4;
	//float distanceFromPlayer = myPlayerPosition.x - vertices[idx * 3];
	//float percentOf = (distanceFromPlayer) / 40.0;
	int ii = randf() * 11;//(int)(percentOf * 12);
	//if (ii > 11) {
	//	ii = 11;
	//}
	
	colors[i+0] = ccolors[ii][0];
	colors[i+1] = ccolors[ii][1];
	colors[i+2] = ccolors[ii][2];
	colors[i+3] = 1.0; //i / (float)11.0;	
}


void Engine::reset_life(int i) {
	life[i] = 1.0;
}


void Engine::buildFountain() {	
	srand48(time(NULL));
	
	int i = 0;
	for(i=0;i<NUM_PARTICLES;i++) {
		elements[i] = i;
	}
	
	for(i=0;i<NUM_PARTICLES;i++) {
		reset_particle(i);
		life[i] -= (float)i / (float)NUM_PARTICLES;
	}	
}


void Engine::tickFountain() {	
	int i = 0; //particle index
	for(i=0; i<NUM_PARTICLES; i++) {
		life[i] -= 0.1;
		if(life[i] <= 0.0) {
			reset_particle(i);
		} else {
			update_color(i);
			update_vertex(i);
		}
	}
}


void Engine::drawFountain() {
	glDisable(GL_DEPTH_TEST);
	if (false) {
		glBindTexture(GL_TEXTURE_2D, textures->at(5));

#ifdef DESKTOP
		glEnable(GL_POINT_SPRITE);
		glPointSize(5.0);
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		//glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_FALSE);
		glEnableClientState(GL_VERTEX_ARRAY); 
		glVertexPointer(3, GL_FLOAT, 0, vertices); 
		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
		//glColor4f(1.0, 1.0, 1.0, 1.0);
		glDisable(GL_POINT_SPRITE);
#else
		glEnable(GL_POINT_SPRITE_OES);
		glPointSize(5.0);
		glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_TRUE);
		//glTexEnvi(GL_POINT_SPRITE_OES, GL_COORD_REPLACE_OES, GL_FALSE);
		glEnableClientState(GL_VERTEX_ARRAY); 
		glVertexPointer(3, GL_FLOAT, 0, vertices); 
		glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);
		//glColor4f(1.0, 1.0, 1.0, 1.0);
		glDisable(GL_POINT_SPRITE_OES);
#endif
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_FLOAT, 0, colors);
		glPointSize(4.0);
		glDrawElements(GL_POINTS, NUM_PARTICLES, GL_UNSIGNED_SHORT, elements);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}
	glEnable(GL_DEPTH_TEST);
}


