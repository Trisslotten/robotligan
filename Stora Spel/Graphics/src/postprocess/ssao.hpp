#ifndef SSAO_HPP_
#define SSAO_HPP_

#include <glad\glad.h>
#include <vector>
#include "glm/glm.hpp"

#include "blur.hpp"
namespace glob {
class Ssao {
 public:
  Ssao();
  ~Ssao();
  void Init(Blur& blur);
  void BindFrameBuffer();
  void Finish(Blur& blur);

  void BindNoiseTexture(int slot);
  void BindSsaoTexture(int slot);
  std::vector<glm::vec3> GetKernel() { return kernel_; }

 private:
  std::vector<glm::vec3> noise_;
  std::vector<glm::vec3> kernel_;

  GLuint noise_texutre_;
  GLuint ssaoFBO_;
  GLuint ssao_texture_;
  GLuint blurred_ssao_texture_;

  uint64_t blur_id_;

  void CreateKernelAndNoise();
  void CreateFrameBuffers();
  void GenerateTextures();
};

} // namespace glob

#endif  // !SSAO_HPP_
