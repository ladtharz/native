#ifdef ANDROID
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "base/logging.h"
#include "gfx_es2/fbo.h"

struct FBO {
  GLuint handle;
  GLuint color_texture;
  GLuint z_stencil_buffer;

  int width;
  int height;
};

FBO *fbo_create(int width, int height, int num_color_textures, bool z_stencil) {
  FBO *fbo = new FBO();
  fbo->width = width;
  fbo->height = height;
  glGenFramebuffers(1, &fbo->handle);
  glGenTextures(1, &fbo->color_texture);
  glGenRenderbuffers(1, &fbo->z_stencil_buffer);

  // Create the surfaces.
  glBindTexture(GL_TEXTURE_2D, fbo->color_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  glBindRenderbuffer(GL_RENDERBUFFER, fbo->z_stencil_buffer);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);

  // Bind it all together
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->handle);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo->color_texture, 0);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->z_stencil_buffer);
  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
  case GL_FRAMEBUFFER_COMPLETE_EXT:
    ILOG("Framebuffer verified complete.");
    break;
  case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
    ELOG("Framebuffer format not supported");
    break;
  default:
    FLOG("Other framebuffer error: %i", status);
    break;
  }
  // Unbind state we don't need
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  return fbo;
}

void fbo_unbind() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void fbo_bind_as_render_target(FBO *fbo) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->handle);
}

void fbo_bind_for_read(FBO *fbo) {
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle);
}

void fbo_bind_color_as_texture(FBO *fbo, int color) {
  glBindTexture(GL_TEXTURE_2D, fbo->color_texture);
}

void fbo_destroy(FBO *fbo) {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo->handle);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
  glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &fbo->handle);
  glDeleteTextures(1, &fbo->color_texture);
  glDeleteRenderbuffers(1, &fbo->z_stencil_buffer);
}
