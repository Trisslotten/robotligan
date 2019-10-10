#include "sound_system.hpp"
#include <shared/transform_component.hpp>
#include <shared/camera_component.hpp>
#include "engine.hpp"
#include <shared/id_component.hpp>
#include <shared/physics_component.hpp>


void SoundSystem::Update(entt::registry& registry) {
  sound_engine_.Update();

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
  auto player_view = registry.view<PlayerComponent, SoundComponent, PhysicsComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent& player_c = player_view.get<PlayerComponent>(player_entity);
    SoundComponent& sound_c = player_view.get<SoundComponent>(player_entity);
    PhysicsComponent& physics_c = player_view.get<PhysicsComponent>(player_entity);

    if (player_c.step_timer.Elapsed() > 0.5f && glm::length(physics_c.velocity) > 0.0f && !physics_c.is_airborne) {
      player_c.step_timer.Restart();
      sound_c.sound_player->Play(sound_step_, 0, 0.1f);
    }
  }
}

void SoundSystem::Init(Engine* engine) {
  engine_ = engine;
  sound_engine_.Init();
  sound_step_ = sound_engine_.GetSound("assets/sound/footstep.wav");
  sound_crowd_ = sound_engine_.GetSound("assets/sound/crowd.mp3");
  sound_kick_ = sound_engine_.GetSound("assets/sound/kick_swing.mp3");
  sound_hit_ = sound_engine_.GetSound("assets/sound/ball_hit_sound.mp3");
  sound_nudge_ = sound_engine_.GetSound("assets/sound/ball_nudge.mp3");
  sound_goal_ = sound_engine_.GetSound("assets/sound/goal.mp3");
  sound_ball_bounce_ = sound_engine_.GetSound("assets/sound/bounce.mp3");
}

void SoundSystem::PlayAmbientSound(entt::registry& registry) {
  // Play static sounds (music, ambient etc)
  auto local_view = registry.view<CameraComponent, SoundComponent>();
  for (auto local_entity : local_view) {
    auto& cam_c = local_view.get<CameraComponent>(local_entity);
    auto& sound_c = local_view.get<SoundComponent>(local_entity);

    sound_c.sound_player->Play(sound_crowd_, -1, 0.2f);
    break;
  }
}

void SoundSystem::ReceiveGameEvent(const GameEvent& event)
{
  auto registry = engine_->GetCurrentRegistry();

  // Listen for events and play associated sounds
  switch (event.type) {
  case GameEvent::GOAL: {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(sound_goal_, 0, 0.5);
      break;
    }
    break;
  }
  case GameEvent::KICK: {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.kick.player_id) {
        sound_c.sound_player->Play(sound_kick_);
        break;
      }
    }
    break;
  }
  case GameEvent::HIT: {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.hit.player_id) {
        sound_c.sound_player->Play(sound_hit_);
        break;
      }
    }
    break;
  }
  case GameEvent::NUDGE: {
    auto view = registry->view<IDComponent, BallComponent, SoundComponent>();
    for (auto entity : view) {
      auto ball_c = view.get<BallComponent>(entity);
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.nudge.ball_id && nudge_timer_.Elapsed() > 0.1f) {
        nudge_timer_.Restart();
        sound_c.sound_player->Play(sound_nudge_, 0, 1.0f);
        break;
      }
    }
    break;
  }
  case GameEvent::BOUNCE: {
    auto view = registry->view<IDComponent, BallComponent, SoundComponent>();
    for (auto entity : view) {
      auto ball_c = view.get<BallComponent>(entity);
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.nudge.ball_id) {
        sound_c.sound_player->Play(sound_ball_bounce_);
        break;
      }
    }
    break;
  }
  }
}
