#ifndef PARTICLE_SETTINGS_HPP_
#define PARTICLE_SETTINGS_HPP_

#include <glm/vec4.hpp>
#include <glm/vec3.hpp>

namespace glob {

struct ParticleSettings {
  glm::vec4 color = glm::vec4(1.0f);
  glm::vec4 color_delta = glm::vec4(0.f);
  glm::vec3 emit_pos = glm::vec3(0.f, -2.f, 0.f);
  glm::vec3 direction = glm::vec3(0.f);
  float direction_strength = 0.f;
  float size = 0.5;
  float size_delta = 0.0;
  float time = 5.f;
  float spawn_rate = 50.f;
  float velocity = 1.f;
  float velocity_delta = 0.f;
  float radius = 0.f;
  float burst_particles = 0.f;
  int number_of_bursts = 1;
  bool burst = false;

  GLuint texture = 0;
};

}  // namespace glob

#endif  // PARTICLE_SETTINGS_HPP_
