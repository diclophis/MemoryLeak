// Jon Bardin GPL 2011


#include "MemoryLeak.h"
#include "MainMenu.h"
#include "SuperStarShooter.h"
#include "RadiantFireEightSixOne.h"
#include "SpaceShipDown.h"
#include "AncientDawn.h"


static std::vector<Game *> games;
static Engine *m_CurrentGame;
static std::vector<FileHandle *> models;
static std::vector<FileHandle *> sounds;
static std::vector<FileHandle *> textures;
static std::vector<FileHandle *> levels;


namespace OpenSteer {
	bool updatePhaseActive = false;
	bool drawPhaseActive = false;
}


#ifdef USE_GLES2

static const char vertex_shader[] =
"attribute vec2 Position;\n"
"attribute vec2 InCoord;\n"
"varying vec2 OutCoord;\n"
"uniform mat4 ModelViewProjectionMatrix;\n"
"void main()\n"
"{\n"
"OutCoord = InCoord;\n"
"gl_Position = ModelViewProjectionMatrix * vec4(Position, 1.0, 1.0);\n"
"}\n";

static const char fragment_shader[] = 
"#ifdef GL_ES\n"
"precision mediump float;\n"
"#endif\n"
"varying vec2 OutCoord;\n"
"uniform sampler2D Sampler;\n"
"void main()\n"
"{\n"
"gl_FragColor = texture2D(Sampler, OutCoord);\n"
"}\n";



void Engine::glTranslatef(float tx, float ty, float tz) {
  ProjectionMatrix[12] += (ProjectionMatrix[0] * tx + ProjectionMatrix[4] * ty + ProjectionMatrix[8] * tz);
  ProjectionMatrix[13] += (ProjectionMatrix[1] * tx + ProjectionMatrix[5] * ty + ProjectionMatrix[9] * tz);
  ProjectionMatrix[14] += (ProjectionMatrix[2] * tx + ProjectionMatrix[6] * ty + ProjectionMatrix[10] * tz);
  ProjectionMatrix[15] += (ProjectionMatrix[3] * tx + ProjectionMatrix[7] * ty + ProjectionMatrix[11] * tz);
  glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, GL_FALSE, ProjectionMatrix);
}


#endif


Engine::~Engine() {
  LOGV("Engine::dealloc\n");
  
  for (std::vector<GLuint>::iterator i = m_Textures.begin(); i != m_Textures.end(); ++i) {
    glDeleteTextures(1, &*i); // yea that happened
  }
  m_Textures.clear();

  for (std::vector<foofoo *>::iterator i = m_FooFoos.begin(); i != m_FooFoos.end(); ++i) {
    delete *i;
  }
  m_FooFoos.clear();

  ClearSprites();

  ClearModels();

  for (std::vector<ModPlugFile *>::iterator i = m_Sounds.begin(); i != m_Sounds.end(); ++i) {
    ModPlug_Unload(*i);
  }
  m_Sounds.clear();


  free(m_StateFoo);

#ifdef USE_GLES2

  glDetachShader(program, v);
  glDetachShader(program, f);
  glDeleteShader(v);
  glDeleteShader(f);
  glDeleteProgram(program);

#endif

}


void Engine::ClearSprites() {
  for (std::vector<SpriteGun *>::iterator i = m_AtlasSprites.begin(); i != m_AtlasSprites.end(); ++i) {
    delete *i;
  }
  m_AtlasSprites.clear();
  m_SpriteCount = 0;
}


void Engine::ClearModels() {
  for (std::vector<Model *>::iterator i = m_Models.begin(); i != m_Models.end(); ++i) {
    delete *i;
  }
  m_Models.clear();
  m_ModelCount = 0;
}


Engine::Engine(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : m_ScreenWidth(w), m_ScreenHeight(h), m_TextureFileHandles(&t), m_ModelFileHandles(&m), m_LevelFileHandles(&l), m_SoundFileHandles(&s) {

  m_SpriteCount = 0;
  m_ModelCount = 0;

	m_IsSceneBuilt = false;
	m_IsScreenResized = false;

	m_SimulationTime = 0.0;		
	m_GameState = 2;
  m_Zoom = 1.0;
  m_Fov = 10.0;

	m_IsPushingAudio = false;



  m_CurrentSound = 0;

#ifdef USE_GLES2

  // Compile the vertex shader
  p = vertex_shader;
  v = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(v, 1, &p, NULL);
  glCompileShader(v);
  glGetShaderInfoLog(v, sizeof msg, NULL, msg);
  LOGV("vertex shader info: %s\n", msg);

  // Compile the fragment shader
  p = fragment_shader;
  f = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(f, 1, &p, NULL);
  glCompileShader(f);
  glGetShaderInfoLog(f, sizeof msg, NULL, msg);
  LOGV("fragment shader info: %s\n", msg);

  // Create and link the shader program
  program = glCreateProgram();
  LOGV("engine shader ID: %d\n", program);
  glAttachShader(program, v);
  glAttachShader(program, f);
  //glBindAttribLocation(program, 0, "Position");
  //glBindAttribLocation(program, 1, "InCoord");

  glLinkProgram(program);
  glGetProgramInfoLog(program, sizeof msg, NULL, msg);
  LOGV("info: %s\n", msg);

  glUseProgram(program);

  // Get the locations of the uniforms so we can access them
  ModelViewProjectionMatrix_location = glGetUniformLocation(program, "ModelViewProjectionMatrix");

#endif

  m_StateFoo = new StateFoo(program); //(StateFoo *)malloc(1 * sizeof(StateFoo));

}


void Engine::ResetStateFoo() {
  m_StateFoo->Reset();
}


int Engine::isExtensionSupported(const char *extension) {
  const GLubyte *extensions = NULL;
  const GLubyte *start;
  GLubyte *where, *terminator;
  // Extension names should not have spaces.
  where = (GLubyte *) strchr(extension, ' ');
  if (where || *extension == '\0')
    return 0;
  extensions = glGetString(GL_EXTENSIONS);
  LOGV("%s\n", extensions);
  // It takes a bit of care to be fool-proof about parsing the OpenGL extensions string. Don't be fooled by sub-strings, etc.
  start = extensions;
  for (;;) {
    where = (GLubyte *) strstr((const char *) start, extension);
    if (!where)
      break;
    terminator = where + strlen(extension);
    if (where == start || *(where - 1) == ' ')
      if (*terminator == ' ' || *terminator == '\0')
        return 1;
    start = terminator;
  }
  return 0;
}


void Engine::DrawScreen(float rotation) {
  Run();
	if (m_IsSceneBuilt && m_IsScreenResized) {

#ifdef USE_GLES2

    glUseProgram(program);

    float a = (-m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
    float b = (m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
    float c = (-m_ScreenHalfHeight) * m_Zoom;
    float d = m_ScreenHalfHeight * m_Zoom;
    float e = 1.0;
    float f = -1.0;

    ortho(ProjectionMatrix, a, b, c, d, e, f);

    glUniformMatrix4fv(ModelViewProjectionMatrix_location, 1, GL_FALSE, ProjectionMatrix);

#endif

    // clear the frame, this is required for optimal performance, which I think is odd
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

#ifdef USE_GLES2
#else
    glLoadIdentity();
#endif
    
    // Render 3D
    //GLU_PERSPECTIVE(m_Fov, (float)m_ScreenWidth / (float)m_ScreenHeight, 1.0, 1000.0);
    //glueLookAt(m_CameraPosition[0], m_CameraPosition[1], m_CameraPosition[2], m_CameraTarget[0], m_CameraTarget[1], m_CameraTarget[2], 0.0, 1.0, 0.0);
    //RenderModelPhase();

    // Reset for switch to 2D
    //glLoadIdentity();
    
    // Render 2D

#ifdef USE_GLES2
#else
    glOrthof((-m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (m_ScreenHalfHeight*m_ScreenAspect) * m_Zoom, (-m_ScreenHalfHeight) * m_Zoom, m_ScreenHalfHeight * m_Zoom, 1.0f, -1.0f);
#endif

    RenderSpritePhase();
	} else {
    ResizeScreen(m_ScreenWidth, m_ScreenHeight);
  }
}


int Engine::Run() {
	timeval tim;
  gettimeofday(&tim, NULL);
  t2=tim.tv_sec+(tim.tv_usec/1000000.0);
  float step = t2 - t1;
  gettimeofday(&tim, NULL);
  t1=tim.tv_sec+(tim.tv_usec/1000000.0);
  int times = 1;
  for (int i=0; i<times; i++) {
    if (m_GameState > 1) {
      //paused
    } else {
      //m_DeltaTime = step / 1.0;
      //m_SimulationTime += (step);
      //for (int j=0; j<1; j++) {
      m_DeltaTime = step;
      m_SimulationTime += m_DeltaTime;
        if (Active()) {
          Simulate();
        }
      //}
    }
  }

  m_IsSceneBuilt = true;
	return m_GameState;
}


void Engine::PauseSimulation() {
	m_GameState = 2;
}


void Engine::StopSimulation() {
  m_IsSceneBuilt = false;
  m_GameState = -1;
}


void Engine::StartSimulation() {
  if (m_GameState == 2) { // TODO: state machine
    timeval tim;
    gettimeofday(&tim, NULL);
    t1=tim.tv_sec+(tim.tv_usec/1000000.0);
    m_GameState = 1;
  }
}


void Engine::DoAudio(short buffer[], int size) {
  memset(buffer, 0, size);
  if (Active() && m_IsPushingAudio) {
    int read = ModPlug_Read(m_Sounds[m_CurrentSound], buffer, size);
    if (read == 0) {
      ModPlug_Seek(m_Sounds[m_CurrentSound], 0);
    }
  }
}


void Engine::RenderModelRange(unsigned int s, unsigned int e, foofoo *batch_foo) {
	for (unsigned int i=s; i<e; i++) {
		m_Models[i]->Render(m_StateFoo, batch_foo);
	}
}


void Engine::RenderSpriteRange(unsigned int s, unsigned int e, foofoo *batch_foo) {
	for (unsigned int i=s; i<e; i++) {
		m_AtlasSprites[i]->Render(m_StateFoo, batch_foo);
	}
}


void Engine::ResizeScreen(int width, int height) {
  m_ScreenWidth = width;
  m_ScreenHeight = height;
  m_ScreenAspect = (float)m_ScreenWidth / (float)m_ScreenHeight;
  m_ScreenHalfHeight = (float)m_ScreenHeight * 0.5;
  glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  m_IsScreenResized = true;
}

#ifndef USE_GLES2

// This is a modified version of the function of the same name from 
// the Mesa3D project ( http://mesa3d.org/ ), which is  licensed
// under the MIT license, which allows use, modification, and 
// redistribution
void Engine::glueLookAt(GLfloat eyex, GLfloat eyey, GLfloat eyez, GLfloat centerx, GLfloat centery, GLfloat centerz, GLfloat upx, GLfloat upy, GLfloat upz)
{
	GLfloat m[16];
	GLfloat x[3], y[3], z[3];
	GLfloat mag;
	
	// Make rotation matrix
	
	// Z vector
	z[0] = eyex - centerx;
	z[1] = eyey - centery;
	z[2] = eyez - centerz;
	mag = sqrtf(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
	if (mag) {			// mpichler, 19950515
		z[0] /= mag;
		z[1] /= mag;
		z[2] /= mag;
	}
	
	// Y vector
	y[0] = upx;
	y[1] = upy;
	y[2] = upz;
	
	// X vector = Y cross Z
	x[0] = y[1] * z[2] - y[2] * z[1];
	x[1] = -y[0] * z[2] + y[2] * z[0];
	x[2] = y[0] * z[1] - y[1] * z[0];
	
	// Recompute Y = Z cross X
	y[0] = z[1] * x[2] - z[2] * x[1];
	y[1] = -z[0] * x[2] + z[2] * x[0];
	y[2] = z[0] * x[1] - z[1] * x[0];
	
	// mpichler, 19950515
	// cross product gives area of parallelogram, which is < 1.0 for
	// non-perpendicular unit-length vectors; so normalize x, y here
	
	mag = sqrtf(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
	if (mag) {
		x[0] /= mag;
		x[1] /= mag;
		x[2] /= mag;
	}
	
	mag = sqrtf(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
	if (mag) {
		y[0] /= mag;
		y[1] /= mag;
		y[2] /= mag;
	}
	
#define M(row,col)  m[col*4+row]
	M(0, 0) = x[0];
	M(0, 1) = x[1];
	M(0, 2) = x[2];
	M(0, 3) = 0.0;
	M(1, 0) = y[0];
	M(1, 1) = y[1];
	M(1, 2) = y[2];
	M(1, 3) = 0.0;
	M(2, 0) = z[0];
	M(2, 1) = z[1];
	M(2, 2) = z[2];
	M(2, 3) = 0.0;
	M(3, 0) = 0.0;
	M(3, 1) = 0.0;
	M(3, 2) = 0.0;
	M(3, 3) = 1.0;
#undef M
	glMultMatrixf(m);
	
	// Translate Eye to Origin

	glTranslatef(-eyex, -eyey, -eyez);
	
}


void Engine::gluePerspective(float fovy, float aspect,
                           float zNear, float zFar)
{
  GLfloat xmin, xmax, ymin, ymax;

  ymax = zNear * (GLfloat)tan(fovy * M_PI / 360);
  ymin = -ymax;
  xmin = ymin * aspect;
  xmax = ymax * aspect;

  glFrustumx((GLfixed)(xmin * 65536), (GLfixed)(xmax * 65536),
             (GLfixed)(ymin * 65536), (GLfixed)(ymax * 65536),
             (GLfixed)(zNear * 65536), (GLfixed)(zFar * 65536));

}

#endif


void Engine::PushBackFileHandle(int collection, FILE *file, unsigned int offset, unsigned int length) {
  FileHandle *fh = new FileHandle;
  fh->fp = file;
  fh->off = offset;
  fh->len = length;
  std::vector<FileHandle *> *collectionHandle = NULL;
  switch(collection) {
    case MODELS:
      collectionHandle = &models;
      break;
    case SOUNDS:
      collectionHandle = &sounds;
      break;
    case TEXTURES:
      collectionHandle = &textures;
      break;
    case LEVELS:
      collectionHandle = &levels;
      break;
    default:
      break;
  }

  if (collectionHandle != NULL) {
    collectionHandle->push_back(fh);
  } else {
    LOGV("this should not happen\n");
    assert(0);
  }
}


void Engine::Start(int i, int w, int h) {
  if (games.size() == 0) {
    games.push_back(new GameImpl<MainMenu>);
    games.push_back(new GameImpl<SuperStarShooter>);
    games.push_back(new GameImpl<RadiantFireEightSixOne>);
    games.push_back(new GameImpl<SpaceShipDown>);
    games.push_back(new GameImpl<AncientDawn>);
  }

  if (m_CurrentGame) {
    m_CurrentGame->StopSimulation();
    delete m_CurrentGame;
  }

  m_CurrentGame = (Engine *)games.at(i)->allocate(w, h, textures, models, levels, sounds);
  m_CurrentGame->StartSimulation();
  
}


void Engine::CurrentGameDestroyFoos() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DestroyFoos();
  } else {
    LOGV("current game is not set to destroy foos\n\n");
  }
}


void Engine::CurrentGameCreateFoos() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->CreateFoos();
  } else {
    LOGV("current game is not set to create foos\n\n");
  }
}


void Engine::CurrentGamePause() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->PauseSimulation();
  } else {
    LOGV("current game is not set to pause\n\n");
  }
}


void Engine::CurrentGameHit(float x, float y, int hitState) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->Hit(x, y, hitState);
  } else {
    LOGV("current game is not set to hit\n\n");
  }
}


void Engine::CurrentGameResizeScreen(int width, int height) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->ResizeScreen(width, height);
  } else {
    LOGV("current game is not set to resize\n\n");
  }
}


void Engine::CurrentGameDrawScreen(float rotation) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DrawScreen(rotation);
  } else {
    LOGV("CurrentGameDrawScreen without m_CurrentGame set\n");
  }
}


void Engine::CurrentGameDoAudio(short buffer[], int bytes) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DoAudio(buffer, bytes);
  } else {
    LOGV("CurrentGameDoAudio without m_CurrentGame set\n");
  }
}


bool Engine::CurrentGame() {
  if (m_CurrentGame != NULL) {
    return true;
  } else {
    return false;
  }
}


void Engine::CurrentGameStart() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->StartSimulation();
  } else {
    LOGV("current game not set to start\n");
  }
}


bool Engine::Active() {
  return (m_GameState == 1);
}


void Engine::LoadSound(int i) {
  void *buffer = (void *)malloc(sizeof(char) * m_SoundFileHandles->at(i)->len);
  fseek(m_SoundFileHandles->at(i)->fp, m_SoundFileHandles->at(i)->off, SEEK_SET);
  size_t r = fread(buffer, 1, m_SoundFileHandles->at(i)->len, m_SoundFileHandles->at(i)->fp);
  if (r > 0) {
    //LOGV("123--- %d\n", m_SoundFileHandles->at(i)->len);
    m_Sounds.push_back(ModPlug_Load(buffer, m_SoundFileHandles->at(i)->len));
  }
  free(buffer);
  m_IsPushingAudio = true;
}


void Engine::LoadModel(int i, int s, int e) {
  //aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph  cause memoryleak
  Assimp::Importer m_Importer;
	m_Importer.SetIOHandler(new FooSystem(*m_ModelFileHandles));
	int m_PostProcessFlags = aiProcess_FlipUVs | aiProcess_ImproveCacheLocality;
	char path[128];
	snprintf(path, sizeof(s), "%d", i);
	m_Importer.ReadFile(path, m_PostProcessFlags);
  const aiScene *scene = m_Importer.GetScene();
	m_FooFoos.push_back(Model::GetFoo(scene, s, e));
	m_Importer.FreeScene();	
}


void Engine::LoadTexture(int i) {
LOGV("111\n");
  png_t tex;
  unsigned char* data;
  GLuint textureHandle;

LOGV("222\n");

  png_init(0, 0);
LOGV("333\n");
  //rewind(m_TextureFileHandles->at(i)->fp);
  fseek(m_TextureFileHandles->at(i)->fp, m_TextureFileHandles->at(i)->off, 0);
LOGV("444\n");
  png_open_read(&tex, 0, m_TextureFileHandles->at(i)->fp);
LOGV("555\n");
  data = (unsigned char*)malloc(tex.width * tex.height * tex.bpp);
LOGV("666 %d %d %d\n", tex.width, tex.height, tex.bpp);
  png_get_data(&tex, data);
LOGV("777\n");

  glGenTextures(1, &textureHandle);
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //TODO: investigate pixel swizzling
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);

LOGV("888\n");

  free(data);

  m_Textures.push_back(textureHandle);
LOGV("999\n");
}

/*
- (GLuint)loadTexture2:(UIImage *)image {
  unsigned int* inPixel32;
  unsigned short* outPixel16;
	GLuint text = 0;
  CGImageRef textureImage = image.CGImage;
  CGColorSpaceRef colorSpace;
  GLuint textureWidth = CGImageGetWidth(textureImage);
  GLuint textureHeight = CGImageGetHeight(textureImage);
  colorSpace = CGColorSpaceCreateDeviceRGB();
  
  void *textureData = malloc(textureWidth * textureHeight * sizeof(unsigned int));
  void *tempData = malloc(textureHeight * textureWidth * sizeof(unsigned short));

  inPixel32 = (unsigned int *)textureData;
  outPixel16 = (unsigned short *)tempData;
  
  //swizzle in all the bits for the to-be-created bitmap, because CGContextDrawImage/CGBitmapContextCreate doesnt like rgba
  for(int i=0; i < textureWidth*textureHeight; ++i) {
    inPixel32[i] = 0;
  }

  CGContextRef textureContext = CGBitmapContextCreate(textureData, textureWidth, textureHeight, 8, sizeof(int) * textureWidth, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big); //
  CGContextDrawImage(textureContext, CGRectMake(0, 0, textureWidth, textureHeight), textureImage);

  //Convert "RRRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA" to "RRRRGGGGBBBBAAAA"
  for (int i=0; i<(textureHeight * textureWidth); i++) {
    unsigned int inP = ((unsigned int *)textureData)[i];
    outPixel16[i] = ((((inP >> 0) & 0xFF) >> 4) << 12) | ((((inP >> 8) & 0xFF) >> 4) << 8) | ((((inP >> 16) & 0xFF) >> 4) << 4) | ((((inP >> 24) & 0xFF) >> 4) << 0);
  }
  
  free(textureData);
  CGContextRelease(textureContext);
  CGColorSpaceRelease(colorSpace);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &text);
	glBindTexture(GL_TEXTURE_2D, text);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, tempData);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
  free(tempData);
	return text;
}
*/


#ifndef USE_GLES2

void Engine::CheckGL(const char *s) {
  // normally (when no error) just return
  const int lastGlError = glGetError();
  if (lastGlError == GL_NO_ERROR) return;
  
  LOGV("\n%s caused\n", s);
  switch (lastGlError)
  {
    case GL_INVALID_ENUM:      LOGV("GL_INVALID_ENUM\n\n");      break;
    case GL_INVALID_VALUE:     LOGV("GL_INVALID_VALUE\n\n");     break;
    case GL_INVALID_OPERATION: LOGV("GL_INVALID_OPERATION\n\n"); break;
    case GL_STACK_OVERFLOW:    LOGV("GL_STACK_OVERFLOW\n\n");    break;
    case GL_STACK_UNDERFLOW:   LOGV("GL_STACK_UNDERFLOW\n\n");   break;
    case GL_OUT_OF_MEMORY:     LOGV("GL_OUT_OF_MEMORY\n\n");     break;
  }
}

#endif
