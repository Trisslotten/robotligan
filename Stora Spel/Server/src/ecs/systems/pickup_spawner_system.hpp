#ifndef PICKUP_SPAWNER_SYSTEM_HPP_
#define PICKUP_SPAWNER_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entt.hpp>

namespace pickup_spawner_system {

void Update(entt::registry& registry, float dt) {
  auto view_spawners =
      registry.view<PickupSpawnerComponent, TransformComponent, IDComponent>();

  for (auto spawner : view_spawners) {
    auto& s_spawner_c = registry.get<PickupSpawnerComponent>(spawner);
    auto& s_trans_c = registry.get<TransformComponent>(spawner);

    auto view_id = registry.view<IDComponent>();
    bool spawned_still_exists = false;
    for (auto id_ent : view_id) {
      if (registry.get<IDComponent>(id_ent).id == s_spawner_c.spawned_id) {
        spawned_still_exists = true;
        s_spawner_c.spawn_timer.Restart();
        break;
      }
    }
    if (!spawned_still_exists &&
        (s_spawner_c.spawn_timer.Elapsed() >= s_spawner_c.spawn_time ||
         s_spawner_c.override_respawn)) {
      // spawn new pickup by sending event to playstate
      EventInfo e;
      e.e_id = registry.get<IDComponent>(spawner).id;
      e.entity = spawner;
      e.event = Event::SPAWNER_SPAWNED_PICKUP;
      dispatcher.trigger(e);
      s_spawner_c.spawn_timer.Restart();
      s_spawner_c.override_respawn = false;
    }
  }
}
}  // namespace pickup_spawner_system

#endif  // PICKUP_SPAWNER_SYSTEM_HPP_