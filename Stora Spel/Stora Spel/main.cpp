#include <iostream>

#include <glob/graphics.h>
#include <entity/registry.hpp>
#include "PrintPositionSystem.h"
#include "collision_system.h"
#include "collision.h"
#include "ball_component.h"
#include "physics.h"

//#include <glad/glad.h>

int main(unsigned argc, char **argv) {
  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  entt::registry registry;

  auto entity = registry.create();
  registry.assign<BallComponent>(entity, true, false);
  registry.assign<Velocity>(entity, 1.0f, 0.0f, 0.0f);
  registry.assign<Sphere>(entity, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);

  entity = registry.create();
  registry.assign<Velocity>(entity, .0f, .0f, .0f);
  registry.assign<OBB>(entity, glm::vec3(5.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), 1.f, 1.f, 1.f);

  UpdateCollisions(registry);

  PhysicsObject po;
  po.friction = 0.f;
  po.velocity = glm::vec3(0, 20, 0);
  po.position = glm::vec3(0, 0, 0);

  for (int i = 0; i < 100; ++i) update(&po, 0.1f);

  std::cout << "Test :" << po.position.y << std::endl;

  return EXIT_SUCCESS;
}