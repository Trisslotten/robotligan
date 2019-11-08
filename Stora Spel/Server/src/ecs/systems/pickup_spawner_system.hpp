#ifndef PICKUP_SPAWNER_SYSTEM_HPP_
#define PICKUP_SPAWNER_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <shared/transform_component.hpp>
#include <shared/pick_up_component.hpp>
#include <shared/id_component.hpp>
#include "util/event.hpp"
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

  auto view_pickups = registry.view<PickUpComponent,TransformComponent>();
  for (auto pickup : view_pickups) {
    float speed = 0.6f;
    auto& trans_c = registry.get<TransformComponent>(pickup);
    auto& pick_c = registry.get<PickUpComponent>(pickup);
	// y = -8.6
    float dir = pick_c.moving_up ? 1.0 : -1.0f;
    trans_c.position.y += dt * speed * dir;
    if (abs(trans_c.position.y - pick_c.o_pos.y) > pick_c.move_range) {
      pick_c.moving_up = !pick_c.moving_up;
	}
    trans_c.Rotate(glm::vec3(0, dt * speed*1.1f, 0));
  }

}
}  // namespace pickup_spawner_system

#endif  // PICKUP_SPAWNER_SYSTEM_HPP_