#ifndef LIFETIME_SYSTEM_HPP_
#define LIFETIME_SYSTEM_HPP_

#include <entt.hpp>
#include "ecs/components.hpp"

namespace lifetime {

void Update(entt::registry& registry, float dt) {
  auto view_timer = registry.view<TimerComponent>();
  for (auto entity : view_timer) {
    auto& timer = view_timer.get(entity);

    timer.time_left -= dt;
    if (timer.time_left <= 0.f) {
      registry.destroy(entity);
    }
  }
}

};  // namespace lifetime

#endif  // !LIFETIME_SYSTEM_HPP_
