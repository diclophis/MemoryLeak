// Jon Bardin GPL

#include "MemoryLeak.h"

RenderTexture::RenderTexture(int width, int height) {
  name = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, &oldRBO);
  //glEnable(GL_TEXTURE_2D);
  glGenTextures(1, &name);
  LOGV("generated: %d\n", name);
  if (name == 0) {
    LOGV("INVALID GL_CONTEXT CANT MAKE TEXTURE\n");
    assert(name);
  }
  glBindTexture(GL_TEXTURE_2D, name);

  // generate FBO
  glGenFramebuffersOES(1, &fbo);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);

  glGenRenderbuffersOES(1, &rbo);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, rbo);

  // associate texture with FBO
  Engine::CheckGL("Prob in RenderTexture in T");

  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  //glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  Engine::CheckGL("wtf in RenderREnder::RenderTexture\n");

  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, rbo);
  Engine::CheckGL("111 in RenderREnder::RenderTexture\n");

  glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, name, 0);
  Engine::CheckGL("222 in RenderREnder::RenderTexture\n");


  Engine::CheckGL("Prob in RenderTexture in T2");
  // check if it worked (probably worth doing :) )
  GLuint status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
  if (status != GL_FRAMEBUFFER_COMPLETE_OES) {
    LOGV("INVALID GL CONTEXT CANT MAKE BUFFER\n");
    assert(false);
  }
  //glBindTexture(GL_TEXTURE_2D, 0);
  //glDisable(GL_TEXTURE_2D);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRBO);
  //glFinish();
}


RenderTexture::~RenderTexture() {
  LOGV("delete text %d\n", name);
  glDeleteTextures(1, &name);
  glDeleteBuffers(1, &fbo);
  glDeleteBuffers(1, &rbo);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glFlush();
  glFinish();
}


void RenderTexture::Begin() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldRBO);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);

  //glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  //glEnable(GL_TEXTURE_2D);
  //glBindTexture(GL_TEXTURE_2D, name);

  /*
    GLfloat texCoord[] = { 0.0f, 1.0f, 
                 0.0f, 0.0f,
                 1.0f, 0.0f,
                 1.0f, 1.0f };

    glTexCoordPointer(2, GL_FLOAT, 0, texCoord);
  */

  glBindRenderbufferOES(GL_RENDERBUFFER_OES, rbo);

/*
      texture[0] = tx;
      texture[1] = (ty + th);
      texture[2] = tx + tw;
      texture[3] = (ty + th);
      texture[4] = tx + tw;
      texture[5] = ty;
      texture[6] = tx;
      texture[7] = ty;

      glTexCoordPointer(2, GL_FLOAT, 0, texture);
*/

  //glBindTexture(GL_TEXTURE_2D, name);
  //glClearColor(1.0, 1.0, 1.0, 1.0);
  //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  //glFinish();

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glFlush();
  glFinish();
}


void RenderTexture::End() {
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  //glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  //glDisable(GL_TEXTURE_2D);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRBO);
  //glBindTexture(GL_TEXTURE_2D, 0);
  //glclearcolor(1.0, 1.0, 1.0, 1.0);
  //glclear(gl_depth_buffer_bit | gl_color_buffer_bit);
  //glfinish();

  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  glFlush();
  glFinish();
}
