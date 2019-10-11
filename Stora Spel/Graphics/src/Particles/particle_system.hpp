#ifndef PARTICLE_SYSTEM_HPP_
#define PARTICLE_SYSTEM_HPP_

#include <random>

#include "../shader.hpp"
#include "glob/camera.hpp"

#include "particle_settings.hpp"

namespace glob {

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

  ParticleSystem(ParticleSystem&& other);
  ParticleSystem& operator=(ParticleSystem&& other) = delete;

  void Settings(const ParticleSettings& ps);
  void SetTexture(GLuint tex);

  void Update(float dt);
  void Draw(ShaderProgram& shader, const Camera& camera);

private:
  std::default_random_engine gen_;

  ShaderProgram* compute_shader_;
  glob::ParticleSettings settings_ = {};
  float spawns_ = 0.f;

  //std::vector<Particle> particles_;
  GLuint position_vbo_;
  GLuint velocity_buffer_;
  GLuint color_vbo_;
  GLuint size_vbo_;
  GLuint time_buffer_;
  GLuint vertex_array_object_;
  GLuint texture_;

  int current_index_ = 0;
  //glm::vec4 color_ = glm::vec4(0.3f, 0.4f, 0.6f, 0.3f);
  //float particle_size_ = 0.03f;
  int SIZE = 10000;

private:
  void CleanUp();
  void Reset();
  void Spawn(int num);
};

}  // namespace glob

#endif  // PARTICLE_SYSTEM_HPP_
