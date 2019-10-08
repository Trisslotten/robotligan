#define NOMINMAX
#include <iostream>

#include "util/event.hpp"
#include "gameserver.hpp"
#include "util/timer.hpp"
#include "serverstate.hpp"

entt::dispatcher dispatcher{};

int main(unsigned argc, char** argv) {
  std::cout << "DEBUG: Starting Server" << std::endl;

  Timer timer;
  double accum_ms = 0;
  // max around 200'000 on home computer
  double update_rate = kServerUpdateRate;
  double update_time = 1.0 / update_rate;
  double update_time_ms = update_time * 1000.0;

  GameServer server;
  server.Init(update_rate);
  dispatcher.sink<EventInfo>().connect<&ServerPlayState::ReceiveEvent>(*server.GetPlayState());
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

    auto sleep_time =
        std::chrono::microseconds((int)glm::min(1000.0, update_time_ms * 1000.0));
    std::this_thread::sleep_for(sleep_time);

    /*
    if (debug_timer.Elapsed() > 5.0) {
      double elapsed = debug_timer.Restart();
      std::cout << "DEBUG: update rate = " << num_frames / elapsed << " U/s\n";
      num_frames = 0;
    }
    */
  }
  dispatcher.sink<EventInfo>().disconnect<&ServerPlayState::ReceiveEvent>(*server.GetPlayState());
  return EXIT_SUCCESS;
}