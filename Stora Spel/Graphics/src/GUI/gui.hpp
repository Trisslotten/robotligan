#ifndef GUI_HPP_
#define GUI_HPP_

#include <shader.hpp>
#include <glm/glm.hpp>

namespace glob {

class GUI {
 private:
  glm::vec2 pos_;
  glm::vec2 tex_;
  unsigned int size_;

 public:
  GUI();
  ~GUI();
  GLint LoadTextureFromFile(const char* path, std::string directory);
  void Draw(ShaderProgram& shader, glm::vec2 pos, GLuint tex_id,
            unsigned int size);
};

}  // namespace glob

#endif  // !GUI_HPP_
