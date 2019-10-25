#ifndef LIFETIME_SYSTEM_HPP_
#define LIFETIME_SYSTEM_HPP_

#include <entt.hpp>
#include "ecs/components.hpp"
#include "util/event.hpp"

#include "shared/id_component.hpp"


namespace lifetime {

void Update(entt::registry& registry, float dt) {
  auto view_health = registry.view<HealthComponent, IDComponent>();

  for (auto entity : view_health) {
    auto& health = view_health.get<HealthComponent>(entity);

    if (health.health <= 0) {
      auto& id = view_health.get<IDComponent>(entity);

      EventInfo event;
      event.event = Event::DESTROY_ENTITY;
      event.e_id = id.id;
      event.entity = entity;

      dispatcher.trigger(event);

      //registry.destroy(entity);
    }
  }

  auto view_timer = registry.view<TimerComponent, IDComponent>();
  for (auto entity : view_timer) {
    auto& timer = view_timer.get<TimerComponent>(entity);

    timer.time_left -= dt;
    if (timer.time_left <= 0.f) {
      auto& id = view_timer.get<IDComponent>(entity);

      EventInfo event;
      event.event = Event::DESTROY_ENTITY;
      event.e_id = id.id;
      event.entity = entity;

      dispatcher.trigger(event);

      //registry.destroy(entity);
    }
  }
}

};  // namespace lifetime

#endif  // !LIFETIME_SYSTEM_HPP_
