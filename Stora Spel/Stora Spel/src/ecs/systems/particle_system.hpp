#ifndef PARTICLE_SYSTEM_HPP_
#define PARTICLE_SYSTEM_HPP_

#include <entity/registry.hpp>

#include <ecs\components\follow_bone_component.hpp>
#include "ecs/components.hpp"

class Engine;

class ParticleSystem {
 public:
  void Init(Engine* engine);

  void Update(entt::registry& registry, float dt);

  void ReceiveGameEvent(const GameEvent& event);

 private:
  void HandleHit(const GameEvent& event);
  void HandleShoot(const GameEvent& event);

  Engine* engine_ = nullptr;
};

#endif  // PARTICLE_SYSTEM_HPP_