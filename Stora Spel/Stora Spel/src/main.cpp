#define NOMINMAX
#pragma comment(lib, "ws2_32.lib")
#include <NetAPI/networkTest.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <glob/window.hpp>
#include <iostream>

#include <GLFW/glfw3.h>  //NTS: This one must be included after certain other things
#include "util/meminfo.hpp"
#include "util/timer.hpp"

#include "engine.hpp"

#include <bitset>
#include <chrono>
#include <thread>

entt::dispatcher menu_dispatcher{};
entt::dispatcher dispatcher{};

int main(unsigned argc, char** argv) {
  std::cout << "WSA is initialized? " << std::boolalpha
            << NetAPI::Initialization::WinsockInitialized() << std::endl;

  glob::window::Create();

  Engine engine;
  engine.Init();

  double net_update_rate = kClientUpdateRate;
  double net_update_time = 1.0 / net_update_rate;

  int num_render_updates = 0;

  Timer net_update_timer;
  double net_update_accum = 0.0;
  int num_net_updates = 0;

  Timer debug_timer;

  Timer frame_timer;
  float dt = 1.f / 60.f;
  while (!glob::window::ShouldClose()) {
    engine.Update(dt);

    while (net_update_accum >= net_update_time) {
      engine.UpdateNetwork();

      num_net_updates++;
      net_update_accum -= net_update_time;
    }

    if (debug_timer.Elapsed() > 5.0) {
      double elapsed = debug_timer.Restart();
      std::cout << "DEBUG:    net update rate = " << num_net_updates / elapsed
                << " U/s\n       render update rate = "
                << num_render_updates / elapsed << " U/s\n";
      num_net_updates = 0;
      num_render_updates = 0;
    }

    if (Input::IsKeyPressed(GLFW_KEY_F6)) {
      std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
                << " MB\n";
      std::cout << "VRAM usage: "
                << util::MemoryInfo::GetInstance().GetUsedVRAM() << " MB\n";
    }

    engine.Render();
    glob::window::Update();

    num_render_updates++;

    dt = frame_timer.Restart();
    double frame_time = net_update_timer.Restart();
    net_update_accum += frame_time;
  }

  glob::window::Cleanup();

  /*
  std::cout << "RAM usage: " << util::MemoryInfo::GetInstance().GetUsedRAM()
            << " MB\n";
  std::cout << "VRAM usage: " << util::MemoryInfo::GetInstance().GetUsedVRAM()
            << " MB\n";
  */
  WSACleanup();
  return EXIT_SUCCESS;
}