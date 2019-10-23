#ifndef FONT_2D_HPP_
#define FONT_2D_HPP_

#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>
#include <shader.hpp>
#include <string>
#include <ft2build.h>
#include FT_FREETYPE_H

class msdfgen::FontHandle {
 public:
  FT_Face face;
};

namespace glob {



class Font2D {
 private:
  std::vector<unsigned char> font_texture_;

  std::vector<double> advances_;

  GLuint tex_id;

  std::string directory_ = "";

  unsigned int width_ = 32;
  unsigned int height_ = 32;
  unsigned int side_dim_ = 16;

  unsigned int texture_width_ = 32;
  unsigned int texture_height_ = 32;

  bool is_loaded_ = false;

  void GenerateMsdfShapes(const std::string& font_path);

  void GenerateMsdfFont(const std::string& font_path,
                        const std::string& output_path);

  std::string GenerateFontDirectoryPath(const std::string* path);

  void CreateTexture();

  msdfgen::FontHandle* font;

  std::vector<msdfgen::Shape> shapes;

 public:
  Font2D();
  Font2D(const std::string& path);
  ~Font2D();

  bool LoadFromFile(const std::string& path);
  bool IsLoaded() { return is_loaded_; };

  void Draw(ShaderProgram& shader, glm::vec2 pos, unsigned int size,
            std::string text, glm::vec4 color = glm::vec4(1,1,1,1), bool visible = true);

  void Draw3D(ShaderProgram& shader, glm::vec3 pos, float size,
              std::string text, glm::vec4 color = glm::vec4(1, 1, 1, 1), glm::mat4 rotation = glm::mat4(1.0f));
};

}  // namespace glob

#endif  // !FONT_2D_HPP_
