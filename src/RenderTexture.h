// Jon Bardin GPL


class RenderTexture {


public:
	RenderTexture(int w, int h);
	~RenderTexture();

  GLuint fbo;
  GLuint rbo;

  GLint oldFBO;
  GLint oldRBO;

  GLuint name;
  
  GLenum pixelFormat;

  void Begin();
  void End();

};
