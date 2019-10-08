#ifndef PARTICLE_SYSTEM_HPP_
#define PARTICLE_SYSTEM_HPP_

#include "../shader.hpp"

namespace glob {

struct Particle {
  glm::vec3 position;
  glm::vec3 velocity;
  float time;
};

class ParticleSystem {
public:
  ParticleSystem();
  ~ParticleSystem();

  void Update();
  void Draw(ShaderProgram& shader);

private:
  std::vector<Particle> particles_;
  GLuint vertex_buffer_object_;
  GLuint vertex_array_object_;
  int SIZE = 10000;

private:
  struct Vertex {
   glm::vec3 pos;
  };
};

}  // namespace glob

#endif  // PARTICLE_SYSTEM_HPP_
