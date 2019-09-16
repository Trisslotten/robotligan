#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <chrono>
#include <vector>

class Timer {
  bool paused_ = false;
  std::chrono::time_point<std::chrono::high_resolution_clock> start;
  std::chrono::time_point<std::chrono::high_resolution_clock> pause_time;

 public:
  Timer() { start = std::chrono::high_resolution_clock::now(); }
  double Restart() {
    auto now = std::chrono::high_resolution_clock::now();
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start)
                      .count();
    start = std::chrono::high_resolution_clock::now();
    paused_ = false;
    return diff / 1000000000.0;
  }
  double ElapsedMS() {
    auto now = std::chrono::high_resolution_clock::now();
    if (paused_) now = pause_time;
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start)
                      .count();
    return diff / 1000000.0;
  }

  double ElapsedNS() {
    auto now = std::chrono::high_resolution_clock::now();
    if (paused_) now = pause_time;
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start)
                      .count();
    return diff;
  }

  double Elapsed() { return ElapsedMS() / 1000.0; }

  void Pause() {
    if (!paused_) {
      pause_time = std::chrono::high_resolution_clock::now();
      paused_ = true;
    }
  }
  void Resume() {
    if (paused_) {
      auto now = std::chrono::high_resolution_clock::now();
      start += now - pause_time;
      paused_ = false;
    }
  }
};

#endif  // TIMER_HPP_