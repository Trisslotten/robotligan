#ifndef FISHING_SYSTEM_HPP_
#define FISHING_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <entt.hpp>
#include <shared/id_component.hpp>
#include <shared/transform_component.hpp>

namespace fishing_system {

void Update(entt::registry& registry, float dt) {
  auto view_hooks = registry.view<HookComponent, TransformComponent,
                                  PhysicsComponent, IDComponent>();
  for (auto hook : view_hooks) {
    auto& hook_hook_c = registry.get<HookComponent>(hook);
    auto& hook_trans_c = registry.get<TransformComponent>(hook);
    auto& hook_phys_c = registry.get<PhysicsComponent>(hook);
    auto& hook_id_c = registry.get<IDComponent>(hook);

    if (hook_hook_c.attached) {
      bool should_remove = false;
      // PULL THE PLAYER TOWARDS THE HOOK POINT (if hooked to arena)
      if (hook_hook_c.hook_m == PULL_PLAYER) {
        TransformComponent* owner_trans_c = nullptr;
        PhysicsComponent* owner_phys_c = nullptr;

        glm::vec3 hook_point = glm::vec3(0);

        bool found_owner = false;
        auto view_entities =
            registry.view<TransformComponent, PhysicsComponent, IDComponent>();
        for (auto entity : view_entities) {
          auto& hooked_id_c = registry.get<IDComponent>(entity);
          if (hooked_id_c.id == hook_hook_c.owner) {
            owner_trans_c = &registry.get<TransformComponent>(entity);
            owner_phys_c = &registry.get<PhysicsComponent>(entity);
            found_owner = true;
            break;
          }
        }
        if (found_owner) {
          glm::vec3 dist = hook_trans_c.position - owner_trans_c->position;
          glm::vec3 dir = glm::normalize(dist);

          glm::vec3 owner_look_dir = owner_trans_c->Forward();
          glm::vec3 point_to_travel = owner_look_dir * glm::length(dist);

          // glm::vec3 travel_dir = glm::normalize(point_to_travel -
          // hook_point); owner_phys_c->velocity += travel_dir * dt;
          owner_phys_c->velocity += dir * hook_hook_c.pull_speed * dt;
          if (glm::length(dist) < 0.2f) should_remove = true;
        }
      }

      // PULL OBJECT TOWARDS PLAYER (if hooked a ball or other player)
      if (hook_hook_c.hook_m == PULL_OBJECT) {
        TransformComponent* hooked_trans_c = nullptr;
        PhysicsComponent* hooked_phys_c = nullptr;
        TransformComponent* owner_trans_c = nullptr;
        PhysicsComponent* owner_phys_c = nullptr;

        glm::vec3 hook_point = glm::vec3(0);
        float collider_size = 0.f;

        bool found_hooked = false;
        bool found_owner = false;
        auto view_entities =
            registry.view<TransformComponent, PhysicsComponent, IDComponent>();
        for (auto entity : view_entities) {
          auto& hooked_id_c = registry.get<IDComponent>(entity);
          if (hooked_id_c.id == hook_hook_c.hooked_entity) {
            hooked_trans_c = &registry.get<TransformComponent>(entity);
            hooked_phys_c = &registry.get<PhysicsComponent>(entity);
            found_hooked = true;
            if (registry.has<physics::Sphere>(entity)) {
              collider_size = registry.get<physics::Sphere>(entity).radius;
            }
          }
          if (hooked_id_c.id == hook_hook_c.owner) {
            owner_trans_c = &registry.get<TransformComponent>(entity);
            owner_phys_c = &registry.get<PhysicsComponent>(entity);
            found_owner = true;
          }
          if (found_hooked && found_owner) {
            break;
          }
        }
        if (found_hooked && found_owner) {
          glm::vec3 dist = hooked_trans_c->position - owner_trans_c->position;
          glm::vec3 dir = glm::normalize(dist);
          glm::vec3 hook_point = hooked_trans_c->position - dir * collider_size;

          hook_trans_c.position = hook_point;

          glm::vec3 owner_look_dir = owner_trans_c->Forward();
          glm::vec3 point_to_travel = owner_look_dir * glm::length(dist);

          glm::vec3 travel_dir = glm::normalize(point_to_travel - hook_point);
          hooked_phys_c->velocity += travel_dir * dt;
          hooked_phys_c->velocity += -dir * hook_hook_c.pull_speed * dt;
          if (glm::length(dist) < 0.2f) should_remove = true;
        }
      }

      // check if timer has run out
      if (hook_hook_c.hook_timer.Elapsed() >= hook_hook_c.hook_time ||
          should_remove) {
        GameEvent ge;
        ge.hook_removed.hook_id = hook_id_c.id;
        ge.type = GameEvent::REMOVE_FISHING_HOOK;
        dispatcher.trigger(ge);
        registry.destroy(hook);
      }
    }
  }
}

}  // namespace fishing_system

#endif  // FISHING_SYSTEM_HPP_
