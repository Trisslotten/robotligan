#ifndef GLOB_ROPE_HPP_
#define GLOB_ROPE_HPP_

#include <glad/glad.h>

#include "glob\camera.hpp"
#include "shader.hpp"

namespace glob {

class Rope {
 public:
  void Init();

  void Submit(glm::vec3 start, glm::vec3 end) {
    submitted_.push_back({start, end});
  }

  void Draw(glm::mat4 cam_transform);

 private:
  struct Submitted {
    glm::vec3 start, end;
  };

  std::vector<Submitted> submitted_;

  ShaderProgram shader_;
  int num_indices_ = 0;

  GLuint vao_;
  GLuint vbo_;
  GLuint ebo_;

  GLuint texture_diffuse_;
  GLuint texture_normal_;
};

}  // namespace glob

#endif  // GLOB_ROPE_HPP_