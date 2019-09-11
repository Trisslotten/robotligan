#include <iostream>

#include <entt.hpp>
#include "PrintPositionSystem.h"
#include <glob/window.h>
#include <glob/graphics.h>

#include "util/meminfo.h"
//#include <glad/glad.h>

int main(unsigned argc, char **argv) {
  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";

  entt::registry registry;

  auto entity = registry.create();
  registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
  registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);

  print(registry);

  glob::window::Create();

  glob::Init();

  while (!glob::window::ShouldClose()) {
    // tick

    // render
    glob::Render();

    glob::window::Update();
  }

  glob::window::Cleanup();

  std::cout << "Test från development2 " << glob::GraphicsTest() << "\n";
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM() << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM() << " MB\n";
  std::cin.ignore();
  return EXIT_SUCCESS;
}