#ifndef HOOK_COMPONENT_HPP_
#define HOOK_COMPONENT_HPP_

#include <shared/shared.hpp>

enum HookMode { PULL_OBJECT, PULL_PLAYER };

struct HookComponent {
  HookMode hook_m = PULL_OBJECT;
  float pull_speed = 75.0f;
  bool attached = false;
  EntityID owner;
  EntityID hooked_entity;
  Timer hook_timer;
  float hook_time = 3.0f;
};

#endif // HOOK_COMPONENT_HPP_