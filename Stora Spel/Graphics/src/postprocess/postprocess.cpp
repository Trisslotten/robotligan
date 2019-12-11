#include "postprocess.hpp"

#include <iostream>

#include "glob/window.hpp"

void glob::PostProcess::Init(Blur& blur) {
  auto ws = glob::window::GetWindowDimensions();

  //glGenRenderbuffers(1, &renderbuffer_);
  glGenFramebuffers(1, &framebuffer_);

  glGenTextures(1, &draw_color_texture_);
  glBindTexture(GL_TEXTURE_2D, draw_color_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ws.x, ws.y, 0, GL_RGBA, GL_FLOAT,
               NULL);

  glGenTextures(1, &draw_emission_texture_);
  glBindTexture(GL_TEXTURE_2D, draw_emission_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ws.x, ws.y, 0, GL_RGBA, GL_FLOAT,
               NULL);
  glGenerateMipmap(GL_TEXTURE_2D);

  glGenTextures(1, &draw_depth_texture_);
  glBindTexture(GL_TEXTURE_2D, draw_depth_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ws.x, ws.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
               NULL);
  // glGenerateMipmap(GL_TEXTURE_2D);
  // glGenerateMipmap(GL_TEXTURE_2D);

  blur_id_ = blur.CreatePass(ws.x / 2, ws.y / 2, GL_RGBA16F);

  glBindTexture(GL_TEXTURE_2D, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  //glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
  //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ws.x, ws.y);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         draw_color_texture_, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         draw_emission_texture_, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         draw_depth_texture_, 0);

  //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
  //                          GL_RENDERBUFFER, renderbuffer_);

  GLuint att[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, att);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR: postprocess.cpp: framebuffer_ is not complete!"
              << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void glob::PostProcess::BeforeDraw() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void glob::PostProcess::AfterDraw(Blur& blur) {
  //glBindTexture(GL_TEXTURE_2D, draw_color_texture_);
  //glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, draw_emission_texture_);
  glGenerateMipmap(GL_TEXTURE_2D);

  blurred_emission_texture = blur.BlurTexture(blur_id_, 3, draw_emission_texture_, 2);
}

void glob::PostProcess::BindColorTex(GLuint slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, draw_color_texture_);
}

void glob::PostProcess::BindEmissionTex(GLuint slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, blurred_emission_texture);
}

void glob::PostProcess::BindDepthTex(GLuint slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, draw_depth_texture_);
}