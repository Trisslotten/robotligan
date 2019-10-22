#include "physics.hpp"

#include <glm/glm.hpp>

namespace physics {
float g = 9.82f;
} // namespace physics



void physics::Update(physics::PhysicsObject* po, float dt) {
  glm::vec3 old_vel = po->velocity;
  po->velocity += po->acceleration * dt;
  float vel = glm::length(po->velocity);
  float deacc = po->friction * dt;
  vel -= deacc;
  if (vel <= 0) {
    po->velocity = glm::vec3(0);
  } else {
    if (vel > po->max_speed) vel = po->max_speed;

    po->velocity = glm::normalize(po->velocity) * vel;
  }

  if (po->airborne) {
    po->velocity += glm::vec3(0, -1, 0) * g * dt;
  }
  po->position += 0.5f * (po->velocity + old_vel) * dt;
}

void physics::SetGravity(float _g) { g = _g; }