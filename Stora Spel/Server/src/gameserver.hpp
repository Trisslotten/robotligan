#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared.hpp>
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init();
  void Update(float dt);

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, unsigned short id);

  void CreatePlayer(PlayerID id);
  void CreateEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void ResetEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void AddBallComponents(entt::entity& entity, glm::vec3 in_pos,
                         glm::vec3 in_vel);
  void AddArenaComponents(entt::entity& entity);

  NetAPI::Socket::Server server_;
  int last_num_players_ = 0;
  entt::registry registry_;

  std::vector<PlayerID> created_players_;
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;
  std::vector<std::string> messages;

  int test_player_guid_ = 0;
};

#endif  // GAME_SERVER_HPP_