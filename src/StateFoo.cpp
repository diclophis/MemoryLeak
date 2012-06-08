// Jon Bardin GPL


#include "MemoryLeak.h"


StateFoo::StateFoo() {
  g_lastTexture = -1;
  g_lastElementBuffer = -1;
  g_lastInterleavedBuffer = -1;
  g_lastVertexArrayObject = -1;
  m_EnabledStates = false;
  m_LastBufferIndex = 0;


}


StateFoo::~StateFoo() {
}


void StateFoo::Reset(GLuint program) {

  g_lastTexture = -1;
  g_lastElementBuffer = -1;
  g_lastInterleavedBuffer = -1;
  g_lastVertexArrayObject = -1;
  m_EnabledStates = false;
  m_LastBufferIndex = 0;

#ifdef USE_GLES2
  
  g_PositionAttribute = glGetAttribLocation(program, "Position");
  g_TextureAttribute = glGetAttribLocation(program, "InCoord");
  
#endif

}