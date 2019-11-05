#include "ssao.hpp"
#include <random>
#include "glm/vector_relational.hpp"
#include "glob/window.hpp"

float lerp(float a, float b, float f) { return a + f * (b - a); }

namespace glob {
Ssao::Ssao() {}

Ssao::~Ssao() {}

void Ssao::Init(Blur& blur) {
  CreateKernelAndNoise();
  GenerateTextures();
  CreateFrameBuffers();

  auto ws = glob::window::GetWindowDimensions();
  blur_id_ = blur.CreatePass(ws.x, ws.y, GL_R32F);
}

void Ssao::BindFrameBuffer() {
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Ssao::Finish(Blur& blur) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D, ssao_texture_);
  glGenerateMipmap(GL_TEXTURE_2D);

  blurred_ssao_texture_ = blur.BlurTexture(blur_id_, 2, ssao_texture_, 0);
}

void Ssao::CreateKernelAndNoise() {
  // kernel
  std::uniform_real_distribution<float> randomFloats(
      0.0, 1.0);  // random floats between 0.0 - 1.0
  std::default_random_engine generator;
  for (unsigned int i = 0; i < 64; ++i) {
    glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
                     randomFloats(generator) * 2.0 - 1.0,
                     randomFloats(generator));
    sample = glm::normalize(sample);
    sample *= randomFloats(generator);
    float scale = (float)i / 64.0;
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    kernel_.push_back(sample);
  }

  // noise
  for (unsigned int i = 0; i < 16; i++) {
    glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0,
                    randomFloats(generator) * 2.0 - 1.0, 0.0f);
    noise_.push_back(noise);
  }
}

void Ssao::CreateFrameBuffers() {
  glGenFramebuffers(1, &ssaoFBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         ssao_texture_, 0);

  GLuint att[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, att);
}

void Ssao::GenerateTextures() {
  auto ws = glob::window::GetWindowDimensions();
  // noise
  glGenTextures(1, &noise_texutre_);
  glBindTexture(GL_TEXTURE_2D, noise_texutre_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT,
               &noise_[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // output ssao texture
  glGenTextures(1, &ssao_texture_);
  glBindTexture(GL_TEXTURE_2D, ssao_texture_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ws.x, ws.y, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Ssao::BindNoiseTexture(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, noise_texutre_);
}

void Ssao::BindSsaoTexture(int slot) {
  glActiveTexture(GL_TEXTURE0 + slot);
  glBindTexture(GL_TEXTURE_2D, blurred_ssao_texture_);
}
}  // namespace glob