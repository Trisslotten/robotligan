#ifndef PICKUP_BOB_SYSTEM_HPP_
#define PICKUP_BOB_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <shared/pick_up_component.hpp>
#include <shared/transform_component.hpp>

namespace pickup_bob_system {

void Update(entt::registry& registry, float dt) {

  entt::basic_view pickup_view =
      registry.view<PickUpComponent, TransformComponent>();
  for (entt::entity pickup : pickup_view) {
    PickUpComponent& pickup_c = registry.get<PickUpComponent>(pickup);
    TransformComponent& transform_c = registry.get<TransformComponent>(pickup);

    float speed = 0.6f;
    float dir = pickup_c.moving_up ? 1.0 : -1.0f;

    pickup_c.current_y_offset += dt * speed * dir;
    if (abs(pickup_c.current_y_offset) > pickup_c.move_range) {
      pickup_c.current_y_offset = pickup_c.move_range * dir;
      pickup_c.moving_up = !pickup_c.moving_up;
    }

    pickup_c.current_rotation_deg += dt * speed * 1.1;
    if (pickup_c.current_rotation_deg > 360) {
      pickup_c.current_rotation_deg -= 360;
    }

    transform_c.position += glm::vec3(0.0f, pickup_c.current_y_offset, 0.0f);
    transform_c.SetRotation(glm::vec3(0, pickup_c.current_rotation_deg, 0));

  }
}

}  // namespace pickup_bob_system

#endif  // PICKUP_BOB_SYSTEM_HPP_