#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <chrono>
#include <vector>

class Timer {
  bool paused_ = false;
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
  std::chrono::time_point<std::chrono::high_resolution_clock> pause_time_;

 public:
  Timer() { start_ = std::chrono::high_resolution_clock::now(); }
  double Restart() {
    auto now = std::chrono::high_resolution_clock::now();
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start_)
                      .count();
    start_ = std::chrono::high_resolution_clock::now();
    paused_ = false;
    return diff / 1000000000.0;
  }
  double RestartNS() {
    auto now = std::chrono::high_resolution_clock::now();
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start_)
                      .count();
    start_ = std::chrono::high_resolution_clock::now();
    paused_ = false;
    return diff;
  }
  double RestartMS() {
    auto now = std::chrono::high_resolution_clock::now();
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start_)
                      .count();
    start_ = std::chrono::high_resolution_clock::now();
    paused_ = false;
    return diff / 1000000.0;
  }
  double ElapsedMS() {
    auto now = std::chrono::high_resolution_clock::now();
    if (paused_) now = pause_time_;
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start_)
                      .count();
    return diff / 1000000.0;
  }

  double ElapsedNS() {
    auto now = std::chrono::high_resolution_clock::now();
    if (paused_) now = pause_time_;
    double diff = (double)std::chrono::duration_cast<std::chrono::nanoseconds>(
                      now - start_)
                      .count();
    return diff;
  }

  double Elapsed() { return ElapsedMS() / 1000.0; }

  void Pause() {
    if (!paused_) {
      pause_time_ = std::chrono::high_resolution_clock::now();
      paused_ = true;
    }
  }
  void Resume() {
    if (paused_) {
      auto now = std::chrono::high_resolution_clock::now();
      start_ += now - pause_time_;
      paused_ = false;
    }
  }
};

#endif  // TIMER_HPP_