#pragma once
#include <util/timer.hpp>
#include <shared/shared.hpp>

struct PickupSpawnerComponent {
  Timer spawn_timer;
  float spawn_time = 10.0f;
  EntityID spawned_id = -1;
  bool override_respawn = false;
};