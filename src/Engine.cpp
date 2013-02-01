// Jon Bardin GPL 2011


#include "MemoryLeak.h"


//#include "SuperStarShooter.h"

/*
#include <stdlib.h>

#define EXPECTED_BYTES 5


static int reformat_null(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_null(g);
    LOGV("reformat_null\n");
    return 1;
}

static int reformat_boolean(void * ctx, int boolean)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_bool(g, boolean);
    LOGV("reformat_boolean %d\n", boolean);
    return 1;
}

static int reformat_number(void * ctx, const char * s, size_t l)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_number(g, s, l);
    //LOGV("reformat_number %f\n", strtof(s, (char **)s+l));
    LOGV("reformat_number\n");
    return 1;
}

static int reformat_string(void * ctx, const unsigned char * stringVal,
                           size_t stringLen)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
    //LOGV("reformat_string %s %d\n", stringVal, stringLen);
    LOGV("reformat_string\n");
    return 1;
}

static int reformat_map_key(void * ctx, const unsigned char * stringVal,
                            size_t stringLen)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
    //LOGV("reformat_map_key %s\n", stringVal);
    LOGV("reformat_map_key\n");
    return 1;
}

static int reformat_start_map(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_map_open(g);
    LOGV("reformat_start_map\n");
    return 1;
}


static int reformat_end_map(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_map_close(g);
    LOGV("reformat_end_map\n");
    return 1;
}

static int reformat_start_array(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_array_open(g);
    LOGV("reformat_start_array\n");
    return 1;
}

static int reformat_end_array(void * ctx)
{
    //yajl_gen g = (yajl_gen) ctx;
    //return yajl_gen_status_ok == yajl_gen_array_close(g);
    LOGV("reformat_end_array\n");
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

unsigned int Engine::get_all_buf(int sock, const unsigned char* output, unsigned int maxsize)
{
  int bytes;

  //read the socket to see how many bytes are there
  if (ioctl(sock, FIONREAD, &bytes)) {
    LOGV("wtf1\n");
    perror("foo\n");
    return 0;
  }

  // how many bytes are available
  if (bytes == 0) {
    //LOGV("wtf\n");
    return 0;
  }

  LOGV("reading: %d of %d\n", maxsize, bytes);

  int n;
  errno = 0;
  n = recv(sock, (void *)output, maxsize, 0);

  if (n>0) {
  } else {
    LOGV("error in get_all_buf!");
  }

  return n;
}


void Engine::StopNetwork() {
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
}


void Engine::iter(void *arg) {

  yajl_status stat;

  int n = get_all_buf(SocketFD, out, 10);

  if (n > 0) {
    out[n] = '\0';
    LOGV("read! n=%d out=%s\n", n, out);
    stat = yajl_parse(hand, out, n * sizeof(unsigned char));
    if (stat == yajl_status_ok) {
    } else {
      unsigned char *str = yajl_get_error(hand, 1, out, 1023);
      fprintf(stderr, "%s", (const char *) str);
      yajl_free_error(hand, str);
    }
  }

  char payload[4] = "[1]";
  ssize_t sent = send(SocketFD, payload, 3, 0); //MSG_DONTWAIT
  if (sent > 0) {
    //LOGV("wtf11111 %d payload-sent: %d\n", SocketFD, sent);
  } else {
    //LOGV("send failed\n");
  }
}

int Engine::ConnectNetwork(void) {
  struct sockaddr_in stSockAddr;
  int Res = 0;
  done = 0;
  SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (-1 == SocketFD)
  {
    LOGV("cannot create socket");
    return 1;
  }

  memset(&stSockAddr, 0, sizeof(stSockAddr));

  stSockAddr.sin_family = AF_INET;
  stSockAddr.sin_port = htons(7001);

  //struct hostent *host0 = gethostbyname("test.com"); // increment hostname counter to check for possible but at 0,0 not differentiating low/high
  //struct hostent *host = gethostbyname("localhost");
  struct hostent *host = gethostbyname("emscripten.risingcode.com");
  char **addr_list = host->h_addr_list;
  int *addr = (int*)*addr_list;
  LOGV("raw addr: %d\n", *addr);
  char name[INET_ADDRSTRLEN];
  if (!inet_ntop(AF_INET, addr, name, sizeof(name))) {
    LOGV("could not figure out name\n");
    return 1;
  }
  LOGV("localhost has 'ip' of %s\n", name);

  Res = inet_pton(AF_INET, name, &stSockAddr.sin_addr);

  if (0 > Res) {
    LOGV("error: first parameter is not a valid address family");
    close(SocketFD);
    return 1;
  } else if (0 == Res) {
    LOGV("char string (second parameter does not contain valid ipaddress)");
    close(SocketFD);
    return 1;
  }

  if (-1 == connect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof(stSockAddr))) {
    LOGV("connect failed");
    close(SocketFD);
    return 1;
  }

  int set = 1;
  setsockopt(SocketFD, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, sizeof(int));

  char magic[1];
  magic[0] = '{';
  ssize_t sent = send(SocketFD, magic, 1, 0); //MSG_DONTWAIT
  LOGV("wtf11111 %d magic-sent: %d\n", SocketFD, sent);

  out = (unsigned char *) malloc(sizeof(unsigned char) * 1024);

  void *g = NULL;

  hand = yajl_alloc(&callbacks, NULL, (void *) g);
  yajl_config(hand, yajl_allow_comments, 1); // allow json comments
  yajl_config(hand, yajl_dont_validate_strings, 1); // dont validate strings

  return 0;
}

*/

static std::vector<Game *> games;
static Engine *m_CurrentGame;
static std::vector<FileHandle *> models;
static std::vector<FileHandle *> sounds;
static std::vector<FileHandle *> textures;
static std::vector<FileHandle *> levels;
static bool m_WarnedAboutGameFailure = false;


static const char vertex_shader[] =
"#ifdef GL_ES\n"
"precision lowp float;\n"
"#endif\n"
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
"precision lowp float;\n"
"#endif\n"
"varying vec2 OutCoord;\n"
"uniform sampler2D Sampler;\n"
"void main()\n"
"{\n"
"gl_FragColor = texture2D(Sampler, OutCoord);\n"
"}\n";
//TODO: https://github.com/evanw/glfx.js/blob/master/src/filters/adjust/vignette.js


void Engine::glTranslatef(float tx, float ty, float tz) {
  if (tx != ltx || ty != lty || tz != ltz) {
    ProjectionMatrix[12] += (ProjectionMatrix[0] * tx + ProjectionMatrix[4] * ty + ProjectionMatrix[8] * tz);
    ProjectionMatrix[13] += (ProjectionMatrix[1] * tx + ProjectionMatrix[5] * ty + ProjectionMatrix[9] * tz);
    ProjectionMatrix[14] += (ProjectionMatrix[2] * tx + ProjectionMatrix[6] * ty + ProjectionMatrix[10] * tz);
    ProjectionMatrix[15] += (ProjectionMatrix[3] * tx + ProjectionMatrix[7] * ty + ProjectionMatrix[11] * tz);
    glUniformMatrix4fv(m_StateFoo->ModelViewProjectionMatrix_location, 1, GL_FALSE, ProjectionMatrix);
  }
    
  ltx = tx;
  lty = ty;
  ltz = tz;
}


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

  //ClearModels();

  for (std::vector<ModPlugFile *>::iterator i = m_Sounds.begin(); i != m_Sounds.end(); ++i) {
    ModPlug_Unload(*i);
  }
  m_Sounds.clear();


  free(m_StateFoo);

  glDetachShader(program, v);
  glDetachShader(program, f);
  glDeleteShader(v);
  glDeleteShader(f);
  glDeleteProgram(program);
}


void Engine::ClearSprites() {
  for (std::vector<SpriteGun *>::iterator i = m_AtlasSprites.begin(); i != m_AtlasSprites.end(); ++i) {
    delete *i;
  }
  m_AtlasSprites.clear();
  m_SpriteCount = 0;
}


Engine::Engine(int w, int h, std::vector<FileHandle *> &t, std::vector<FileHandle *> &m, std::vector<FileHandle *> &l, std::vector<FileHandle *> &s) : m_ScreenWidth(w), m_ScreenHeight(h), m_TextureFileHandles(&t), m_ModelFileHandles(&m), m_LevelFileHandles(&l), m_SoundFileHandles(&s) {

  std::sort(m_TextureFileHandles->begin(), m_TextureFileHandles->end(), CompareFileHandles);
  std::sort(m_ModelFileHandles->begin(), m_ModelFileHandles->end(), CompareFileHandles);
  std::sort(m_LevelFileHandles->begin(), m_LevelFileHandles->end(), CompareFileHandles);
  std::sort(m_SoundFileHandles->begin(), m_SoundFileHandles->end(), CompareFileHandles);

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
  LOGV("engine program ID: %d\n", program);
  glAttachShader(program, v);
  glAttachShader(program, f);

  m_StateFoo = new StateFoo(program); //(StateFoo *)malloc(1 * sizeof(StateFoo));

  ltx = lty = ltz = 0;

  //ConnectNetwork();
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
  if (where || *extension == '\0') {
    return 0;
  }
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

    if (!m_StateFoo->m_EnabledStates) {
      m_StateFoo->Link();
    }

    // clear the frame, this is required for optimal performance, which I think is odd
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        
    float a = (-m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
    float b = (m_ScreenHalfHeight * m_ScreenAspect) * m_Zoom;
    float c = (-m_ScreenHalfHeight) * m_Zoom;
    float d = m_ScreenHalfHeight * m_Zoom;
    float e = 1.0;
    float f = -1.0;

    ortho(ProjectionMatrix, (a), (b), (c), (d), (e), (f));

    //glUniformMatrix4fv(m_StateFoo->ModelViewProjectionMatrix_location, 1, GL_FALSE, ProjectionMatrix);

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
      float steps = 1.0; //4 for overdrive
      m_DeltaTime = step / steps;
      for (int j=0; j<(int)steps; j++) {
        if (Active()) {
          m_SimulationTime += m_DeltaTime;
          Simulate();
          //iter(NULL);
        }
      }
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


void Engine::RenderSpriteRange(unsigned int s, unsigned int e, foofoo *batch_foo, float offsetX, float offsetY) {
	for (unsigned int i=s; i<e; i++) {
		m_AtlasSprites[i]->Render(m_StateFoo, batch_foo, offsetX, offsetY);
	}
}


void Engine::ResizeScreen(int width, int height) {
  m_ScreenWidth = width;
  m_ScreenHeight = height;
  m_ScreenAspect = (float)m_ScreenWidth / (float)m_ScreenHeight;
  m_ScreenHalfHeight = (float)m_ScreenHeight * 0.5;
  glViewport(0, 0, m_ScreenWidth, m_ScreenHeight);
  //glClearColor(0.07, 0.26, 0.46, 0.0);
  //glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearColor(1.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  m_IsScreenResized = true;
}


bool Engine::CompareFileHandles(void *pa, void *pb) {
  FileHandle *a = (FileHandle *)pa;
  FileHandle *b = (FileHandle *)pb;
  return strcmp(a->name, b->name) < 0;
}


void Engine::PushBackFileHandle(int collection, FILE *file, unsigned int offset, unsigned int length, const char *name) {
  FileHandle *fh = new FileHandle;
  fh->fp = file;
  fh->off = offset;
  fh->len = length;
  fh->name = name;
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
    games.push_back(new GameImpl<SuperStarShooter>);
  }

  if (m_CurrentGame) {
    m_CurrentGame->StopSimulation();
    delete m_CurrentGame;
  }

  try {
    m_CurrentGame = (Engine *)games.at(i)->allocate(w, h, textures, models, levels, sounds);
    m_CurrentGame->StartSimulation();
  } catch (std::exception& e) {
    LOGV("Exception is: %s", e.what());
    WarnAboutGameFailure("exception in construct\n");
  }
}


void Engine::CurrentGameDestroyFoos() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DestroyFoos();
  } else {
    WarnAboutGameFailure("current game is not set to destroy foos\n\n");
  }
}


void Engine::CurrentGameCreateFoos() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->CreateFoos();
  } else {
    WarnAboutGameFailure("current game is not set to create foos\n\n");
  }
}


void Engine::CurrentGamePause() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->PauseSimulation();
  } else {
    WarnAboutGameFailure("current game is not set to pause\n\n");
  }
}


void Engine::CurrentGameHit(float x, float y, int hitState) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->Hit(x, y, hitState);
  } else {
    WarnAboutGameFailure("current game is not set to hit\n\n");
  }
}


void Engine::CurrentGameResizeScreen(int width, int height) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->ResizeScreen(width, height);
  } else {
    WarnAboutGameFailure("current game is not set to resize\n\n");
  }
}


void Engine::CurrentGameDrawScreen(float rotation) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DrawScreen(rotation);
  } else {
    WarnAboutGameFailure("CurrentGameDrawScreen without m_CurrentGame set\n");
  }
}


void Engine::CurrentGameDoAudio(short buffer[], int bytes) {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->DoAudio(buffer, bytes);
  } else {
    WarnAboutGameFailure("CurrentGameDoAudio without m_CurrentGame set\n");
  }
}


bool Engine::CurrentGame() {
  if (m_CurrentGame != NULL) {
    return true;
  } else {
    return false;
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
    m_Sounds.push_back(ModPlug_Load(buffer, m_SoundFileHandles->at(i)->len));
  }
  free(buffer);
  m_IsPushingAudio = true;
}


void Engine::LoadTexture(int i) {
  if (i >= m_TextureFileHandles->size()) {
    LOGV("missing texture: %d\n", i);
  }
  png_t tex;
  unsigned char* data;
  GLuint textureHandle;

  png_init(0, 0);
  //rewind(m_TextureFileHandles->at(i)->fp);
  fseek(m_TextureFileHandles->at(i)->fp, m_TextureFileHandles->at(i)->off, 0);
  png_open_read(&tex, 0, m_TextureFileHandles->at(i)->fp);
  data = (unsigned char*)malloc(tex.width * tex.height * tex.bpp);
  for(int i=0; i < tex.width*tex.height*tex.bpp; ++i) {
    data[i] = 0;
  }
  png_get_data(&tex, data);

  unsigned int* inPixel32;
  unsigned short* outPixel16;

  void *textureData = data;
  void *tempData = malloc(tex.height * tex.width * sizeof(unsigned short));

  inPixel32 = (unsigned int *)textureData;
  outPixel16 = (unsigned short *)tempData;

  //Convert "RRRRRRRRRGGGGGGGGBBBBBBBBAAAAAAAA" to "RRRRGGGGBBBBAAAA"
  for (int i=0; i<(tex.height * tex.width); i++) {
    unsigned int inP = ((unsigned int *)textureData)[i];
    outPixel16[i] = ((((inP >> 0) & 0xFF) >> 4) << 12) | ((((inP >> 8) & 0xFF) >> 4) << 8) | ((((inP >> 16) & 0xFF) >> 4) << 4) | ((((inP >> 24) & 0xFF) >> 4) << 0);
  }

  glGenTextures(1, &textureHandle);
  glBindTexture(GL_TEXTURE_2D, textureHandle);


  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  bool useSwizzledBits = true;
  if (useSwizzledBits) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, tempData);
  } else {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  }
  
  bool generateMipMap = true;
  if (generateMipMap) {
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  free(data);
  free(tempData);

  m_Textures.push_back(textureHandle);
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
    case GL_OUT_OF_MEMORY:     LOGV("GL_OUT_OF_MEMORY\n\n");     break;
  }
}


// Creates an identity 4x4 matrix.
// @param m the matrix make an identity matrix
void Engine::identity(GLfloat *m) {
  GLfloat t[16] = {
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0,
  };
  
  memcpy(m, t, sizeof(t));
}


void Engine::ortho(GLfloat *m, GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearZ, GLfloat farZ) {
  
  GLfloat deltaX = right - left;
  GLfloat deltaY = top - bottom;
  GLfloat deltaZ = farZ - nearZ;
  
  GLfloat tmp[16];
  identity(tmp);
  
  if ((deltaX == 0) || (deltaY == 0) || (deltaZ == 0)) {
    LOGV("Invalid ortho");
    return;
  }
  
  tmp[0] = 2 / deltaX;
  tmp[12] = -(right + left) / deltaX;
  tmp[5] = 2 / deltaY;
  tmp[13] = -(top + bottom) / deltaY;
  tmp[10] = -2 / deltaZ;
  tmp[14] = -(nearZ + farZ) / deltaZ;
  
  memcpy(m, tmp, sizeof(tmp));
  
}


void Engine::WarnAboutGameFailure(const char *s) {
  if (!m_WarnedAboutGameFailure) {
    m_WarnedAboutGameFailure = true;
    LOGV("Game failed: %s", s);
  }
}


void Engine::CurrentGameStart() {
  if (m_CurrentGame != NULL) {
    m_CurrentGame->StartSimulation();
  } else {
    LOGV("current game not set to start\n");
  }
}
