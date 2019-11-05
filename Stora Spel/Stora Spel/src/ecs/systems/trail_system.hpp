#ifndef TRAIL_SYSTEM_HPP_
#define TRAIL_SYSTEM_HPP_

#include <entt.hpp>
#include <shared/physics_component.hpp>
#include <shared/transform_component.hpp>
#include "ecs/components/trail_component.hpp"

namespace trailsystem {

void Update(entt::registry& registry, float dt) {
  auto view = registry.view<TransformComponent, TrailComponent>();
  for (auto& entity : view) {
    auto& trans_c = view.get<TransformComponent>(entity);
    auto& trail_c = view.get<TrailComponent>(entity);

    if (trail_c.position_history.empty()) {
      for (int i = 0; i < 50; i++) {
        trail_c.position_history.push_back(trans_c.position);
      }
    } else {
      for (int i = trail_c.position_history.size() - 1; i >= 1; i--) {
        trail_c.position_history[i] = trail_c.position_history[i - 1];
      }
      trail_c.position_history[0] =
          trans_c.position + trans_c.rotation * trail_c.offset;
    }
  }

  auto view_phys =
      registry.view<TransformComponent, TrailComponent, PhysicsComponent>();
  for (auto& entity : view_phys) {
    auto& trail_c = view_phys.get<TrailComponent>(entity);
    auto& physics_c = view_phys.get<PhysicsComponent>(entity);

    float vel = length(physics_c.velocity);
    float opacity = glm::pow(0.048f * vel, 5.f);
    float f = 0.0001f;
    trail_c.color.a = glm::mix(trail_c.color.a, opacity, 1.f - glm::pow(f, dt));

    // std::cout << "vel: " << vel << "\n";
  }
}

}  // namespace trailsystem

#endif  // TRAIL_SYSTEM_HPP_