#include "shadows.hpp"

#include <iostream>

#include <glm/glm.hpp>
#include "postprocess/blur.hpp"


void glob::Shadows::Init(Blur& blur) {
  shadow_blur_id_ =
      blur.CreatePass(shadow_blurred_size_, shadow_blurred_size_, GL_RG32F);

  shadow_shader_.add("modelshader.vert");
  shadow_shader_.add("shading.vert");
  shadow_shader_.add("shadow.frag");
  shadow_shader_.compile();

  glGenFramebuffers(1, &shadow_framebuffer_);
  glGenRenderbuffers(1, &shadow_renderbuffer_);

  glGenTextures(1, &shadow_texture_);
  glBindTexture(GL_TEXTURE_2D, shadow_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadow_size_, shadow_size_, 0, GL_RG,
               GL_UNSIGNED_BYTE, NULL);

  glGenTextures(num_shadow_maps_, shadow_blurred_textures_);
  for (int i = 0; i < num_shadow_maps_; i++) {
    glBindTexture(GL_TEXTURE_2D, shadow_blurred_textures_[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, shadow_blurred_size_,
                 shadow_blurred_size_, 0, GL_RG, GL_UNSIGNED_BYTE, NULL);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_);

  glBindRenderbuffer(GL_RENDERBUFFER, shadow_renderbuffer_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, shadow_size_,
                        shadow_size_);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         shadow_texture_, 0);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, shadow_renderbuffer_);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR: graphics.cpp: shadow_framebuffer is not complete!"
              << std::endl;
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void glob::Shadows::BeforePass() {
  std::vector<std::pair<glm::vec3, glm::mat4>> shadow_casters;
  shadow_casters.push_back(std::make_pair(
      glm::vec3(10.6, 5.7, 7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(10.6, 5.7, 7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  shadow_casters.push_back(std::make_pair(
      glm::vec3(-10.6, 5.7, -7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(-10.6, 5.7, -7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  shadow_casters.push_back(std::make_pair(
      glm::vec3(10.6, 5.7, -7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(10.6, 5.7, -7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  shadow_casters.push_back(std::make_pair(
      glm::vec3(-10.6, 5.7, 7.1),
      glm::perspective(glm::radians(70.f), 1.f, 0.1f, 50.f) *
          glm::lookAt(glm::vec3(-10.6, 5.7, 7.1), glm::vec3(0, -5.7, 0),
                      glm::vec3(0, 1, 0))));

  glBindFramebuffer(GL_FRAMEBUFFER, shadow_framebuffer_);
  glViewport(0, 0, shadow_size_, shadow_size_);
  
  for (int i = 0; i < shadow_casters.size(); i++) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::vec3 shadow_light_pos = shadow_casters[i].first;
    glm::mat4 shadow_transform = shadow_casters[i].second;

    shadow_shader.use();
    shadow_shader.uniform("cam_transform", shadow_transform);
    shadow_shader.uniform("shadow_light_pos", shadow_light_pos);
    for (auto &render_item : items_to_render) {
      shadow_shader.uniform("model_transform", render_item.transform);
      render_item.model->Draw(shadow_shader);
    }
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadow_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    blur.BlurTexture(shadow_blur_id, shadow_texture, 1,
                     shadow_blurred_textures[i]);

    model_shader.use();
    model_shader.uniform("shadow_transforms[" + std::to_string(i) + "]",
                         shadow_transform);
    model_shader.uniform("shadow_light_positions[" + std::to_string(i) + "]",
                         shadow_light_pos);
    model_shader.uniform("shadow_maps[" + std::to_string(i) + "]", 3 + i);

    model_emission_shader.use();
    model_emission_shader.uniform(
        "shadow_transforms[" + std::to_string(i) + "]", shadow_transform);
    model_emission_shader.uniform(
        "shadow_light_positions[" + std::to_string(i) + "]", shadow_light_pos);
    model_emission_shader.uniform("shadow_maps[" + std::to_string(i) + "]",
                                  3 + i);
  }
}
