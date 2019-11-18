#include "shadows.hpp"

#include <iostream>

#include <glm/glm.hpp>
#include <glm\ext\matrix_clip_space.hpp>
#include <glm\ext\matrix_transform.hpp>
#include "postprocess/blur.hpp"

glob::Shadows::Shadows() {
  size_ = 512;
  blurred_level_ = 0;
  internal_format_ = GL_RG32F;

  num_maps_used_ = 2;
}

void glob::Shadows::Init(Blur& blur) {
  blur_id_ =
      blur.CreatePass(GetBlurredSize(), GetBlurredSize(), internal_format_);

  shader_.add("modelshader.vert");
  shader_.add("shading.vert");
  shader_.add("shadow.frag");
  shader_.compile();

  anim_shader_.add("animatedmodelshader.vert");
  anim_shader_.add("shading.vert");
  anim_shader_.add("shadow.frag");
  anim_shader_.compile();

  glGenFramebuffers(1, &framebuffer_);
  glGenRenderbuffers(1, &renderbuffer_);

  glGenTextures(1, &texture_);
  glBindTexture(GL_TEXTURE_2D, texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, blurred_level_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  if (blurred_level_ > 0) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format_, size_, size_, 0, GL_RG,
               GL_UNSIGNED_BYTE, NULL);

  glGenTextures(max_maps_, blurred_textures_);
  for (int i = 0; i < max_maps_; i++) {
    glBindTexture(GL_TEXTURE_2D, blurred_textures_[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format_, GetBlurredSize(),
                 GetBlurredSize(), 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size_, size_);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texture_, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, renderbuffer_);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR: graphics.cpp: framebuffer is not complete!"
              << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void glob::Shadows::RenderToMaps(
    std::function<void(ShaderProgram&)> draw_function,
    std::function<void(ShaderProgram&)> anim_draw_function, Blur& blur) {
  if (!set_manually_) {
    float xrot = 1.f;
    float zrot = 1.f;
    for (int i = 0; i < max_maps_; i++) {
      positions_[i] = glm::vec3(xrot * 40.0, 20.0, zrot * 20.0);
      transforms_[i] = glm::perspective(glm::radians(70.f), 1.f, 0.1f, 100.f) *
                       glm::lookAt(positions_[i], glm::vec3(0, -40.0, 0),
                                   glm::vec3(0, 1, 0));

      std::swap(xrot, zrot);
      zrot *= -1.f;
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
  glViewport(0, 0, size_, size_);

  for (int i = 0; i < num_maps_used_; i++) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 light_pos = positions_[i];
    glm::mat4 transform = transforms_[i];

    shader_.use();
    shader_.uniform("cam_transform", transform);
    shader_.uniform("shadow_light_pos", light_pos);
    draw_function(shader_);

    anim_shader_.use();
    anim_shader_.uniform("cam_transform", transform);
    anim_shader_.uniform("shadow_light_pos", light_pos);
    anim_draw_function(anim_shader_);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_);
    if (blurred_level_ > 0) {
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    blur.BlurTexture(blur_id_, 1, texture_, blurred_level_,
                     blurred_textures_[i]);
  }
}

void glob::Shadows::BindMaps(int start_slot) {
  for (int i = 0; i < num_maps_used_; i++) {
    glActiveTexture(GL_TEXTURE0 + start_slot + i);
    glBindTexture(GL_TEXTURE_2D, blurred_textures_[i]);
    start_slots_[i] = start_slot + i;
  }
}

void glob::Shadows::SetUniforms(ShaderProgram& shader) {
  shader.uniform("num_shadows", num_maps_used_);
  shader.uniformv("shadow_light_positions", num_maps_used_, positions_);
  shader.uniformv("shadow_transforms", num_maps_used_, transforms_);
  shader.uniformv("shadow_maps", num_maps_used_, start_slots_);
}

void glob::Shadows::AddSpotlight(glm::vec3 position, glm::mat4 transform) {
  if (!set_manually_) {
    num_maps_used_ = 0;
  }
  if (num_maps_used_ < max_maps_) {
    positions_[num_maps_used_] = position;
    transforms_[num_maps_used_] = transform;
    num_maps_used_++;
  }
  set_manually_ = true;
}
