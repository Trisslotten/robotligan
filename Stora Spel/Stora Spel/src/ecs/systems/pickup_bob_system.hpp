#ifndef PICKUP_BOB_SYSTEM_HPP_
#define PICKUP_BOB_SYSTEM_HPP_

#include <entity/registry.hpp>
#include <ecs/components.hpp>
#include <shared/transform_component.hpp>
#include <shared/pick_up_component.hpp>
#include <entt.hpp>

namespace pickup_bob_system {

void Update(entt::registry& registry, float dt) {
  auto view_pickups = registry.view<PickUpComponent, TransformComponent>();
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
    trans_c.Rotate(glm::vec3(0, dt * speed * 1.1f, 0));
  }
}

}  // namespace pickup_bob_system

#endif  // PICKUP_BOB_SYSTEM_HPP_