#ifndef GLOB_MATERIAL_HPP_
#define GLOB_MATERIAL_HPP_

#include <glad\glad.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace glob {

class Material {
 public:
  bool HasNormalMap() { return normal_map_ != 0; }
  void BindNormalMap(int slot);
  void SetNormalMap(GLuint texture_id) { normal_map_ = texture_id; }

 private:
  GLuint normal_map_ = 0;
};

namespace materials {
enum Type {
  NORMAL,
};

Material Get(std::unordered_map<Type, std::string> wanted_textures);
};  // namespace materials

}  // namespace glob

#endif  // GLOB_MATERIAL_HPP_