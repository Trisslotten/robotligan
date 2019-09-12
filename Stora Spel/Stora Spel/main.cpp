#include <iostream>

#include <glob/graphics.h>
#include <glob/window.h>
#include <entt.hpp>
#include "AbilityControllerSystem.hpp"
#include "PlayerControllerSystem.h"
#include "PrintPositionSystem.h"

#include <GLFW/glfw3.h>
#include "util/input.h"
#include "util/meminfo.h"

void init() {
  glob::window::Create();
  glob::Init();
  Input::initialize();
}

void updateSystems(entt::registry *reg) {
  p_controller::update(*reg);
  a_controller::Update(*reg);
}

int main(unsigned argc, char **argv) {
  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  entt::registry registry;

  auto entity = registry.create();
  registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
  registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);

  print(registry);

  init();  // Initialize everything

  auto avatar = registry.create();  // this is the player avatar
  registry.assign<CameraComponent>(
      avatar,
      (Camera *)
          glob::GetCamera());  // get the camera pointer from glob renderer
  registry.assign<PlayerComponent>(avatar);
  registry.assign<TransformComponent>(avatar, glm::vec3(0, 0, 0),
                                      glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));

  while (!glob::window::ShouldClose()) {
    // tick
    updateSystems(&registry);

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