// Jon Bardin GPL


class StateFoo {

public:

  GLuint g_lastTexture;
  GLuint g_lastElementBuffer;
  GLuint g_lastInterleavedBuffer;
  GLuint g_lastVertexArrayObject;
  GLuint ModelViewProjectionMatrix_location;
  int m_LastBufferIndex;
  bool m_EnabledStates;

  GLuint g_PositionAttribute;
  GLuint g_TextureAttribute;
  GLuint m_Program;
  StateFoo(GLuint program);
  ~StateFoo();
  void Reset();
  void Link();

};
