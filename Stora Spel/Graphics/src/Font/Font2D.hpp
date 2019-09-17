#ifndef FONT_2D_HPP_
#define FONT_2D_HPP_

#include <msdfgen/msdfgen-ext.h>
#include <msdfgen/msdfgen.h>
#include <string>
#include <shader.hpp>

namespace glob {

class Font2D {
 private:
  std::vector<unsigned char> font_texture_;

  std::string directory_ = "";

  bool is_loaded_ = false;

  void GenerateMsdfFont(const std::string& font_path,
                        const std::string& output_path);

  std::string GenerateFontDirectoryPath(const std::string* path);

 public:
  Font2D();
  Font2D(const std::string& path);
  ~Font2D();

  bool LoadFromFile(const std::string& path);
  bool IsLoaded() { return is_loaded_; };

  void Draw(ShaderProgram& shader);
};

}  // namespace glob

#endif  // !FONT_2D_HPP_
