// Jon Bardin GPL


#include "MemoryLeak.h"


StateFoo::StateFoo(GLuint program) {
  m_Program = program;
  g_lastTexture = -1;
  g_lastElementBuffer = -1;
  g_lastInterleavedBuffer = -1;
  g_lastVertexArrayObject = -1;
  m_EnabledStates = false;
  m_LastBufferIndex = 0;


}


StateFoo::~StateFoo() {
}


void StateFoo::Link() {
#ifdef USE_GLES2

  char msg[512];
  glLinkProgram(m_Program);
  glGetProgramInfoLog(m_Program, sizeof msg, NULL, msg);
  LOGV("info: %s\n", msg);

  glUseProgram(m_Program);

  g_PositionAttribute = glGetAttribLocation(m_Program, "Position");
  g_TextureAttribute = glGetAttribLocation(m_Program, "InCoord");

  // Get the locations of the uniforms so we can access them
  ModelViewProjectionMatrix_location = glGetUniformLocation(m_Program, "ModelViewProjectionMatrix");

  
#endif
}


void StateFoo::Reset() {

  g_lastTexture = -1;
  g_lastElementBuffer = -1;
  g_lastInterleavedBuffer = -1;
  g_lastVertexArrayObject = -1;
  m_EnabledStates = false;
  m_LastBufferIndex = 0;


}
