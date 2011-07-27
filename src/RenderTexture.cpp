// Jon Bardin GPL


#include "MemoryLeak.h"



RenderTexture::RenderTexture(int width, int height) {
  name = 0;
  glFlush();
  
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldRBO);

  glEnable(GL_TEXTURE_2D);
  
  glGenTextures(1, &name);
  LOGV("made in other: %d\n", name);
  Engine::CheckGL();
  if (name == 0) {
    LOGV("INVALID GL_CONTEXT CANT MAKE TEXTURE\n");
    assert(name);
  }
  glBindTexture(GL_TEXTURE_2D, name);
  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  // generate FBO
  glGenFramebuffersOES(1, &fbo);
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
  
  // attach depth buffer
  //GLuint depthRenderbuffer;
  //glGenRenderbuffersOES(1, &depthRenderbuffer);
  //glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
  //glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, width, height);
  //glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
  //glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
  
  // associate texture with FBO
  glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, name, 0);

  // check if it worked (probably worth doing :) )
  GLuint status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
  if (status != GL_FRAMEBUFFER_COMPLETE_OES) {
    LOGV("INVALID GL CONTEXT CANT MAKE BUFFER\n");
  }

  glDisable(GL_TEXTURE_2D);

  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  glBindFramebufferOES(GL_RENDERBUFFER_OES, oldRBO);

}


RenderTexture::~RenderTexture() {
  LOGV("dealloc render texture\n!!!!!!!!!!!!!!!!!!!!!\n");
}


void RenderTexture::Begin() {
  
  glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &oldFBO);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, fbo);
  //glBindTexture(GL_TEXTURE_2D, name);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}


void RenderTexture::End() {
  glBindFramebufferOES(GL_FRAMEBUFFER_OES, oldFBO);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glFlush();
}
