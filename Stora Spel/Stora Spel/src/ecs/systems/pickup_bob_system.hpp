#ifndef PICKUP_BOB_SYSTEM_HPP_
#define PICKUP_BOB_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <shared/pick_up_component.hpp>
#include <shared/transform_component.hpp>

namespace pickup_bob_system {

void Update(entt::registry& registry, float dt) {
  dt = 0.0078125;

  // auto view_pickups = registry.view<PickUpComponent, TransformComponent>();
  // for (auto pickup : view_pickups) {
  //  float speed = 0.6f;
  //  auto& trans_c = registry.get<TransformComponent>(pickup);
  //  auto& pick_c = registry.get<PickUpComponent>(pickup);
  //  // y = -8.6
  //  float dir = pick_c.moving_up ? 1.0 : -1.0f;
  //  trans_c.position.y += dt * speed * dir;
  //  if (abs(trans_c.position.y - pick_c.o_pos.y) > pick_c.move_range) {
  //    pick_c.moving_up = !pick_c.moving_up;
  //  }
  //  trans_c.Rotate(glm::vec3(0, dt * speed * 1.1f, 0));
  //}

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

    pickup_c.current_rotation_deg += dt * speed * 0.2;
    if (pickup_c.current_rotation_deg > 360) {
      pickup_c.current_rotation_deg -= 360;
    }

	transform_c.position += glm::vec3(0.0f, pickup_c.current_y_offset, 0.0f);
    transform_c.Rotate(glm::vec3(0, pickup_c.current_rotation_deg, 0));

	
  }

  /* dt ~=
        0.0329773
        0.0191957
        0.0204126
        0.020605
        0.0180558
        0.0195068
        0.0174772
        0.0144295
        0.0225026
        0.0294389
        0.0206974
  */
}

}  // namespace pickup_bob_system

#endif  // PICKUP_BOB_SYSTEM_HPP_