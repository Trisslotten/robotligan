#define NOMINMAX
#pragma comment(lib, "ws2_32.lib")
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include <NetAPI/networkTest.hpp>
#include <NetAPI/packet.hpp>
#include <NetAPI/socket/server.hpp>
#include <NetAPI/socket/tcpclient.hpp>
#include <bitset>
#include <chrono>
#include <glob/window.hpp>
#include <iostream>
#include <thread>

#include <GLFW/glfw3.h>  //NTS: This one must be included after certain other things
#include "engine.hpp"
#include "util/debugoverlay.hpp"
#include "util/meminfo.hpp"
#include "util/timer.hpp"

entt::dispatcher menu_dispatcher{};
entt::dispatcher dispatcher{};

int main(unsigned argc, char** argv) {
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
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

  DebubOverlay debug_overlay;
  debug_overlay.Init();

  Timer frame_timer;
  float dt = 1.f / 60.f;
  while (!glob::window::ShouldClose() && !engine.should_quit) {
    debug_overlay.BeforeEngineUpdate();
    engine.Update(dt);
    debug_overlay.AfterEngineUpdate();

    while (net_update_accum >= net_update_time) {
      debug_overlay.BeforeNetworkUpdate();
      engine.UpdateNetwork();
      debug_overlay.AfterNetworkUpdate();

      num_net_updates++;
      net_update_accum -= net_update_time;
    }

    if (debug_timer.Elapsed() > 1.0) {
      double elapsed = debug_timer.Restart();
      std::cout << "DEBUG:    net update rate = " << num_net_updates / elapsed
                << " U/s\n       render update rate = "
                << num_render_updates / elapsed << " U/s\n";
      num_net_updates = 0;
      num_render_updates = 0;
    }

    debug_overlay.Update();

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
  _CrtDumpMemoryLeaks();
  return EXIT_SUCCESS;
}