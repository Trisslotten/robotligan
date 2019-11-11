#ifndef GLOB_MATERIAL_HPP_
#define GLOB_MATERIAL_HPP_

#include <glad\glad.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "shader.hpp"

namespace glob {

namespace materials {
enum Type {
  NORMAL,
  METALLIC,
  ROUGHNESS,
};
}

class Material {
 public:
  void AddTexture(materials::Type type, GLuint id, int slot, const std::string& uniform_name);
  void Bind(ShaderProgram& shader);

 private:
  struct Texture {
    GLuint id = 0;
    int slot = 0;
    std::string uniform_name;
  };
  std::unordered_map<materials::Type, Texture> textures_;
};

namespace materials {

void Init();
Material Get(std::unordered_map<Type, std::string> wanted_textures);

};  // namespace materials

}  // namespace glob

#endif  // GLOB_MATERIAL_HPP_