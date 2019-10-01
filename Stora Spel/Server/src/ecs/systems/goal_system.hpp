#ifndef GOAL_SYSTEM_HPP_
#define GOAL_SYSTEM_HPP_

#include <boundingboxes.hpp>
#include <collision.hpp>
#include <entt.hpp>
#include <physics_object.hpp>
#include <transform_component.hpp>

#include "../components/ball_component.hpp"
#include "../components/team_component.hpp"
#include "../components/physics_component.hpp"
#include "../components/goal_component.hpp"



namespace goal_system {
void Update(entt::registry& registry) {
  auto view_balls =
      registry.view<BallComponent, TransformComponent, physics::Sphere, PhysicsComponent>();

  // get all balls
  for (auto ball : view_balls) {
    BallComponent& ball_ball_c = registry.get<BallComponent>(ball);
    TransformComponent& ball_trans_c = registry.get<TransformComponent>(ball);
    physics::Sphere& ball_sphere_c = registry.get<physics::Sphere>(ball);
    PhysicsComponent& ball_physics_c = registry.get<PhysicsComponent>(ball);

    // if ball is real, get all goals
    if (ball_ball_c.is_real) {
      auto view_goals =
          registry.view<physics::OBB, TeamCoponent, TransformComponent, GoalComponenet>();
      for (auto goal : view_goals) {
        physics::OBB& goal_OBB_c = registry.get<physics::OBB>(goal);
        TeamCoponent goal_team_c = registry.get<TeamCoponent>(goal);
        TransformComponent& goal_trans_c =
            registry.get<TransformComponent>(goal);
        GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);

        physics::IntersectData data = Intersect(ball_sphere_c, goal_OBB_c);
        if (data.collision) {
          // each team "owns" the goal where to score. 
          printf("Team %i scored a goal!\n", goal_team_c.team);
          ball_trans_c.position = glm::vec3(0.f, 0.f, 0.f);
          //ball_ball_c.is_airborne = true;
          ball_physics_c.is_airborne = true;
          ball_physics_c.velocity = glm::vec3(0, 0, 0);
          goal_goal_c.goals++;
        }
      }
    }
  }
}
}  // namespace goal_system
#endif  // GOAL_SYSTEM_HPP_