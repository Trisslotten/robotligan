#include <iostream>

#include <glob/graphics.h>
#include <glob/window.h>
#include <entt.hpp>
#include "PrintPositionSystem.h"
#include "ball_component.h"
#include "collision.h"
#include "collision_system.h"
#include "physics_system.h"
#include "playercontroller_system.h"

#include <GLFW/glfw3.h>
#include "util/input.h"
#include "util/meminfo.h"

#include "util/timer.hpp"

void init() {
  glob::window::Create();
  glob::Init();
  Input::initialize();
}

void updateSystems(entt::registry *reg, float dt) {
  p_controller::Update(*reg, dt);
  UpdatePhysics(*reg, dt);
  UpdateCollisions(*reg);
}

int main(unsigned argc, char **argv) {
  Timer timer;

  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  entt::registry registry;

  auto entity = registry.create();
  registry.assign<BallComponent>(entity, true, false);
  registry.assign<Velocity>(entity, glm::vec3(1.0f, 0.0f, 0.0f));
  registry.assign<physics::Sphere>(entity, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f);

  entity = registry.create();

  entity = registry.create();
  registry.assign<physics::Arena>(entity, -10.f, 10.f, 0.f, 5.f, -10.f, 10.f);
  registry.assign<Velocity>(entity, glm::vec3(.0f, .0f, .0f));

  init();  // Initialize everything

  auto avatar = registry.create();  // this is the player avatar
  registry.assign<CameraComponent>(
      avatar, (Camera *)glob::GetCamera(),
      glm::vec3(0, 1, 0));  // get the camera pointer from glob renderer
  registry.assign<PlayerComponent>(avatar);
  registry.assign<TransformComponent>(avatar, glm::vec3(-9.f, 0.f, 0.f),
                                      glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
  registry.assign<Velocity>(avatar, glm::vec3(.0f, .0f, .0f));
  registry.assign<physics::OBB>(
      avatar, glm::vec3(5.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.f, 0.f),
      glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f), 1.f, 1.f, 1.f);

  timer.restart();
  float dt = 0.0f;
  while (!glob::window::ShouldClose()) {
    dt = timer.restart();
    Input::reset();
    // tick
    updateSystems(&registry, dt);

    // render
    glob::Render();

    glob::window::Update();
  }

  glob::window::Cleanup();
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
            << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM()
            << " MB\n";
  std::cin.ignore();
  return EXIT_SUCCESS;
}