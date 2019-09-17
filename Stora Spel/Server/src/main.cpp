#include <iostream>

#include "util/timer.hpp"
#include "gameserver.hpp"

int main(unsigned argc, char** argv) {
  std::cout << "DEBUG: Starting Server" << std::endl;

  Timer timer;
  double accum = 0;
  double tickrate = 64;
  double ticktime = 1.0 / tickrate;

  GameServer server;

  server.Init();

  int numframes = 0;
  Timer t;
  Timer sleep;

  bool running = true;
  while (running) {
    timer.Restart();
    // start tick

    server.Update(ticktime);

    // end tick
    numframes++;
    double time = timer.Elapsed();
    double time_left = ticktime - time;
    sleep.Restart();
    while (sleep.Elapsed() < time_left) {
      // busy wait lmao
    }

    if (t.Elapsed() > 1.0) {
      double elapsed = t.Restart();
      std::cout << "DEBUG: Real Tickrate = " << numframes / elapsed << "\n";
      numframes = 0;
    }
  }

  return EXIT_SUCCESS;
}