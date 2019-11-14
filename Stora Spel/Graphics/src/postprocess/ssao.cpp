#include "ssao.hpp"
#include <iostream>
#include <random>
#include "glm/vector_relational.hpp"
#include "glob/window.hpp"
#include <glob\camera.hpp>

float lerp(float a, float b, float f) { return a + f * (b - a); }

namespace glob {
Ssao::Ssao() {
  internal_format_ = GL_R8;
  downscale_ = 4.f;
  num_samples_ = 8;
}

Ssao::~Ssao() {}

void Ssao::Init(Blur& blur) {
  CreateKernelAndNoise();
  GenerateTextures();

  auto ws = glob::window::GetWindowDimensions();
  blur_id_ = blur.CreatePass(ws.x / downscale_, ws.y / downscale_, internal_format_);

  shader.add("ssao.comp");
  shader.compile();
}

void Ssao::CreateKernelAndNoise() {
  // kernel
  std::uniform_real_distribution<float> randomFloats(
      0.0, 1.0);  // random floats between 0.0 - 1.0
  std::default_random_engine generator;
  for (unsigned int i = 0; i < num_samples_; ++i) {
    glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
                     randomFloats(generator) * 2.0 - 1.0,
                     randomFloats(generator));
    sample = glm::normalize(sample);
    sample *= randomFloats(generator);
    float scale = (float)i / num_samples_;
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    //sample.z += 0.25 * length(sample);
    kernel_.push_back(sample);
  }

  // noise
  for (unsigned int i = 0; i < 16; i++) {
    glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0, 0.f);
    noise_.push_back(noise);
  }
}

void Ssao::GenerateTextures() {
  auto ws = glob::window::GetWindowDimensions();
  // noise
  glGenTextures(1, &noise_texture_);
  glBindTexture(GL_TEXTURE_2D, noise_texture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT,
               &noise_[0]);

  // output ssao texture
  glGenTextures(1, &ssao_texture_);
  glBindTexture(GL_TEXTURE_2D, ssao_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, internal_format_, ws.x / downscale_, ws.y / downscale_, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void Ssao::BindNoiseTexture(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, noise_texture_);
}

void Ssao::BindSsaoTexture(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, blurred_ssao_texture_);
}
void Ssao::Process(PostProcess& post_process, Blur& blur,
                   Camera camera) {
  auto ws = glob::window::GetWindowDimensions();
  glm::mat4 cam_transform = camera.GetViewPerspectiveMatrix();
  float near = camera.GetNearPlane();
  float far = camera.GetFarPlane();

  glm::vec2 tex_size = ws / downscale_;

  shader.use();
  post_process.BindDepthTex(0);
  BindNoiseTexture(2);
  glBindImageTexture(0, ssao_texture_, 0, GL_FALSE, 0, GL_WRITE_ONLY, internal_format_);
  shader.uniform("texture_depth", 0);
  shader.uniform("texture_noise", 2);

  shader.uniformv("samples", (GLuint)kernel_.size(), kernel_.data());
  shader.uniform("projection", cam_transform);
  shader.uniform("inv_projection", inverse(cam_transform));
  shader.uniform("size", tex_size);
  shader.uniform("screen_dims", ws);

  shader.uniform("near", near);
  shader.uniform("far", far);
  
  float group_size = 16;
  glm::ivec2 num_groups = glm::ceil(tex_size / group_size);
  glDispatchCompute(num_groups.x, num_groups.y, 1);

  //glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R8);
  //glMemoryBarrier(GL_ALL_BARRIER_BITS);

  blurred_ssao_texture_  = ssao_texture_;
  blurred_ssao_texture_ = blur.BlurTexture(blur_id_, 4, ssao_texture_, 0);
}
}  // namespace glob