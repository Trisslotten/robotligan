#ifndef PARTICLE_SYSTEM_HPP_
#define PARTICLE_SYSTEM_HPP_

#include <random>

#include "../shader.hpp"
#include "glob/camera.hpp"

#include "particle_settings.hpp"

namespace glob {

constexpr int SIZE = 1000;

struct Particle {
  glm::vec4 position;
  glm::vec4 velocity;
  glm::vec4 color;
  float size;
  float time;
};

class ParticleSystem {
public:
  ParticleSystem(ShaderProgram* ptr, GLuint tex);
  ~ParticleSystem();

  ParticleSystem(const ParticleSystem&) = delete;
  ParticleSystem& operator=(const ParticleSystem&) = delete;

  ParticleSystem(ParticleSystem&& other) = default;
  ParticleSystem& operator=(ParticleSystem&& other) = delete;

  void Settings(const ParticleSettings& ps);
  ParticleSettings GetSettings();
  void SetPosition(glm::vec3 pos);
  void SetDirection(glm::vec3 dir);
  void SetTexture(GLuint tex);
  void SetShader(ShaderProgram* ptr);

  void Reset();

  void Update(float dt);
  void Draw(ShaderProgram& shader);

private:
  std::default_random_engine gen_;

  glob::ParticleSettings settings_ = {};
  float spawns_ = 0.f;

  GLuint position_vbo_ = 0;
  GLuint velocity_buffer_ = 0;
  GLuint color_vbo_ = 0;
  GLuint size_vbo_ = 0;
  GLuint time_buffer_ = 0;
  GLuint vertex_array_object_ = 0;

  int current_index_ = 0;
  int created_bursts = 0;

private:
  void Spawn(int num);
};

}  // namespace glob

#endif  // PARTICLE_SYSTEM_HPP_
