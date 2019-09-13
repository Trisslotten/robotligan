#include <iostream>

#include <glob/graphics.h>
#include <entt.hpp>
#include <NetAPI/networkTest.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include "PrintPositionSystem.h"
#include "util/meminfo.h"
#include <thread>
#include <chrono>
//#include <glad/glad.h>
#pragma comment(lib, "Ws2_32.lib")
int main(unsigned argc, char **argv) {

  std::cout << "Hello World!*!!!111\n";

  std::cout << "Test från development\n";
  entt::registry registry;

  auto entity = registry.create();
  registry.assign<Position>(entity, 1.0f, 2.0f, 3.0f);
  registry.assign<Velocity>(entity, 3.0f, 2.0f, 1.0f);
  print(registry);

  std::cout << "Test från development2 " << glob::GraphicsTest() << "\n";

  std::cout << "WSA is initialized? " << std::boolalpha << NetAPI::Initialization::WinsockInitialized() << std::endl;

  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM() << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM() << " MB\n";
  std::cin.ignore();
  return EXIT_SUCCESS;
}