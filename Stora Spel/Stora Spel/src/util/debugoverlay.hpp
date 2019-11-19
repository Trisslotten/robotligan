#ifndef DEBUG_OVERLAY_HPP_
#define DEBUG_OVERLAY_HPP_

#include <GLFW/glfw3.h>
#include <glob/graphics.hpp>
#include "timer.hpp"

class DebubOverlay {
 public:
  void Init();
  void Update();

  void BeforeNetworkUpdate();
  void AfterNetworkUpdate();

  void BeforeEngineUpdate();
  void AfterEngineUpdate();

 private:
  bool active_ = false;

  Timer network_timer_;
  Timer engine_timer_;

  Timer update_values_timer;

  std::vector<double> network_times_;
  std::vector<double> engine_times_;

  double avg_network_time_ = 0;
  double min_network_time_ = 0;
  double max_network_time_ = 0;
  double network_fps_ = 0;

  double avg_engine_time_ = 0;
  double min_engine_time_ = 0;
  double max_engine_time_ = 0;
  double engine_fps_ = 0;

  int num_network_updates_ = 0;
  int num_engine_updates_ = 0;

  glob::GUIHandle background_;
  glob::Font2DHandle font_;
};

#endif  // DEBUG_OVERLAY_HPP_