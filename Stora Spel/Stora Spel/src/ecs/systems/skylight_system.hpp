#ifndef SKYLIGHT_SYSTEM_HPP_
#define SKYLIGHT_SYSTEM_HPP_

#include <entt.hpp>
#include <shared/transform_component.hpp>

#include "ecs/components/skylight_component.hpp"

namespace skylight_system {

void Update(entt::registry& registry) {
  auto view = registry.view<TransformComponent, SkyLightComponent>();
  for (auto entity : view) {
    auto& trans_c = view.get<TransformComponent>(entity);
    auto& skylight_c = view.get<SkyLightComponent>(entity);

    glm::quat look_at_rot =
        glm::quatLookAt(glm::normalize(-trans_c.position), glm::vec3(0, 1, 0)) *
        glm::quat(glm::vec3(0, 3.1415 / 2., 0));
    glm::quat base_rot = glm::quat(glm::vec3(0, 0, -3.1415 / 2));

    // animation
    float time = skylight_c.timer.Elapsed();
    float sign = glm::sign(trans_c.position.z) * glm::sign(trans_c.position.x);
    glm::quat wiggle(glm::vec3(0.2f*sign*glm::sin(time), 0, 0));

    trans_c.rotation = look_at_rot * wiggle * base_rot;
  }
}

}  // namespace skylight_system

#endif  // SKYLIGHT_SYSTEM_HPP_
