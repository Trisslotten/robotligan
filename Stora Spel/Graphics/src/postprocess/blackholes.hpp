#ifndef GLOB_BLACKHOLES_HPP_
#define GLOB_BLACKHOLES_HPP_

#include <glm/glm.hpp>
#include <timer/timer.hpp>
#include <vector>

#include "glob/camera.hpp"
#include "shader.hpp"

namespace glob {

class BlackHoles {
 public:
  void Create(glm::vec3 position);

 void Update(Camera camera);

  void SetUniforms(ShaderProgram& shader);

 private:
  struct BlackHole{
    glm::vec3 position;
     
    Timer timer;
  };

  std::vector<BlackHole> black_holes_;

  std::vector<glm::vec3> uniform_positions_;
  std::vector<float> uniform_radii_;
  std::vector<float> uniform_strengths_;
};

}  // namespace glob
#endif  // GLOB_BLACKHOLES_HPP_