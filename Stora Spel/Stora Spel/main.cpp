#include <NetAPI/networkTest.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#undef min
#undef max
#include <glob/window.hpp>
#include <iostream>

#include <GLFW/glfw3.h>  //NTS: This one must be included after certain other things
#include "util/meminfo.hpp"
#include "util/timer.hpp"

#include "engine.hpp"

#include <bitset>
#include <chrono>
#include <thread>


int main(unsigned argc, char** argv) {
  std::cout << "WSA is initialized? " << std::boolalpha
            << NetAPI::Initialization::WinsockInitialized() << std::endl;
  
  glob::window::Create();

  Engine engine;
  engine.Init();

  Timer frame_timer;
  float dt = 1.f / 60.f;
  while (!glob::window::ShouldClose()) {
    engine.Update(dt);
    engine.Render();
    glob::window::Update();

    dt = frame_timer.Restart();
  }

  glob::window::Cleanup();

  /*
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
            << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM()
            << " MB\n";
  */
  return EXIT_SUCCESS;
}