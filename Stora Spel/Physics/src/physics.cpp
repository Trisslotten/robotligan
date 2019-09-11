#include "physics.h"

constexpr float g = 9.82;

void update(PhysicsObject* po, float dt) {
  float deacc = po->friction * dt;
  float vel = glm::length(po->velocity);
  vel -= deacc;
  if (vel < 0) vel = 0;

  glm::vec3 old_vel = po->velocity;
  po->velocity = glm::normalize(po->velocity) * vel +
                glm::vec3(0, -1, 0) * g * dt;

  po->position += 0.5f * (po->velocity + old_vel) * dt;
}