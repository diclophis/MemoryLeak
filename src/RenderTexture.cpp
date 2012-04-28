// Jon Bardin GPL

#include "MemoryLeak.h"

RenderTexture::RenderTexture(int width, int height) {
  //glFinish();
  //glEnable(GL_TEXTURE_2D);

  name = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, &oldRBO);

  glGenTextures(1, &name);
  if (name == 0) {
    LOGV("INVALID GL_CONTEXT CANT MAKE TEXTURE\n");
    assert(false);
  }
  glBindTexture(GL_TEXTURE_2D, name);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  // generate FBO
  glGenFramebuffersOES(1, &fbo);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
  glGenRenderbuffersOES(1, &rbo);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, rbo);
  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, rbo);
  glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, name, 0);

  //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, self.textureId, 0);

  //glGenRenderbuffersOES(1, &depthRenderbuffer);
  //glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
  //glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, 512, 512);
  //glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);

  // check if it worked (probably worth doing :) )
  //Engine::CheckGL("wtf");
  
  GLuint status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
  if (status != GL_FRAMEBUFFER_COMPLETE_OES) {
    LOGV("INVALID GL CONTEXT CANT MAKE BUFFER\n");
    assert(false);
  }
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRBO);
  
  //glBindTexture(GL_TEXTURE_2D, 0);
}


RenderTexture::~RenderTexture() {
  LOGV("dealloc rendertexture %d\n", name);
  glDeleteTextures(1, &name);
  glDeleteBuffers(1, &fbo);
  glDeleteBuffers(1, &rbo);
  //glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}


void RenderTexture::Begin() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldRBO);

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, rbo);

  //glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  //glFlush();
  //glFinish();
  glBindTexture(GL_TEXTURE_2D, name);
}


void RenderTexture::End() {
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRBO);
  //glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  //glFlush();
  //glFinish();
  glBindTexture(GL_TEXTURE_2D, 0);
}
