#ifndef GLOB_BLUR_HPP_
#define GLOB_BLUR_HPP_

#include <unordered_map>
#include <unordered_set>
#include "shader.hpp"

namespace glob {

class Blur {
 public:
  void Init();
  // internal_format must be valid format for glBindImageTexture
  // example: GL_RGB8 does not work, use GL_RGBA8 instead
  uint64_t CreatePass(uint16_t width, uint16_t height, int32_t internal_format);

  GLuint BlurTexture(uint64_t pass_id, int num_passes, GLuint source_texture,
                     int source_level = 0, GLuint result_texture = 0);

 private:
  union PassInfo {
    struct {
      uint16_t width;
      uint16_t height;
      int32_t internal_format;
    } info;
    uint64_t id;
  };

  struct Pass {
    GLuint textures[2] = {0, 0};
    int index = 0;
  };

  ShaderProgram kawase_blur_compute_;

  std::unordered_map<uint64_t, Pass> passes_;
};

}  // namespace glob

#endif  // GLOB_BLUR_HPP_