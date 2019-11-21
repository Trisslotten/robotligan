#include "sound_system.hpp"
#include <shared/camera_component.hpp>
#include <shared/id_component.hpp>
#include <shared/physics_component.hpp>
#include <shared/transform_component.hpp>
#include "engine.hpp"

void SoundSystem::Update(entt::registry& registry) {
  sound_engine_.Update();

  // Set listener attributes to match the local player
  auto cam_view =
      registry.view<CameraComponent, TransformComponent, PhysicsComponent>();
  for (auto cam_entity : cam_view) {
    CameraComponent& camera_c = cam_view.get<CameraComponent>(cam_entity);
    TransformComponent& trans_c = cam_view.get<TransformComponent>(cam_entity);
    PhysicsComponent& phys_c = cam_view.get<PhysicsComponent>(cam_entity);

    sound_engine_.SetListenerAttributes(trans_c.position, camera_c.orientation,
                                        phys_c.velocity);
  }

  // Set 3D space attributes for sounds coming from each object/player on the
  // field
  auto sound_view = registry.view<TransformComponent, SoundComponent>();
  for (auto sound_entity : sound_view) {
    TransformComponent& trans_c =
        sound_view.get<TransformComponent>(sound_entity);
    SoundComponent& sound_c = sound_view.get<SoundComponent>(sound_entity);
    glm::vec3 vel = glm::vec3(0.f);

    if (registry.has<PhysicsComponent>(sound_entity)) {
      PhysicsComponent& phys_c = registry.get<PhysicsComponent>(sound_entity);
      vel = phys_c.velocity;
    }

    sound_c.sound_player->Set3DAttributes(trans_c.position, vel);
  }
  // Play footstep sounds from each player on the field
  auto player_view =
      registry.view<PlayerComponent, SoundComponent, PhysicsComponent>();
  for (auto player_entity : player_view) {
    PlayerComponent& player_c = player_view.get<PlayerComponent>(player_entity);
    SoundComponent& sound_c = player_view.get<SoundComponent>(player_entity);
    PhysicsComponent& physics_c =
        player_view.get<PhysicsComponent>(player_entity);

    if (player_c.step_timer.Elapsed() > 0.5f &&
        glm::length(physics_c.velocity) > 1.0f && !physics_c.is_airborne) {
      player_c.step_timer.Restart();
      sound_c.sound_player->Play(sound_step_, 0, 0.2f);
    }
  }
}

void SoundSystem::Init(Engine* engine) {
  engine_ = engine;
  sound_engine_.Init();

  sound_engine_.CreatePlayer();

  button_hover_ = sound_engine_.GetSound("assets/sound/button_hover.mp3");
  button_click_ = sound_engine_.GetSound("assets/sound/button_click.mp3");

  sound_step_ = sound_engine_.GetSound("assets/sound/footstep.wav");
  sound_crowd_ = sound_engine_.GetSound("assets/sound/crowd.mp3");
  sound_woosh_ = sound_engine_.GetSound("assets/sound/kick_swing.mp3");
  sound_hit_ = sound_engine_.GetSound("assets/sound/ball_hit_sound.mp3");
  sound_nudge_ = sound_engine_.GetSound("assets/sound/ball_nudge.mp3");
  sound_goal_ = sound_engine_.GetSound("assets/sound/goal.mp3");
  sound_ball_bounce_ = sound_engine_.GetSound("assets/sound/bounce.mp3");
  sound_player_land_ = sound_engine_.GetSound("assets/sound/robot_land.mp3");
  sound_player_jump_ = sound_engine_.GetSound("assets/sound/robot_jump.mp3");
  sound_ability_missile_impact_ =
      sound_engine_.GetSound("assets/sound/missile_explode.wav");
  sound_ability_teleport_impact_ =
      sound_engine_.GetSound("assets/sound/teleport_2.mp3");
  sound_ability_force_push_impact_ =
      sound_engine_.GetSound("assets/sound/forcepush.mp3");
  sound_ability_fake_ball_poof_ =
      sound_engine_.GetSound("assets/sound/poof.mp3");
  sound_ability_invisibility_end_ =
      sound_engine_.GetSound("assets/sound/invis_end.mp3");
  sound_ability_blackout_end_ =
      sound_engine_.GetSound("assets/sound/blackout_end.mp3");
  sound_ability_mine_trigger_ =
      sound_engine_.GetSound("assets/sound/mine_trigger.mp3");

  sound_pickup_spawned_ = sound_engine_.GetSound("assets/sound/pickup.wav");
  sound_player_stunned_ = sound_engine_.GetSound("assets/sound/stunned.mp3");

  ability_sounds_[AbilityID::GRAVITY_CHANGE] =
      sound_engine_.GetSound("assets/sound/gravitydrop.wav");
  ability_sounds_[AbilityID::SUPER_STRIKE] =
      sound_engine_.GetSound("assets/sound/superkick.mp3");
  ability_sounds_[AbilityID::MISSILE] =
      sound_engine_.GetSound("assets/sound/missile_fire.mp3");
  ability_sounds_[AbilityID::TELEPORT] =
      sound_engine_.GetSound("assets/sound/teleport_1.mp3");
  ability_sounds_[AbilityID::HOMING_BALL] =
      sound_engine_.GetSound("assets/sound/homingball.wav");
  ability_sounds_[AbilityID::SWITCH_GOALS] =
      sound_engine_.GetSound("assets/sound/switch_goals.mp3");
  ability_sounds_[AbilityID::BUILD_WALL] =
      sound_engine_.GetSound("assets/sound/build_wall.mp3");
  ability_sounds_[AbilityID::FAKE_BALL] =
      sound_engine_.GetSound("assets/sound/fake_ball.wav");
  ability_sounds_[AbilityID::INVISIBILITY] =
      sound_engine_.GetSound("assets/sound/invis_pop.mp3");
  ability_sounds_[AbilityID::BLACKOUT] =
      sound_engine_.GetSound("assets/sound/blackout_start.mp3");
  ability_sounds_[AbilityID::MINE] =
      sound_engine_.GetSound("assets/sound/place_mine.mp3");
}

void SoundSystem::PlayAmbientSound(entt::registry& registry) {
  // Play static sounds (music, ambient etc)
  auto local_view = registry.view<CameraComponent, SoundComponent>();
  for (auto local_entity : local_view) {
    auto& cam_c = local_view.get<CameraComponent>(local_entity);
    auto& sound_c = local_view.get<SoundComponent>(local_entity);

    sound_c.sound_player->Play(sound_crowd_, -1, 0.1f);
    break;
  }
}

void SoundSystem::ReceiveGameEvent(const GameEvent& event) {
  auto registry = engine_->GetCurrentRegistry();
  // Listen for GameEvents and play associated sounds

  if (event.type == GameEvent::GOAL) {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(sound_goal_, 0, 0.1f);
      break;
    }
  }
  if (event.type == GameEvent::KICK) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.kick.player_id) {
        sound_c.sound_player->Play(sound_woosh_, 0, 1.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::HIT) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.hit.player_id) {
        sound_c.sound_player->Play(sound_hit_, 0, 0.5f);
        break;
      }
    }
  }
  if (event.type == GameEvent::NUDGE) {
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
  }
  if (event.type == GameEvent::BOUNCE) {
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
  }
  if (event.type == GameEvent::LAND) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.land.player_id) {
        sound_c.sound_player->Play(sound_player_land_, 0, 0.05f);
        break;
      }
    }
  }
  if (event.type == GameEvent::JUMP) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.jump.player_id) {
        sound_c.sound_player->Play(sound_player_jump_, 0, 0.05f);
        break;
      }
    }
  }
  if (event.type == GameEvent::GRAVITY_DROP) {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(ability_sounds_[AbilityID::GRAVITY_CHANGE], 0,
                                 0.6f);
      break;
    }
  }
  if (event.type == GameEvent::SUPER_KICK) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.super_kick.player_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::SUPER_STRIKE], 0,
                                   0.2f);
        break;
      }
    }
  }
  if (event.type == GameEvent::MISSILE_FIRE) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      // Keep cout here for later use when solving order of entity
      // creaton/deletion and game event triggers std::cout << "entity id: " <<
      // id_c.id << std::endl << "proj id: " <<
      // event.missile_fire.projectile_id << std::endl;
      if (id_c.id == event.missile_fire.projectile_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::MISSILE], 0,
                                   2.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::MISSILE_IMPACT) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      // Keep cout here for later use when solving order of entity
      // creaton/deletion and game event triggers std::cout << "entity id: " <<
      // id_c.id << std::endl << " " <<
      // event.missile_impact.projectile_id << std::endl;
      if (id_c.id == event.missile_impact.projectile_id) {
        sound_c.sound_player->Stop(ability_sounds_[AbilityID::MISSILE]);
        sound_c.sound_player->Play(sound_ability_missile_impact_, 0, 3.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::TELEPORT_CAST) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.teleport_cast.player_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::TELEPORT], 0,
                                   0.4f);
        break;
      }
    }
  }
  if (event.type == GameEvent::TELEPORT_IMPACT) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.teleport_cast.player_id) {
        sound_c.sound_player->Play(sound_ability_teleport_impact_, 0, 0.4f);
        break;
      }
    }
  }
  if (event.type == GameEvent::HOMING_BALL) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.homing_ball.ball_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::HOMING_BALL], 0,
                                   10.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::FORCE_PUSH) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.force_push.player_id) {
        sound_c.sound_player->Play(sound_woosh_);
        break;
      }
    }
  }
  if (event.type == GameEvent::FORCE_PUSH_IMPACT) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.force_push_impact.projectile_id) {
        sound_c.sound_player->Play(sound_ability_force_push_impact_, 0, 2.5f);
        break;
      }
    }
  }
  if (event.type == GameEvent::SWITCH_GOALS) {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(ability_sounds_[AbilityID::SWITCH_GOALS], 0,
                                 0.3f);
      break;
    }
  }
  if (event.type == GameEvent::SWITCH_GOALS_DONE) {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(ability_sounds_[AbilityID::SWITCH_GOALS], 0,
                                 0.3f);
      break;
    }
  }
  if (event.type == GameEvent::BUILD_WALL) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.build_wall.wall_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::BUILD_WALL], 0,
                                   10.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::FAKE_BALL_CREATED) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.fake_ball_created.ball_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::FAKE_BALL]);
        break;
      }
    }
  }
  if (event.type == GameEvent::FAKE_BALL_POOF) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.fake_ball_poofed.ball_id) {
        sound_c.sound_player->Play(sound_ability_fake_ball_poof_);
        break;
      }
    }
  }
  if (event.type == GameEvent::INVISIBILITY_CAST) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.invisibility_cast.player_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::INVISIBILITY], 0,
                                   0.5f);
        break;
      }
    }
  }
  if (event.type == GameEvent::INVISIBILITY_END) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.invisibility_end.player_id) {
        sound_c.sound_player->Play(sound_ability_invisibility_end_, 0, 0.5f);
        break;
      }
    }
  }
  if (event.type == GameEvent::BLACKOUT_CAST) {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(ability_sounds_[AbilityID::BLACKOUT], 0, 0.7);
      break;
    }
  }
  if (event.type == GameEvent::BLACKOUT_END) {
    auto view = registry->view<CameraComponent, SoundComponent>();
    for (auto entity : view) {
      auto& sound_c = view.get<SoundComponent>(entity);
      sound_c.sound_player->Play(sound_ability_blackout_end_);
      break;
    }
  }
  if (event.type == GameEvent::PICKUP_SPAWNED) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.super_kick.player_id) {
        sound_c.sound_player->Play(sound_pickup_spawned_, 0, 1.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::MINE_PLACE) {
    auto view = registry->view<IDComponent, MineComponent, SoundComponent>();
    for (auto entity : view) {
      IDComponent& id_c = view.get<IDComponent>(entity);
      MineComponent& mine_c = view.get<MineComponent>(entity);
      SoundComponent& sound_c = view.get<SoundComponent>(entity);

      if (id_c.id == event.mine_place.entity_id) {
        sound_c.sound_player->Play(ability_sounds_[AbilityID::MINE]);
        break;
      }
    }
  }
  if (event.type == GameEvent::MINE_TRIGGER) {
    auto view = registry->view<IDComponent, MineComponent, SoundComponent>();
    for (auto entity : view) {
      IDComponent& id_c = view.get<IDComponent>(entity);
      MineComponent& mine_c = view.get<MineComponent>(entity);
      SoundComponent& sound_c = view.get<SoundComponent>(entity);

      if (id_c.id == event.mine_trigger.entity_id) {
        sound_c.sound_player->Play(sound_ability_mine_trigger_, 0, 2.0f);
        break;
      }
    }
  }
  if (event.type == GameEvent::PLAYER_STUNNED) {
    auto view = registry->view<IDComponent, SoundComponent>();
    for (auto entity : view) {
      auto& id_c = view.get<IDComponent>(entity);
      auto& sound_c = view.get<SoundComponent>(entity);
      if (id_c.id == event.player_stunned.player_id) {
        sound_c.sound_player->Play(sound_player_stunned_);
        break;
      }
    }
  }
}

void SoundSystem::ReceiveMenuEvent(const MenuEvent& event) {
  auto registry = engine_->GetCurrentRegistry();

  // Listen for GameEvents and play associated sounds
  switch (event.type) {
    case MenuEvent::HOVER: {
      // Play hover sound
      sound_engine_.GetPlayer()->Play(button_hover_);
      break;
    }
    case MenuEvent::CLICK: {
      // Play click sound
      sound_engine_.GetPlayer()->Play(button_click_);
      break;
    }
  }
}