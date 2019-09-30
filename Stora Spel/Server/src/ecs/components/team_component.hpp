#ifndef TEAM_COMPONENT_HPP_
#define TEAM_COMPONENT_HPP_

#include <entt.hpp>

#define TEAM_RED (unsigned int)0
#define TEAM_BLUE (unsigned int)1

struct TeamCoponent {
  unsigned int team = TEAM_RED;
};

#endif  // TEAM_COMPONENT_HPP_