#include "sound_system.hpp"
#include <shared\transform_component.hpp>
#include <shared\camera_component.hpp>


void SoundSystem::Update(entt::registry& registry)
{
  auto cam_view = registry.view<CameraComponent, TransformComponent>();
  for (auto cam_entity : cam_view) {
    CameraComponent& camera_c = cam_view.get<CameraComponent>(cam_entity);
    TransformComponent& trans_c =
      cam_view.get<TransformComponent>(cam_entity);

    // TODO: Get velocity from physics component
    sound_engine_.SetListenerAttributes(trans_c.position, camera_c.orientation, glm::vec3(0));
  }

  auto sound_view = registry.view<TransformComponent, SoundComponent>();
  for (auto sound_entity : sound_view) {
    TransformComponent& trans_c =
      sound_view.get<TransformComponent>(sound_entity);
    SoundComponent& sound_c = sound_view.get<SoundComponent>(sound_entity);

    sound_c.sound_player->Set3DAttributes(trans_c.position, glm::vec3(0));
  }

  auto player_view = registry.view<PlayerComponent, SoundComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent& player_c = player_view.get<PlayerComponent>(player_entity);
    SoundComponent& sound_c = player_view.get<SoundComponent>(player_entity);

    if (player_c.step_timer.Elapsed() > 0.5f) {
      player_c.step_timer.Restart();
      sound_c.sound_player->Play(sound_step_);
    }
  }

  // Loop over each entity with a PlayerComponent
  // and SoundComponent

  // Play footsteps from player

  // Listen for events and play associated sounds
}

void SoundSystem::Init() {
  sound_engine_.Init();
  sound_step_ = sound_engine_.GetSound("assets/sound/footstep.wav");
}
