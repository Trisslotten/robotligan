#ifndef GLOB_ROPE_HPP_
#define GLOB_ROPE_HPP_

#include <glad/glad.h>

#include "glob\camera.hpp"
#include "shader.hpp"

namespace glob {

class Rope {
 public:
  void Init();

  void TestDraw(Camera camera);

 private:
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