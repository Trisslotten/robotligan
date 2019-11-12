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

    float min_history_time = 1.f / 200.f;
    if (trail_c.history.empty()) {
      for (int i = 0; i < 200; i++) {
        trail_c.history.push_back({trans_c.position});
      }
    } else {
      trail_c.accum_dt += dt;
      if (trail_c.accum_dt >= min_history_time) {
        for (int i = trail_c.history.size() - 1; i >= 1; i--) {
          trail_c.history[i] = trail_c.history[i - 1];
        }
        trail_c.history[0] = {
            trans_c.position + trans_c.rotation * trail_c.offset,
            trail_c.accum_dt};
        trail_c.accum_dt = 0.f;
      }
    }

    if (trail_c.positions.empty()) {
      // TODO: check limit
      for (int i = 0; i < trail_c.length; i++) {
        trail_c.positions.push_back(trans_c.position);
      }
    } else {
      // trail_c.positions[0] = trail_c.history[0].position;

      float time_per_pos = 0.016f;

      float accum_time = 0.f;
      int hist_index = 0;
      for (int i = 0; i < trail_c.positions.size() &&
                      hist_index < trail_c.history.size() - 1;
           i++) {
        auto& curr_hist = trail_c.history[hist_index];
        auto& next_hist = trail_c.history[hist_index + 1];

        float t = accum_time / curr_hist.dt;
        trail_c.positions[i] = glm::mix(curr_hist.position, next_hist.position,
                                        glm::smoothstep(0.f, 1.f, t));
        accum_time += time_per_pos;
        if (accum_time >= curr_hist.dt) {
          accum_time -= curr_hist.dt;
          hist_index++;
        }
      }
    }
  }

  auto view_phys =
      registry.view<TransformComponent, TrailComponent, PhysicsComponent>();
  for (auto& entity : view_phys) {
    auto& trail_c = view_phys.get<TrailComponent>(entity);
    auto& physics_c = view_phys.get<PhysicsComponent>(entity);

    float vel = length(physics_c.velocity);
    float opacity = glm::pow(0.048f * vel, 5.f);
    float f = 0.01f;
    trail_c.color.a = glm::mix(trail_c.color.a, opacity, 1.f - glm::pow(f, dt));
  }
}

}  // namespace trailsystem

#endif  // TRAIL_SYSTEM_HPP_