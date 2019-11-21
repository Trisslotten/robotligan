#ifndef GLOB_SHOCKWAVES_HPP_
#define GLOB_SHOCKWAVES_HPP_

#include <glm/glm.hpp>
#include <timer/timer.hpp>
#include <vector>

#include "glob/camera.hpp"
#include "shader.hpp"

namespace glob {

class Shockwaves {
 public:
  void Create(glm::vec3 position, float duration, float size);

  void Update(Camera camera);

  void SetUniforms(ShaderProgram& shader);

 private:
  struct Shockwave {
    glm::vec3 position = glm::vec3(0);
    float duration = 0;
    float size = 0;
    Timer timer;
  };
  std::vector<Shockwave> shockwaves_;

  std::vector<glm::vec3> uniform_positions_;
  std::vector<float> uniform_time_ratios_;
  std::vector<float> uniform_radii_;
};

}  // namespace glob
#endif  // GLOB_SHOCKWAVES_HPP_