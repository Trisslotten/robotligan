#include "sound_system.hpp"
#include <shared\transform_component.hpp>
#include <shared\camera_component.hpp>


void SoundSystem::Update(entt::registry& registry)  {
  // Set listener attributes to match the local player
  auto cam_view = registry.view<CameraComponent, TransformComponent>();
  for (auto cam_entity : cam_view) {
    CameraComponent& camera_c = cam_view.get<CameraComponent>(cam_entity);
    TransformComponent& trans_c =
      cam_view.get<TransformComponent>(cam_entity);

    // TODO: Get velocity and airbourne bool-check from physics component
    sound_engine_.SetListenerAttributes(trans_c.position, camera_c.orientation, glm::vec3(0));
  }

  // Set 3D space attributes for sounds coming from each object/player on the field
  auto sound_view = registry.view<TransformComponent, SoundComponent>();
  for (auto sound_entity : sound_view) {
    TransformComponent& trans_c =
      sound_view.get<TransformComponent>(sound_entity);
    SoundComponent& sound_c = sound_view.get<SoundComponent>(sound_entity);

    sound_c.sound_player->Set3DAttributes(trans_c.position, glm::vec3(0));
  }

  // Play footstep sounds from each player on the field
  auto player_view = registry.view<PlayerComponent, SoundComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent& player_c = player_view.get<PlayerComponent>(player_entity);
    SoundComponent& sound_c = player_view.get<SoundComponent>(player_entity);

    if (player_c.step_timer.Elapsed() > 0.5f /*&& physics_c.velocity > 0.0f && !physics_c.is_airborne*/) {
      player_c.step_timer.Restart();
      sound_c.sound_player->Play(sound_step_, 0, 0.1f);
    }
  }

  // Listen for events and play associated sounds

}

void SoundSystem::Init() {
  sound_engine_.Init();
  sound_step_ = sound_engine_.GetSound("assets/sound/footstep.wav");
  sound_crowd_ = sound_engine_.GetSound("assets/sound/crowd.mp3");
}

void SoundSystem::PlayStaticSound(entt::registry& registry) {
  // Play static sounds (music, ambient etc)
  auto local_view = registry.view<CameraComponent, SoundComponent>();
  for (auto local_entity : local_view) {
    CameraComponent& cam_c = local_view.get<CameraComponent>(local_entity);
    SoundComponent& sound_c = local_view.get<SoundComponent>(local_entity);

    sound_c.sound_player->Play(sound_crowd_, -1, 0.3f);
  }
}
