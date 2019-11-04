#ifndef GLOB_MATERIAL_HPP_
#define GLOB_MATERIAL_HPP_

#include <string>
#include <glad\glad.h>

namespace glob {

class Material {
 public:
  void SetNormalMap(const std::string& path);
  bool HasNormalMap() { return normal_map_ != 0; }
  void BindNormalMap(int slot);

 private:

 GLuint normal_map_ = 0;

};

}  // namespace glob

#endif  // GLOB_MATERIAL_HPP_