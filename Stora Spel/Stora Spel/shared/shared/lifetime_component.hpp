#ifndef LIFETIME_COMPONENT_HPP
#define LIFETIME_COMPONENT_HPP
#include <util/timer.hpp>

struct {
  Timer timer;
  float time = 2.0f;
};

#endif  // LIFETIME_COMPONENT_HPP