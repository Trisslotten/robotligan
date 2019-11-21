#ifndef BLACK_HOLE_SYSTEM_HPP_
#define BLACK_HOLE_SYSTEM_HPP_

#include <entt.hpp>

#include "ecs/components.hpp"
#include "shared/id_component.hpp"
#include "shared/shared.hpp"
#include "shared/transform_component.hpp"
#include "util/event.hpp"
#include "util/global_settings.hpp"

namespace black_hole {
void ApplyForceOnObjects(entt::registry& registry, entt::entity black_hole, float dt);

void Update(entt::registry& registry, float dt) {
  auto view = registry.view<BlackHoleComponent, IDComponent>();

  for (auto black_hole : view) {
    auto& black_hole_c = view.get<BlackHoleComponent>(black_hole);
    auto& id_c = view.get<IDComponent>(black_hole);
    black_hole_c.time -= dt;
    if (black_hole_c.is_active == true) {
      if (black_hole_c.time < 0) {
        EventInfo e;
        e.event = Event::DESTROY_ENTITY;
        e.entity = black_hole;
        e.e_id = id_c.id;
        dispatcher.enqueue<EventInfo>(e);
        GameEvent destroy_black_hole;
        destroy_black_hole.type = GameEvent::BLACK_HOLE_DESTROYED;
        destroy_black_hole.destroy_black_hole.black_hole_id = id_c.id;
        dispatcher.trigger<GameEvent>(destroy_black_hole);
      } else {
        ApplyForceOnObjects(registry, black_hole, dt);
      }
    } else if (black_hole_c.time < 0) {
      black_hole_c.is_active = true;
      black_hole_c.time =
          GlobalSettings::Access()->ValueOf("ABILITY_BLACK_HOLE_DURATION");
      //registry.assign<physics::Sphere>(black_hole, glm::vec3(0.0f), 1.5f);
      registry.remove<PhysicsComponent>(black_hole);

      GameEvent activate_black_hole;
      activate_black_hole.type = GameEvent::BLACK_HOLE_ACTIVATED;
      activate_black_hole.activate_black_hole.black_hole_id = id_c.id;
      dispatcher.trigger<GameEvent>(activate_black_hole);
    }
  }
}

void ApplyForceOnObjects(entt::registry& registry, entt::entity black_hole, float dt) {
  auto view = registry.view<TransformComponent, PhysicsComponent>();

  for (auto object : view) {
    auto& trans_c = view.get<TransformComponent>(object);
    auto& black_hole_trans = registry.get<TransformComponent>(black_hole);
    glm::vec3 dir = black_hole_trans.position - trans_c.position;
    float length = glm::length(dir);
    float range = GlobalSettings::Access()->ValueOf("ABILITY_BLACK_HOLE_RANGE");

    if (length <= range) {
      dir = glm::normalize(dir);
      float strength =
          GlobalSettings::Access()->ValueOf("ABILITY_BLACK_HOLE_STRENGTH");
      auto& phys_c = view.get<PhysicsComponent>(object);
      phys_c.velocity += dir * strength * dt * (range - length);
    }
  }
}
};       // namespace black_hole
#endif  // BLACK_HOLE_SYSTEM_HPP_