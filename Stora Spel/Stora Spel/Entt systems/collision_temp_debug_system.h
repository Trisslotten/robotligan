#pragma once
#include "entt.hpp"
#include "physics_component.hpp"
#include "player_component.hpp"
#include "transform_component.hpp"

namespace collision_debug {
void Update(entt::registry& registry) {
  auto view_players =
      registry.view<PlayerComponent, TransformComponent, PhysicsComponent>();

  for (auto entity : view_players) {
    PhysicsComponent& physics_c = view_players.get<PhysicsComponent>(entity);
    TransformComponent& trans_c = view_players.get<TransformComponent>(entity);
    PlayerComponent& player_c = view_players.get<PlayerComponent>(entity);
	if (trans_c.position.y <= 0.f && !player_c.no_clip) {
      trans_c.position.y = 0.f;
      physics_c.velocity.y = 0.f;
      physics_c.is_airborne = false;
	}
  }
}
}  // namespace collision_debug