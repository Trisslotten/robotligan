#define NOMINMAX
#include <iostream>
#include <sstream>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "util/event.hpp"
#include "gameserver.hpp"
#include "util/timer.hpp"
#include "serverstate.hpp"
#include "util/global_settings.hpp"
#include "util/winadpihelpers.hpp"
#define NO_KILL_EXISTING_
entt::dispatcher dispatcher{};
std::string workingdir() {
  char buf[MAX_PATH + 1];
  GetCurrentDirectoryA(MAX_PATH, buf);
  return std::string(buf) + '\\';
}
int main(unsigned argc, char** argv) {
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
  std::cout << "Workingdir: " << workingdir() << std::endl;
  std::unordered_map<std::string, std::string> arguments;
  std::cout << "Num server arguments: " << argc << std::endl;
  GlobalSettings::Access()->UpdateValuesFromFile();

  if (argc > 1) {
    // IP - PORT
    arguments["IP"] = argv[0];
    arguments["PORT"] = argv[1];
    arguments["MPLAYERS"] = argv[2];
  } else {
    arguments["PORT"] = std::to_string(1337);
    float port = GlobalSettings::Access()->ValueOf("PORT");
    if (port > 0 && port < 65565) {
      arguments["PORT"] = std::to_string((int)port);
    }
    arguments["MPLAYERS"] = "6";
    float mplayers = GlobalSettings::Access()->ValueOf("MAX_PLAYERS");
    if (mplayers > 0) {
      arguments["MPLAYERS"] = std::to_string((int)(std::ceilf(mplayers)));
    }
#ifdef KILL_EXISTING_
	helper::ps::KillProcess("Server.exe");
	helper::ps::KillProcess("server.exe");
#endif // KILL_EXISTING
  }
  std::cout << "DEBUG: Starting Server" << std::endl;
  Timer timer;
  double accum_ms = 0;
  // max around 200'000 on home computer
  double update_rate = kServerUpdateRate;
  double update_time = 1.0 / update_rate;
  double update_time_ms = update_time * 1000.0;
  GameServer server;
  dispatcher.sink<EventInfo>().connect<&GameServer::ReceiveEvent>(server);
  dispatcher.sink<GameEvent>().connect<&GameServer::ReceiveGameEvent>(server);
  server.Init(update_rate, arguments);
  dispatcher.sink<EventInfo>().connect<&ServerPlayState::ReceiveEvent>(
      *server.GetPlayState());
  int num_frames = 0;
  Timer debug_timer;

  bool running = true;
  while (running) {
    accum_ms += timer.RestartMS();
    while (accum_ms >= update_time_ms) {
      server.Update(update_time);
      num_frames++;
      accum_ms -= update_time_ms;
    }

    auto sleep_time = std::chrono::microseconds(
        (int)glm::min(1000.0, update_time_ms * 1000.0));
    std::this_thread::sleep_for(sleep_time);

    
    if (debug_timer.Elapsed() > 2.0) {
      double elapsed = debug_timer.Restart();
      std::cout << "DEBUG: update rate = " << num_frames / elapsed << " U/s\n";
      num_frames = 0;
    }
    
  }
  dispatcher.sink<EventInfo>().disconnect<&ServerPlayState::ReceiveEvent>(
      *server.GetPlayState());
  WSACleanup();
  _CrtDumpMemoryLeaks();
  return EXIT_SUCCESS;
}