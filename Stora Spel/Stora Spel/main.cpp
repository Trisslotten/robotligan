#include <iostream>

#include <glob/graphics.h>
#include <entt.hpp>
#include "PrintPositionSystem.h"
#include "collision.h"
#include "physics.h"

//#include <glad/glad.h>

int main(unsigned argc, char **argv) {
  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  entt::registry registry;

  auto entity = registry.create();
  registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
  registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);

  print(registry);

  std::cout << "Test från development2 " << glob::GraphicsTest() << "\n";

  Sphere s1 = {};
  s1.radius = 5.f;
  Sphere s2 = {};
  s2.center = {6.f, 0.f, 0.f};
  s2.radius = 0.5f;

  OBB o = {};
  o.center = {-6.0f, 1.0f, 0.0f};
  o.extents[0] = 1.0f;
  o.extents[1] = 1.0f;
  o.extents[2] = 1.0f;
  o.normals[0] = {1.0f, 0.0f, 0.0f};
  o.normals[1] = {0.0f, 1.0f, 0.0f};
  o.normals[2] = {0.0f, 0.0f, 1.0f};

  OBB o2 = {};
  o2.center = { 0.0f, 0.0f, 0.0f};
  o2.extents[0] = 4.99f;
  o2.extents[1] = 5.0f;
  o2.extents[2] = 6.0f;
  o2.normals[0] = {1.0f, 0.0f, 0.0f};
  o2.normals[1] = {0.0f, 1.0f, 0.0f};
  o2.normals[2] = {0.0f, 0.0f, 1.0f};


  if (Intersect(s1, o))
    std::cout << "Intersection\n";
  else
    std::cout << "No collision\n";
  if (Intersect(o2, o))
    std::cout << "Intersection\n";
  else
    std::cout << "No collision\n";

  PhysicsObject po;
  po.friction = 0.f;
  po.velocity = glm::vec3(0, 20, 0);
  po.position = glm::vec3(0, 0, 0);

  for (int i = 0; i < 100; ++i) update(&po, 0.1f);

  std::cout << "Test :" << po.position.y << std::endl;

  return EXIT_SUCCESS;
}