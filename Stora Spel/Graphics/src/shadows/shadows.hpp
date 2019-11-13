#ifndef GLOB_SHADOWS_HPP_
#define GLOB_SHADOWS_HPP_
#include <functional>

// no move plz
#include <glad/glad.h>
// no move plz

#include <GLFW/glfw3.h>

#include "postprocess/blur.hpp"
#include "shader.hpp"

namespace glob {

class Shadows {
 public:
  Shadows();

  void Init(Blur& blur);

  void RenderToMaps(std::function<void(ShaderProgram&)> draw_function,
                    std::function<void(ShaderProgram&)> anim_draw_function,
                    Blur& blur);

  void BindMaps(int start_slot);

  void SetUniforms(ShaderProgram& shader);

  int GetNumMaps() { return num_maps_used_; }
  int GetMaxMaps() { return max_maps_; }

  void SetNumUsed(int num_used) { num_maps_used_ = num_used; }

 private:
  int GetBlurredSize() { return size_ / glm::pow(2, blurred_level_); }

  static const int max_maps_ = 4;
  int num_maps_used_ = 4;
  GLuint framebuffer_ = 0;
  GLuint renderbuffer_ = 0;
  GLuint texture_ = 0;
  GLuint blurred_textures_[max_maps_] = {0};
  int size_ = 0;
  int blurred_level_ = 0;
  ShaderProgram shader_;
  ShaderProgram anim_shader_;
  uint64_t blur_id_ = 0;
  GLint internal_format_ = 0;

  int start_slots_[max_maps_];
  glm::vec3 positions_[max_maps_];
  glm::mat4 transforms_[max_maps_];
};

}  // namespace glob

#endif  //  GLOB_SHADOWS_HPP_