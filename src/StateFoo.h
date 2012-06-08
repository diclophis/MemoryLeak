// Jon Bardin GPL


class StateFoo {

public:

  GLuint g_lastTexture;
  GLuint g_lastElementBuffer;
  GLuint g_lastInterleavedBuffer;
  GLuint g_lastVertexArrayObject;
  int m_LastBufferIndex;
  bool m_EnabledStates;

  GLuint g_PositionAttribute;
  GLuint g_TextureAttribute;

  StateFoo();
  ~StateFoo();
  void Reset(GLuint program);

};
