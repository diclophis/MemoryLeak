//
//  Engine.m
//  MemoryLeak
//
//  Created by Jon Bardin on 9/7/09.
//

#include "MemoryLeak.h"

//#include <yajl/yajl_parse.h>
//#include <yajl/yajl_gen.h>
#include <api/yajl_parse.h>
#include <api/yajl_gen.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Model.h"
#include "Engine.h"

static float xyz[3];
static int xyz_index = 0;
static int model_index = 0;

static int command = 0;
static int next_scene = 1;
static int set_camera_position = 2;
static int set_camera_target = 3;
static int render_models = 4;
static int render_model = 5;
static int set_model_position = 6;
static int set_model_rotation = 7;
static int set_model_scale = 8;
static int first_model = 9;
static int next_model = 10;

static std::vector<Model *> m_Models;
static std::vector<foofoo *> m_FooFoos;
static float m_CameraPosition[3];
static float m_CameraTarget[3];

static int reformat_null(void * ctx)
{
    return 1;
}

static int reformat_boolean(void * ctx, int boolean)
{
    return 1;
}

static int reformat_number(void * ctx, const char * s, unsigned int l)
{
  xyz[xyz_index++] = strtof(s, NULL);
  return 1;
}

static int reformat_string(void * ctx, const unsigned char * stringVal, unsigned int stringLen)
{
    return 1;
}

static int reformat_map_key(void * ctx, const unsigned char * stringVal, unsigned int stringLen)
{
    return 1;
}

static int reformat_start_map(void * ctx)
{
    return 1;
}


static int reformat_end_map(void * ctx)
{
    return 1;
}

static int reformat_start_array(void * ctx)
{
  //LOGV("start_array\n");
  if (command == 0) { //set command = set_camera_position and set xyz_index = 0 if command == 0
    //LOGV("0\n");
    command = next_scene;
  } else if (command == next_scene) {
    //LOGV("next_scene\n");
    command = set_camera_position;
    xyz_index = 0;
  } else if (command == set_camera_position) { //set command = set_camera_target and set xyz_index = 0 if command == set_camera_position
    //LOGV("set_camera_position\n");
    command = set_camera_target;
    xyz_index = 0;
  } else if (command == set_camera_target) { //set command = render_models and model_index = 0 if command == set_camera_target
    //LOGV("set_camera_target\n");
    command = render_models;
    model_index = 0;
  } else if (command == render_models) { //set command = next_model if command == render_models
    //LOGV("render_models\n");
    command = first_model;
  } else if (command == first_model) { //set command = render_model and xyz_index = 0 if command == next_model
    //LOGV("next_model\n");
    command = render_model;
    xyz_index = 0;
  } else if (command == next_model) { //set command = render_model and xyz_index = 0 if command == next_model
    //LOGV("next_model\n");
    command = first_model;
    xyz_index = 0;
  } else if (command == render_model) { //set command = set_model_position and xyz_index = 0 if command == render_model
    //LOGV("render_model\n");
    command = set_model_position;
    xyz_index = 0;
  } else if (command == set_model_position) { //set command = set_model_rotation and set xyz_index = 0 if command == set_model_position
    //LOGV("set_position\n");
    command = set_model_rotation;
    xyz_index = 0;
  } else if (command == set_model_rotation) { //set command = set_model_scale and set xyz_index = 0 if command == set_model_rotation
    //LOGV("set_rotation\n");
    command = set_model_scale;
    xyz_index = 0;
  }

  return 1;
}

static int reformat_end_array(void * ctx)
{
  //LOGV("end_array\n");
  if (command == set_camera_position) { //set camera position = last_vector if command == set_camera_position
    //LOGV("set camera position\n");
    m_CameraPosition[0] = xyz[0];
    m_CameraPosition[1] = xyz[1];
    m_CameraPosition[2] = xyz[2];
  } else if (command == set_camera_target) { //set camera target = last_vector if command == set_camera_target
    //LOGV("set camera target\n");
    m_CameraTarget[0] = xyz[0];
    m_CameraTarget[1] = xyz[1];
    m_CameraTarget[2] = xyz[2];
  } else if (command == render_model) { //if model_index+1 beyond bounds, create model using last vector x/y/z => model_file, texture_file, frame if command == render_model
    //LOGV("render_model %d/%d\n", model_index, m_Models.size());
    if ((model_index + 1) > m_Models.size()) {
      //LOGV("making model\n");
      m_Models.push_back(new Model(m_FooFoos[xyz[0]]));
    }
    //LOGV("set_texture and set_frame\n");
	  int t = reinterpret_cast<Engine *>(ctx)->GetTextureAt(xyz[1]);
    m_Models[model_index]->SetTexture(t);
    m_Models[model_index]->SetFrame(xyz[2]);
  } else if (command == set_model_position) { //set current model_index position equal to last_vector if command == set_model_position
    //LOGV("set_position\n");
    m_Models[model_index]->SetPosition(xyz[0], xyz[1], xyz[2]);
  } else if (command == set_model_rotation) { //set current model_index rotation equal to last_vector if command == set_model_rotation
    //LOGV("set_rotation\n");
    m_Models[model_index]->SetRotation(xyz[0], xyz[1], xyz[2]);
  } else if (command == set_model_scale) {//set current model_index scale equal to last_vector and model_index++ and command = next_model if command == set_model_scale
    //LOGV("set_scale %d\n", model_index);
    m_Models[model_index]->SetScale(xyz[0], xyz[1], xyz[2]);
    //LOGV("inc model_index %d\n", model_index);
    model_index++;
    command = next_model;
  } else if (command == first_model) {
    //LOGV("first_model\n");
  } else if (command == next_model) {
    //LOGV("next_model\n");
    //LOGV("inc model_index %d\n", model_index);
  } else if (command == next_scene) {
    //LOGV("next_scene\n");
  }
  return 1;
}

static yajl_callbacks callbacks = {
    reformat_null,
    reformat_boolean,
    NULL,
    NULL,
    reformat_number,
    reformat_string,
    reformat_start_map,
    reformat_map_key,
    reformat_end_map,
    reformat_start_array,
    reformat_end_array
};







































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
        throw 1;
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
      fflush(m_Foo->fp);
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
	screenWidth = width;
	screenHeight = height;
	importer.SetIOHandler(new ResourceIOSystem(*textures, *models));

  buildCamera();

  int m_PostProcessFlags =  aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_ImproveCacheLocality | aiProcess_GenSmoothNormals | aiProcess_GenNormals | aiProcess_FixInfacingNormals | aiProcess_Triangulate;
  
  importer.ReadFile("0", m_PostProcessFlags);	
  m_FooFoos.push_back(Model::GetFoo(importer.GetScene()));
  importer.FreeScene();

  importer.ReadFile("1", m_PostProcessFlags);	
  m_FooFoos.push_back(Model::GetFoo(importer.GetScene()));
  importer.FreeScene();

  importer.ReadFile("2", m_PostProcessFlags);	
  m_FooFoos.push_back(Model::GetFoo(importer.GetScene()));
  importer.FreeScene();

	pthread_mutex_init(&m_mutex, 0);

	mySimulationTime = 0.0;		
  mySceneBuilt = true;
}


void Engine::buildCamera() {
	myCameraTarget = Vector3DMake(0.0, 0.0, 0.0);
	myCameraPosition = Vector3DMake(0.0, 0.0, 0.0);
	myCameraSpeed = Vector3DMake(0.0, 0.0, 0.0);
}


void Engine::go() {
	build();
	mySceneBuilt = true;
	//pthread_mutex_init(&m_mutex, 0);
	//pthread_create(&m_thread, 0, Engine::start_thread, this);
}


void *Engine::start_thread(void *obj) {
  LOGV("no!!!!!!!!!!!\n");
	reinterpret_cast<Engine *>(obj)->tick();
	return 0;
}


void Engine::pause() {
  LOGV("pausing in engine\n");
  pthread_mutex_lock(&m_mutex);
  gameState = 0;
  pthread_mutex_unlock(&m_mutex);
}


void Engine::parse(const char *fileData, size_t rd) {
  pthread_mutex_lock(&m_mutex);
  yajl_handle hand;
  //static unsigned char fileData[65536];
  /* generator config */
  yajl_status stat;
  //size_t rd;
  /* allow comments */
  yajl_parser_config cfg = { 1, 1 };
  cfg.checkUTF8 = 0;
  command = 0;

	if (rd > 0) {
    //LOGV("got interp: %s\n", fileData);

    hand = yajl_alloc(&callbacks, &cfg, NULL, (void *)this);
    stat = yajl_parse(hand, (const unsigned char*)fileData, rd);

    if (stat != yajl_status_ok && stat != yajl_status_insufficient_data) {
      unsigned char * str = yajl_get_error(hand, 1, (const unsigned char*)fileData, rd);
      LOGV("json error: %s", (const char *) str);
      yajl_free_error(hand, str);
    }

    stat = yajl_parse_complete(hand);
    yajl_free(hand);
  }
  pthread_mutex_unlock(&m_mutex);
}


int Engine::tick() {
  LOGV("starting tick\n");
  /*

  gameState = -1;
  int tickedGameState = -1;

  double t1, t2;
  timeval tim;
  gettimeofday(&tim, NULL);

  int waitedCount = 50;
  int waitedIndex = 0;

  double waitSum = 0.0;
  double averageWait = 0.0;

  for (unsigned int i=0; i<waitedCount; i++) {
    m_Waits[i] = 0.0;
  }

  gettimeofday(&tim, NULL);
  t1=tim.tv_sec+(tim.tv_usec/1000000.0);

	while (gameState != 0) {
		if (mySceneBuilt) {
      if (pthread_mutex_lock(&m_mutex) == 0) {

        gettimeofday(&tim, NULL);
        t2=tim.tv_sec+(tim.tv_usec/1000000.0);

        m_Waits[waitedIndex] = t2 - t1;

        gettimeofday(&tim, NULL);
        t1=tim.tv_sec+(tim.tv_usec/1000000.0);
        
        waitedIndex++;
        if ((waitedIndex % waitedCount) == 0) {
          waitedIndex = 0;
        }

        waitSum = 0.0;
        for (unsigned int i=0; i<waitedCount; i++) {
          waitSum += m_Waits[i];
        }

        averageWait = waitSum / (double)waitedCount;

        if (averageWait > (1.0 / 30.0)) {
          LOGV("slow avg: %f\n", averageWait);
        } else {
          myDeltaTime = averageWait; //(averageWait / 0.15) * (1.0);
          //mySimulationTime += (myDeltaTime);
          //tickedGameState = simulate();
          //rd = fread((void *) fileData, 1, sizeof(fileData) - 1, stdin);

          command = 0;

          //function set_p(i, x,y,y) {
          //world[2][i][1][0] = x;
          //world[2][i][1][1] = y;
          //world[2][i][1][2] = z;
          //}

			}
          //LOGV("done parsing json\n");
        }
        pthread_mutex_unlock(&m_mutex);
        //sched_yield();
      }
    }

    if (tickedGameState == 0) {
      break;
    }
  }


  LOGV("exiting tick thread\n");
	
	return gameState;
  */
}




void Engine::draw(float rotation) {
	if (pthread_mutex_lock(&m_mutex) == 0) {
    if (mySceneBuilt) {
      if (myViewportSet) {
        glPushMatrix();
        {
          glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
          glMatrixMode(GL_MODELVIEW);
          glLoadIdentity();
          glRotatef(rotation, 0.0, 0.0, 1.0);

          gluLookAt(
            m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2],
            m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2],
            0.0, 1.0, 0.0
          );

          for (unsigned int i=0; i<m_Models.size(); i++) {
            m_Models[i]->render(0);
          }
        }
        glPopMatrix();
      } else {
        prepareFrame(screenWidth, screenHeight);
      }
    }
    pthread_mutex_unlock(&m_mutex);
    sched_yield();
  }
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0, (float) width / (float) height, 1.0, 5000.0);
	//GOOOOOD gluPerspective(45.0, (float) width / (float) height, 100.0, 10000.0);
	gluPerspective(20.0, (float)width / (float)height, 0.1, 500.0);
	//gluPerspective(20.0, 1.0, 0.1, 500.0);

	//gluPerspective(90.0, (float) width / (float) height, 1.0, 1000.0);

  //glOrthof(0, 320, 0, 480, -100.0, 500.0);

	glMatrixMode(GL_MODELVIEW);

  //glEnable(GL_LIGHTING);
  //glEnable(GL_LIGHT0);
  //const GLfloat light0Ambient[] = {0.75, 0.75, 0.75, 1.0};
  //glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);


  //glEnable(GL_LIGHTING);
  //glEnable(GL_LIGHT0);
/*
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
	*/
	
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_NORMALIZE);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	

	glLoadIdentity();
	myViewportSet = true;
}


void Engine::resizeScreen(int width, int height) {
	screenWidth = width;
	screenHeight = height;
	myViewportSet = false;
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
	//return (lrand48() % 255) / 255.f;
		static unsigned int mirand = 1;
		unsigned int a;
		mirand *= 16807;
		a = (mirand&0x007fffff) | 0x40000000;
		return( *((float*)&a) - 3.0f );
}

















//DEPC
int Engine::tickX() {
  gameState = -1;
	timeval t1, t2;
	double elapsedTime;
  double interval = 20.0;
  int stalled = 0;
	
	gettimeofday(&t1, NULL);

  float last_waited = 0.0;
  unsigned long waited = 0;

  bool checkTime = false;

  int waitedCount = 30;
  int waitedIndex = 0;

  long double waitSum = 0.0;
  long double averageWait = 0.0;

  for (unsigned int i=0; i<waitedCount; i++) {
    m_Waits[i] = 0.0;
  }

	while (gameState != 0) {
		if (mySceneBuilt) {

      waitSum = 0.0; 
      for (unsigned int i=0; i<waitedCount; i++) {
        waitSum += m_Waits[i];
      }

      averageWait = waitSum / (long double)waitedCount;
      
      if (waited > (averageWait * 0.9)) {
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
            myDeltaTime = ((elapsedTime / interval)) * 2.0;
            mySimulationTime += (myDeltaTime);
            pthread_mutex_lock(&m_mutex);
            gameState = simulate();
            pthread_mutex_unlock(&m_mutex);
            checkTime = false;
          } else {
            int times = (elapsedTime / interval);
            myDeltaTime = 2.0;
            for (int i=0; i<times; i++) {
              mySimulationTime += (myDeltaTime);
              pthread_mutex_lock(&m_mutex);
              gameState = simulate();
              pthread_mutex_unlock(&m_mutex);
            }
          }

          m_Waits[waitedIndex] = last_waited = waited;
          waited = 0;
          waitedIndex++;
          if ((waitedIndex % waitedCount) == 0) {
            waitedIndex = 0;
          }
        }
      }
      
      waited++;
		}
	}

  LOGV("exiting tick thread\n");
	
	return gameState;
}
