#ifndef SSAO_HPP_
#define SSAO_HPP_

#include <glad\glad.h>
#include <vector>
#include "glm/glm.hpp"

class Ssao {
 public:
  Ssao();
  ~Ssao();
  void Init();
  void Pass();
  void BindFrameBuffer();
  void Finish();

  void BindNoiseTexture(int slot);
  void BindSsaoTexture(int slot);
  std::vector<glm::vec3> GetKernel() { return kernel_; }

 private:
  std::vector<glm::vec3> noise_;
  std::vector<glm::vec3> kernel_;

  GLuint noise_texutre_;
  GLuint ssaoFBO_;
  GLuint ssao_texture_;

  void CreateKernelAndNoise();
  void CreateFrameBuffers();
  void GenerateTextures();
};

#endif  // !SSAO_HPP_
