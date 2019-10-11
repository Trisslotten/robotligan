#ifndef PARTICLE_SETTINGS_HPP_
#define PARTICLE_SETTINGS_HPP_

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace glob {

struct ParticleSettings {
  glm::vec4 color = glm::vec4(1.0f);
  glm::vec3 emit_pos = glm::vec3(0.f, -2.f, 0.f);
  float size = 0.5;
  float time = 5.f;
  float spawn_rate = 50.f;
};

}  // namespace glob

#endif  // PARTICLE_SETTINGS_HPP_
