#ifndef GOAL_SYSTEM_HPP_
#define GOAL_SYSTEM_HPP_

#include <boundingboxes.hpp>
#include <collision.hpp>
#include <entt.hpp>
#include <physics_object.hpp>
#include <transform_component.hpp>

#include "../components/ball_component.hpp"
#include "../components/team_component.hpp"



namespace goal_system {
class GameServer {
 public:
  void AddScore(unsigned int TeamId){
  };
};
void Update(entt::registry& registry, GameServer* server) {
  auto view_balls =
      registry.view<BallComponent, TransformComponent, physics::Sphere>();

  // get all balls
  for (auto ball : view_balls) {
    BallComponent& ball_ball_c = registry.get<BallComponent>(ball);
    TransformComponent& ball_trans_c = registry.get<TransformComponent>(ball);
    physics::Sphere& ball_sphere_c = registry.get<physics::Sphere>(ball);

    // if ball is real, get all goals
    if (ball_ball_c.is_real) {
      auto view_goals =
          registry.view<physics::OBB, TeamCoponent, TransformComponent>();
      for (auto goal : view_goals) {
        physics::OBB& goal_OBB_c = registry.get<physics::OBB>(goal);
        TeamCoponent goal_team_c = registry.get<TeamCoponent>(goal);
        TransformComponent& goal_trans_c =
            registry.get<TransformComponent>(goal);

        physics::IntersectData data = Intersect(ball_sphere_c, goal_OBB_c);
        if (data.collision) {
          // each team "owns" the goal where to score. This is the only place it
          // is used -> no need for annoying if case or math and award points to
          // opposite team.
          server->AddScore(goal_team_c.team);
        }
      }
    }
  }
}
}  // namespace goal_system
#endif  // GOAL_SYSTEM_HPP_