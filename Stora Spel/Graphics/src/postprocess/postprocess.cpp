#include "postprocess.hpp"

#include <iostream>

#include "glob/window.hpp"

void glob::PostProcess::Init() {
  kawase_blur_compute_.add("kawaseblur.comp");
  kawase_blur_compute_.compile();

  auto ws = glob::window::GetWindowDimensions();

  glGenRenderbuffers(1, &renderbuffer_);
  glGenFramebuffers(1, &framebuffer_);

  glGenTextures(1, &draw_color_texture_);
  glBindTexture(GL_TEXTURE_2D, draw_color_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ws.x, ws.y, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);

  glGenTextures(1, &draw_emission_texture_);
  glBindTexture(GL_TEXTURE_2D, draw_emission_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ws.x, ws.y, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glGenerateMipmap(GL_TEXTURE_2D);

  glGenTextures(2, emission_blur_textures_);
  for (int i = 0; i < 2; i++) {
    glBindTexture(GL_TEXTURE_2D, emission_blur_textures_[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, int(ws.x) / 2, int(ws.y) / 2, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  }

  glBindTexture(GL_TEXTURE_2D, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, ws.x, ws.y);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         draw_color_texture_, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         draw_emission_texture_, 0);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, renderbuffer_);

  GLuint att[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, att);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR: postprocess.cpp: default_framebuffer is not complete!"
              << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void glob::PostProcess::BeforeDraw() {
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void glob::PostProcess::AfterDraw() {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D, draw_emission_texture_);
  glGenerateMipmap(GL_TEXTURE_2D);

  kawase_blur_compute_.use();
  float group_size = 16;
  auto tex_size = glob::window::GetWindowDimensions() / 2.f;
  glm::ivec2 num_groups = glm::ceil(tex_size / group_size);
  kawase_blur_compute_.uniform("size", tex_size);
  // the kernels for each pass
  std::vector<int> kernels = {0, 1, 1};
  for (int i = 0; i < kernels.size(); i++) {
    kawase_blur_compute_.uniform("pass", i);
    kawase_blur_compute_.uniform("kernel", kernels[i]);

    int curr_i = emission_blur_tex_index_;
    int next_i = (emission_blur_tex_index_ + 1) % 2;
    glActiveTexture(GL_TEXTURE0);
    if (i == 0)
      glBindTexture(GL_TEXTURE_2D, draw_emission_texture_);
    else {
      glBindTexture(GL_TEXTURE_2D, emission_blur_textures_[curr_i]);
    }
    kawase_blur_compute_.uniform("read_tex", 0);
    glBindImageTexture(0, emission_blur_textures_[next_i], 0, GL_FALSE, 0,
                       GL_WRITE_ONLY, GL_RGBA8);

    glDispatchCompute(num_groups.x, num_groups.y, 1);

    emission_blur_tex_index_ = next_i;
  }
  glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
}

void glob::PostProcess::BindColorTex(GLuint slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, draw_color_texture_);
}

void glob::PostProcess::BindEmissionTex(GLuint slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, emission_blur_textures_[emission_blur_tex_index_]);
}
