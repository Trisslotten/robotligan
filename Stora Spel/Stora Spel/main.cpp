#include <iostream>

#include <glob/graphics.h>
#include <entity/registry.hpp>
#include "collision_system.hpp"
#include "physics_system.hpp"
#include "collision.hpp"
#include "ball_component.hpp"
#include <glob/window.h>
#include <entt.hpp>
#include "print_position_system.hpp"
#include "ball_component.hpp"
#include "collision_system.hpp"
#include "physics_system.hpp"
#include "player_controller_system.hpp"
#include "ability_controller_system.hpp"

#include "util/meminfo.h"
//#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include "util/input.hpp"
#include "util/meminfo.hpp"

#include "util/timer.hpp"

void init() {
  glob::window::Create();
  glob::Init();
  Input::Initialize();
}

void updateSystems(entt::registry *reg, float dt) {
  player_controller::Update(*reg, dt);
  ability_controller::Update(*reg);
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
  registry.assign<VelocityComponent>(entity, glm::vec3(1.0f, 0.0f, 0.0f));
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

  timer.Restart();
  float dt = 0.0f;
  while (!glob::window::ShouldClose()) {
    dt = timer.Restart();
    Input::Reset();
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