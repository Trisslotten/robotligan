#ifndef MISSILE_SYSTEM_HPP_
#define MISSILE_SYSTEM_HPP_

#include <ecs/components.hpp>
#include <ecs/components/ball_component.hpp>
#include <entt.hpp>
#include <shared/id_component.hpp>
#include <shared/transform_component.hpp>
#include <util/global_settings.hpp>
#include <util/timer.hpp>

namespace missile_system {
void UpdateHomingBalls(entt::registry& registry, float dt);

bool balls_are_homing = false;
Timer homing_timer;
float homing_time = 5.0f;
    

void SetBallsAreHoming(bool val) {
  balls_are_homing = val;
  homing_timer.Restart();
}

void Update(entt::registry& registry, float dt) {
  //
  homing_time = GlobalSettings::Access()->ValueOf("ABILITY_HOMING_DURATION");

  auto view_missiles =
      registry.view<MissileComponent, TransformComponent, PhysicsComponent>();

  bool exploded = false;
  for (auto missile : view_missiles) {
    auto& missile_missile_c = registry.get<MissileComponent>(missile);
    auto& missile_trans_c = registry.get<TransformComponent>(missile);
    auto& missile_phys_c = registry.get<PhysicsComponent>(missile);

    auto view_players = registry.view<PlayerComponent, TransformComponent,
                                      IDComponent, PhysicsComponent>();
    for (auto player : view_players) {
      auto& player_player_c = registry.get<PlayerComponent>(player);
      auto& player_trans_c = registry.get<TransformComponent>(player);
      auto& player_id_c = registry.get<IDComponent>(player);
      auto& player_phys_c = registry.get<PhysicsComponent>(player);

      if (player_id_c.id == missile_missile_c.target_id &&
          missile_missile_c.target_id != -1) {
        glm::vec3 diff = player_trans_c.position - missile_trans_c.position;
        glm::vec3 dir = glm::normalize(diff);

        glm::vec3 v = missile_trans_c.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 vn = glm::normalize(v);

        // rotation axis
        glm::vec3 cross = glm::cross(vn, dir);

        glm::quat q = glm::angleAxis(missile_missile_c.turn_rate * dt,
                                     glm::normalize(cross));
        missile_trans_c.rotation = q * missile_trans_c.rotation;
        missile_trans_c.rotation = glm::normalize(missile_trans_c.rotation);
      }
      glm::vec3 diff = player_trans_c.position - missile_trans_c.position;
      glm::vec3 dir = glm::normalize(diff);
      if (glm::length(diff) < missile_missile_c.detonation_dist &&
          player_player_c.client_id != missile_missile_c.creator && !exploded) {
        player_phys_c.velocity += dir * 50.f;
        player_phys_c.is_airborne = true;
        player_player_c.stunned = true;
        player_player_c.stun_timer.Restart();
        GameEvent ge;
        ge.type = GameEvent::PLAYER_STUNNED;
        if (registry.has<IDComponent>(player))
          ge.player_stunned.player_id = registry.get<IDComponent>(player).id;
        ge.player_stunned.stun_time = player_player_c.stun_time;
        dispatcher.trigger(ge);
        exploded = true;
      }
    }
    // missile_phys_c.velocity = missile_trans_c.Forward() * 0.03f;
    missile_phys_c.velocity =
        missile_trans_c.rotation * glm::vec3(missile_missile_c.speed, 0, 0);
    if (exploded) {
      // Save game event
      GameEvent missile_impact_event;
      missile_impact_event.type = GameEvent::MISSILE_IMPACT;
      missile_impact_event.missile_impact.projectile_id = registry.get<IDComponent>(missile).id;
      dispatcher.trigger(missile_impact_event);
      
      EventInfo info;
      if (registry.has<IDComponent>(missile)) {
        auto id = registry.get<IDComponent>(missile);
        info.event = Event::DESTROY_ENTITY;
        info.e_id = id.id;
        info.entity = missile;
        dispatcher.enqueue<EventInfo>(info);
	  }
      //registry.destroy(missile);
    }
  }

  if (balls_are_homing) {
    UpdateHomingBalls(registry, dt);
  }
}

void UpdateHomingBalls(entt::registry& registry, float dt) {
  auto view_balls =
      registry.view<BallComponent, PhysicsComponent, TransformComponent, IDComponent>();
  for (auto ball : view_balls) {
    auto& ball_ball_c = registry.get<BallComponent>(ball);
    auto& ball_trans_c = registry.get<TransformComponent>(ball);
    auto& ball_phys_c = registry.get<PhysicsComponent>(ball);
    auto& ball_id_c = registry.get<IDComponent>(ball);

    if (ball_ball_c.is_homing) {
      entt::entity play;
      auto view_players =
          registry.view<PlayerComponent, TransformComponent, CameraComponent>();
      for (auto player : view_players) {
        auto player_player_c = registry.get<PlayerComponent>(player);
        if (player_player_c.client_id == ball_ball_c.homer_cid) {
          play = player;
          break;
        }
      }

      auto& player_trans_c = registry.get<TransformComponent>(play);
      auto& player_camera_c = registry.get<CameraComponent>(play);

      glm::vec3 look_dir = player_camera_c.GetLookDir();
      glm::vec3 ball_travel_dir = glm::normalize(ball_phys_c.velocity);
      float ball_speed = glm::length(ball_phys_c.velocity);

      glm::vec3 combined_dir = glm::normalize(look_dir + ball_travel_dir);

      ball_phys_c.velocity = combined_dir * ball_speed;

      if (homing_timer.Elapsed() >= homing_time) {
        balls_are_homing = false;
        ball_ball_c.is_homing = false;
        ball_ball_c.homer_cid = -1;

        // Save game event
        GameEvent homing_ball_end_event;
        homing_ball_end_event.type = GameEvent::HOMING_BALL_END;
        homing_ball_end_event.homing_ball_end.ball_id = ball_id_c.id;
        dispatcher.trigger(homing_ball_end_event);
      }
    }
  }
}

}  // namespace missile_system
#endif  // !MISSILE_SYSTEM_HPP_
