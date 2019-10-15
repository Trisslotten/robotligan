#ifndef GLOB_SHADOWS_HPP_
#define GLOB_SHADOWS_HPP_

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>

#include "shader.hpp"
#include "postprocess/blur.hpp"

namespace glob {

class Shadows {
 public:

  void Init(Blur& blur);

  void BeforePass();

 private:
  static const int num_shadow_maps_ = 4;
  GLuint shadow_framebuffer_;
  GLuint shadow_renderbuffer_;
  GLuint shadow_texture_;
  GLuint shadow_blurred_textures_[num_shadow_maps_] = {0};
  int shadow_size_ = 1024;
  int shadow_blurred_level_ = 1;
  int shadow_blurred_size_ = shadow_size_ / glm::pow(2, shadow_blurred_level_);
  ShaderProgram shadow_shader_;
  uint64_t shadow_blur_id_ = 0;
};

}  // namespace glob

#endif  //  GLOB_SHADOWS_HPP_