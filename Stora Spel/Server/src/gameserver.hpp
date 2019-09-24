#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init();
  void Update(float dt);

 private:
  void UpdateSystems(float dt);

  void CreateEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void ResetEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void AddBallComponents(entt::entity& entity, glm::vec3 in_pos,
                         glm::vec3 in_vel);
  void AddArenaComponents(entt::entity& entity);
  void AddPlayerComponents(entt::entity& entity);
  void AddRobotComponents(entt::entity& entity, glm::vec3 in_pos);

  NetAPI::Socket::Server server_;
  entt::registry registry_;

  int test_player_guid = 0;
};

#endif  // GAME_SERVER_HPP_