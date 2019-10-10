#ifndef SOUND_SYSTEM_HPP_
#define SOUND_SYSTEM_HPP_

#include <entt.hpp>
#include <slob/sound_engine.hpp>

//#include "util/global_settings.hpp"
#include "shared/shared.hpp"
#include "ecs/components.hpp"
#include "util/timer.hpp"

class Engine;

class SoundSystem {
public:
  void Update(entt::registry& registry);
  void Init(Engine* engine);
  void PlayAmbientSound(entt::registry& registry);
  slob::SoundEngine& GetSoundEngine() { return sound_engine_; }

  void ReceiveGameEvent(const GameEvent& event);
private:
  Timer nudge_timer_;

  slob::SoundEngine sound_engine_;

  slob::SoundHandle sound_step_;
  slob::SoundHandle sound_crowd_;
  slob::SoundHandle sound_kick_;
  slob::SoundHandle sound_hit_;
  slob::SoundHandle sound_nudge_;
  slob::SoundHandle sound_goal_;
  slob::SoundHandle sound_ball_bounce_;

  Engine* engine_;
};


#endif // !SOUND_SYSTEM_HPP_