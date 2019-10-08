#ifndef SOUND_SYSTEM_HPP_
#define SOUND_SYSTEM_HPP_

#include <entt.hpp>
#include <slob/sound_engine.hpp>

//#include "util/global_settings.hpp"
#include "shared/shared.hpp"
#include "ecs/components.hpp"

class SoundSystem {
public:
  void Update(entt::registry& registry);
  void Init();
  slob::SoundEngine& GetSoundEngine() { return sound_engine_; }

private:
  void TriggerSound();
  void PlayFootstep(PlayerComponent& in_player_component);
  void PlayCrowd();
  void PlayBounce();

  slob::SoundEngine sound_engine_;
  slob::SoundHandle sound_step_;
};


#endif // !SOUND_SYSTEM_HPP_