#ifndef GLOB_BLUR_HPP_
#define GLOB_BLUR_HPP_

#include <unordered_map>
#include "shader.hpp"
#include <unordered_set>

namespace glob {

class Blur {
 public:
  void Init();
  void CreatePass(uint16_t width, uint16_t height, int32_t internal_format);
  void Finalize();

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
    GLuint textures[2];
    int index = 0;
  };

  ShaderProgram kawase_blur_compute_;

  std::unordered_set<uint64_t> passes_;

  std::unordered_map<uint64_t, Pass> textures_;
  std::unordered_map<uint64_t, 
};

}  // namespace glob

#endif  // GLOB_BLUR_HPP_