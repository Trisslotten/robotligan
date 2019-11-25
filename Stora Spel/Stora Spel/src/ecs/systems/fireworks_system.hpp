#ifndef FIREWORKS_SYSTEM_HPP_
#define FIREWORKS_SYSTEM_HPP_

#include <random>
#include <entity/registry.hpp>
#include <slob/sound_engine.hpp>
#include "ecs/components.hpp"

namespace fireworks {

void Update(entt::registry& registry, slob::SoundEngine& sound_engine, float dt) {
  auto view_fw = registry.view<FireworksComponent>();
  for (auto entity : view_fw) {
    auto& c = view_fw.get(entity);
    c.timer += dt;

    if (c.timer >= c.spawn) {
      c.timer -= c.spawn;
      auto new_rocket = registry.create();
      auto handle = glob::CreateParticleSystem();

      std::vector handles = {handle};
      std::vector<glm::vec3> offsets = {glm::vec3(0.f)};
      std::uniform_real_distribution<float> dist(-5.0, 5.0);
      std::uniform_int_distribution<int> intdist(0, c.colors.size() - 1);
      static std::default_random_engine gen;
      std::vector<glm::vec3> directions = {glm::vec3(dist(gen), 70.f, dist(gen))};

      glob::SetParticleSettings(handle, "firework.txt");
      glob::SetEmitPosition(handle, c.position);
      std::unordered_map<std::string, std::string> map;
      int i = intdist(gen);
      map["color"] = std::to_string(c.colors[i].r) + " " +
                     std::to_string(c.colors[i].g) + " " +
                     std::to_string(c.colors[i].b) + " " +
                     std::to_string(c.colors[i].a);
      glob::SetParticleSettings(handle, map);

      registry.assign<ParticleComponent>(new_rocket, handles, offsets,
                                         directions);
      registry.assign<TimerComponent>(new_rocket, 7.f);
      registry.assign<TransformComponent>(new_rocket, c.position);
      auto sound_player = sound_engine.CreatePlayer();
      registry.assign<SoundComponent>(new_rocket, sound_player);
      sound_player->Play(sound_engine.GetSound("assets/sound/fireworks.mp3"),
                         0, 5.1f);
    }
  }
}

}  // namespace fireworks

#endif  // FIREWORKS_SYSTEM_HPP_