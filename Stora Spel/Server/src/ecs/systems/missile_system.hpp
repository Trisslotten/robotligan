#ifndef MISSILE_SYSTEM_HPP_
#define MISSILE_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entt.hpp>
#include <shared/id_component.hpp>
#include <shared/transform_component.hpp>

namespace missile_system {
void Update(entt::registry& registry, float dt) {
  //
  auto view_missiles =
      registry.view<MissileComponent, TransformComponent, PhysicsComponent>();

  bool exploded = false;
  for (auto missile : view_missiles) {
    auto& missile_missile_c = registry.get<MissileComponent>(missile);
    auto& missile_trans_c = registry.get<TransformComponent>(missile);
    auto& missile_phys_c = registry.get<PhysicsComponent>(missile);

    auto view_players = registry.view<PlayerComponent, TransformComponent,
                                      IDComponent, PhysicsComponent>();
    for (auto player : view_players) {
      auto& player_player_c = registry.get<PlayerComponent>(player);
      auto& player_trans_c = registry.get<TransformComponent>(player);
      auto& player_id_c = registry.get<IDComponent>(player);
      auto& player_phys_c = registry.get<PhysicsComponent>(player);

      if (player_id_c.id == missile_missile_c.target_id &&
          missile_missile_c.target_id != -1) {
        glm::vec3 diff = player_trans_c.position - missile_trans_c.position;
        glm::vec3 dir = glm::normalize(diff);

        glm::vec3 v = missile_trans_c.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 vn = glm::normalize(v);

		//rotation axis
        glm::vec3 cross = glm::cross(vn, dir);

        glm::quat q = glm::angleAxis(missile_missile_c.turn_rate * dt,
                                     glm::normalize(cross));
        missile_trans_c.rotation = q * missile_trans_c.rotation;
        missile_trans_c.rotation = glm::normalize(missile_trans_c.rotation);
      }
      glm::vec3 diff = player_trans_c.position - missile_trans_c.position;
      glm::vec3 dir = glm::normalize(diff);
      if (glm::length(diff) < missile_missile_c.detonation_dist &&
          player_player_c.client_id != missile_missile_c.creator) {
        player_phys_c.velocity += dir * 10.f;
        player_phys_c.is_airborne = true;
        exploded = true;
      }
    }
    // missile_phys_c.velocity = missile_trans_c.Forward() * 0.03f;
    missile_phys_c.velocity =
        missile_trans_c.rotation * glm::vec3(missile_missile_c.speed, 0, 0);
    if (exploded) {
      EventInfo info;
      if (registry.has<IDComponent>(missile) == false) return;
      auto id = registry.get<IDComponent>(missile);
      info.event = Event::DESTROY_ENTITY;
      info.e_id = id.id;
      dispatcher.enqueue<EventInfo>(info);
      registry.destroy(missile);
    }
  }
}
}  // namespace missile_system
#endif  // !MISSILE_SYSTEM_HPP_
