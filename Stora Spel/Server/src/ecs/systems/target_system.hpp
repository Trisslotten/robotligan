#ifndef TARGET_SYSTEM_HPP_
#define TARGET_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entt.hpp>
#include <shared/camera_component.hpp>
#include <shared/transform_component.hpp>
#include <shared/id_component.hpp>

namespace target_system {

void Update(entt::registry& registry) {
  auto view_players =
      registry.view<PlayerComponent, TeamComponent, AbilityComponent,
                    TransformComponent, CameraComponent>();

  for (auto player : view_players) {
    //
    auto& player_c = registry.get<PlayerComponent>(player);
    auto& ability_c = registry.get<AbilityComponent>(player);
    auto& team_c = registry.get<TeamComponent>(player);
    auto& trans_c = registry.get<TransformComponent>(player);
    auto& cam_c = registry.get<CameraComponent>(player);

    if (ability_c.primary_ability == AbilityID::MISSILE ||
        ability_c.secondary_ability == AbilityID::MISSILE) {
      auto view_others =
          registry.view<PlayerComponent, TeamComponent, TransformComponent, IDComponent>();

      float best_score = 0;
      EntityID best_ent = -1;
      for (auto other : view_others) {
        auto& other_player_c = registry.get<PlayerComponent>(other);
        auto& other_team_c = registry.get<TeamComponent>(other);
        auto& other_trans_c = registry.get<TransformComponent>(other);
        auto& other_id_c = registry.get<IDComponent>(other);

        glm::vec3 diff = other_trans_c.position - trans_c.position;
        glm::vec3 dir = glm::normalize(diff);
        float dot = glm::dot(dir, cam_c.GetLookDir());

        if (dot > 0.6f) {
          float dist = glm::length(diff);
          float dist_points = 20 - dist;
          float score = dot * 10 + dist_points;
          if (score > best_score) {
            best_score = score;
            player_c.target = other_id_c.id;
		  }
        }
      }
    }
  }
}

}  // namespace target_system
#endif  // TARGET_SYSTEM_HPP_
