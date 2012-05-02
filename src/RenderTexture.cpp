// Jon Bardin GPL

#include "MemoryLeak.h"

RenderTexture::RenderTexture(int width, int height) {
  name = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_RENDERBUFFER_BINDING_OES, &oldRBO);
}


RenderTexture::~RenderTexture() {
  LOGV("dealloc rendertexture %d\n", name);
  glDeleteTextures(1, &name);
  glDeleteBuffers(1, &fbo);
  glDeleteBuffers(1, &depthRenderbuffer);
}


void RenderTexture::Begin() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldRBO);

  // generate FBO
  glGenFramebuffersOES(1, &fbo);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);

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
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)512, (GLsizei)512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, name, 0);

  glGenRenderbuffersOES(1, &depthRenderbuffer);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
  glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, 512, 512);
  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);

  GLuint status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
  if (status != GL_FRAMEBUFFER_COMPLETE_OES) {
    LOGV("2222 INVALID GL CONTEXT CANT MAKE BUFFER\n");
    assert(false);
  }
}


void RenderTexture::End() {
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  glBindRenderbufferOES(GL_RENDERBUFFER_OES, oldRBO);
  glBindTexture(GL_TEXTURE_2D, 0);
}
