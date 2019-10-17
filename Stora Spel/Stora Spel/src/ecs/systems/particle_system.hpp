#ifndef PARTICLE_SYSTEM_HPP_
#define PARTICLE_SYSTEM_HPP_

#include <entity/registry.hpp>

#include "ecs/components.hpp"

void ParticleSystem(entt::registry& registry, float dt) {
  auto view_movable = registry.view<ParticleComponent, TransformComponent>();
  for (auto& entity : view_movable) {
    auto& particle_c = view_movable.get<ParticleComponent>(entity);
    auto& transform_c = view_movable.get<TransformComponent>(entity);

    for (int i = 0; i < particle_c.handles.size(); ++i) {
      glob::SetEmitPosition(particle_c.handles[i],
                            transform_c.position + particle_c.offsets[i]);
      glob::SetParticleDirection(
          particle_c.handles[i],
          transform_c.rotation * particle_c.directions[i]);
    }
  }

  auto view_particle = registry.view<ParticleComponent>();
  for (auto& entity : view_particle) {
    auto& particle_c = view_particle.get(entity);

    for (int i = 0; i < particle_c.handles.size(); ++i) {
      glob::UpdateParticles(particle_c.handles[i], dt);
    }
  }
}
#endif  // PARTICLE_SYSTEM_HPP_