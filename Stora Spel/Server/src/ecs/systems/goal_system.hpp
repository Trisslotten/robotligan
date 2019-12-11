#ifndef GOAL_SYSTEM_HPP_
#define GOAL_SYSTEM_HPP_
#include <boundingboxes.hpp>
#include <collision.hpp>
#include <entt.hpp>
#include <physics_object.hpp>
#include <shared/physics_component.hpp>
#include <shared/shared.hpp>
#include <shared/transform_component.hpp>

#include "ecs/components/ball_component.hpp"
#include "ecs/components/goal_component.hpp"
#include "ecs/components/team_component.hpp"
namespace goal_system {
const unsigned kDistanceForBlock = 500;
const float kTimeToSimulate = 1.5f;
bool just_blocked = false;
glm::vec3 last_blocked = glm::vec3(0.0f);
std::unordered_map<PlayerID, int> add_blocks;
void PerformSwitchGoals(entt::registry& registry) {
  auto view_goals = registry.view<GoalComponenet, TeamComponent>();
  GoalComponenet* first_goal_comp = nullptr;
  GoalComponenet* second_goal_comp = nullptr;
  bool got_first = false;
  for (auto goal : view_goals) {
    TeamComponent& goal_team_c = registry.get<TeamComponent>(goal);
    GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);

    if (goal_team_c.team == TEAM_RED) {
      goal_team_c.team = TEAM_BLUE;
    } else {
      goal_team_c.team = TEAM_RED;
    }
    if (!got_first) {
      first_goal_comp = &goal_goal_c;
      got_first = true;
    } else {
      second_goal_comp = &goal_goal_c;
    }
  }
  if (first_goal_comp != nullptr && second_goal_comp != nullptr) {
    unsigned int first_goals = first_goal_comp->goals;
    first_goal_comp->goals = second_goal_comp->goals;
    second_goal_comp->goals = first_goals;
  }
}

void Update(entt::registry& registry) {
  auto view_balls = registry.view<BallComponent, TransformComponent,
                                  physics::Sphere, PhysicsComponent>();
  just_blocked = false;
  // get all balls
  for (auto ball : view_balls) {
    BallComponent& ball_ball_c = registry.get<BallComponent>(ball);
    TransformComponent& ball_trans_c = registry.get<TransformComponent>(ball);
    physics::Sphere& ball_sphere_c = registry.get<physics::Sphere>(ball);
    PhysicsComponent& ball_physics_c = registry.get<PhysicsComponent>(ball);

    // if ball is real, get all goals
    if (ball_ball_c.is_real) {
      auto view_goals = registry.view<physics::OBB, TeamComponent,
                                      TransformComponent, GoalComponenet>();
      for (auto goal : view_goals) {
        physics::OBB& goal_OBB_c = registry.get<physics::OBB>(goal);
        TeamComponent goal_team_c = registry.get<TeamComponent>(goal);
        TransformComponent& goal_trans_c =
            registry.get<TransformComponent>(goal);
        GoalComponenet& goal_goal_c = registry.get<GoalComponenet>(goal);

        physics::IntersectData data = Intersect(ball_sphere_c, goal_OBB_c);
        auto distance = glm::distance(ball_sphere_c.center, goal_OBB_c.center);
        if (data.collision) {
          // create event for goal
          GameEvent event;
          event.type = GameEvent::GOAL;
          event.goal.x = goal_trans_c.position.x;
          dispatcher.trigger<GameEvent>(event);

          // each team "owns" the goal where to score.
          printf("Team %i scored a goal!\n", goal_team_c.team);
          goal_goal_c.goals++;
          auto view_players =
              registry.view<PlayerComponent, TeamComponent, PointsComponent>();
          // give the player who touched the ball last points
          for (auto player : view_players) {
            auto& player_player_c = registry.get<PlayerComponent>(player);
            auto& player_team_c = registry.get<TeamComponent>(player);
            auto& player_points_c = registry.get<PointsComponent>(player);

            if (player_player_c.client_id == ball_ball_c.last_touch) {
              if (goal_team_c.team == player_team_c.team) {
                // if correct goal, add POINTS_GOAL points
                player_points_c.AddPoints(POINTS_GOAL);
                player_points_c.AddGoals(1);
              } else {
                // else, self goal, subtract POINTS_GOAL
                player_points_c.AddPoints((int)(POINTS_GOAL) * -1);
                // player_points_c.AddGoals(-1); Man får inte minus mål bara för
                // att man gör självmål >:(
              }
            } else if (player_player_c.client_id == ball_ball_c.prev_touch) {
              if (goal_team_c.team == player_team_c.team) {
                player_points_c.AddPoints(POINTS_ASSIST);
                player_points_c.AddAssists(1);
              }
            }
          }
          
          return;
        } else if (distance < kDistanceForBlock) {
          auto view_players =
              registry.view<PlayerComponent, TeamComponent, PointsComponent>();
          //Simulate ball position
          //s = (v² – u²)/2a
          //v² = u² + 2as
          glm::vec3 simulated_position(1.0f);
          physics::Sphere s;
          auto goal_pos = goal_trans_c.position;
        }
      }
    }
    just_blocked = false;
  }
}
}  // namespace goal_system
#endif  // GOAL_SYSTEM_HPP_