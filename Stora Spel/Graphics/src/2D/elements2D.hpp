#ifndef ELEMENTS2D_HPP_
#define ELEMENTS2D_HPP_

#include <glm/glm.hpp>
#include <shader.hpp>

namespace glob {

class Elements2D {
 private:
  std::string directory_;
  std::vector<unsigned char> gui_element_texture_;
  unsigned int texture_width_;
  unsigned int texture_height_;
  GLuint tex_id_;
  bool is_loaded_;
  void CreateTexture();

 public:
  Elements2D();
  ~Elements2D();
  bool LoadFromFile(const std::string& path);
  void DrawOnScreen(ShaderProgram& shader, glm::vec2 pos, float scale,
                    float scale_x, float opacity);
  void DrawInWorld(ShaderProgram& shader, glm::vec3 pos, float scale,
                   glm::mat4 rot);
  bool IsLoaded() { return is_loaded_; };
};

}  // namespace glob

#endif  // !ELEMENTS2D_HPP_
