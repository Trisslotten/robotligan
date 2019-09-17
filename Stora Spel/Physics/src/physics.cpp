#include "physics.hpp"

#include <glm/glm.hpp>

constexpr float g = 9.82f;

void physics::Update(physics::PhysicsObject* po, float dt) {
  float deacc = po->friction * dt;
  float vel = glm::length(po->velocity);
  vel -= deacc;
  glm::vec3 old_vel = po->velocity;
  if (vel <= 0) {
    po->velocity = glm::vec3(0);
  } else {
    po->velocity = glm::normalize(po->velocity) * vel;	
  }

  
  if (po->airborne) {
    po->velocity += glm::vec3(0, -1, 0) * g * dt;
  }
  po->position += 0.5f * (po->velocity + old_vel) * dt;
}