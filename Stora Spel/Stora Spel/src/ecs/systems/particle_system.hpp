#ifndef PARTICLE_SYSTEM_HPP_
#define PARTICLE_SYSTEM_HPP_

#include <entity/registry.hpp>

#include "ecs/components.hpp"

void ParticleSystem(entt::registry& registry, float dt) {
  auto view_particle = registry.view<ParticleComponent>();
  for (auto& entity : view_particle) {
    auto& particle_c = view_particle.get(entity);

    for (int i = 0; i < particle_c.handles.size(); ++i) {
      glob::UpdateParticles(particle_c.handles[i], dt);
    }
  }
}
#endif  // PARTICLE_SYSTEM_HPP_