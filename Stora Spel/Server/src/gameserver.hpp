#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared.hpp>
#include "state.hpp"
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init();
  void Update(float dt);

  void ChangeState(StateType state) { wanted_state_type_ = state; }

  NetAPI::Socket::Server& getServer() { return server_; }
  entt::registry& getRegistry() { return registry_; }

 private:
  void UpdateSystems(float dt);

  void CreatePlayer(PlayerID id);
  void CreateEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void ResetEntities(glm::vec3* in_pos_arr, unsigned int in_num_pos);
  void AddBallComponents(entt::entity& entity, glm::vec3 in_pos,
                         glm::vec3 in_vel);
  void AddArenaComponents(entt::entity& entity);
  void HandlePacketBlock(NetAPI::Common::Packet& packet, int16_t block_type,
                         int client_id);

  NetAPI::Socket::Server server_;
  int last_num_players_ = 0;
  entt::registry registry_;

  State* current_state_ = nullptr;
  PlayState play_state_;
  LobbyState lobby_state_;
  StateType wanted_state_type_ = StateType::LOBBY;
  StateType current_state_type_ = StateType::LOBBY;

  int test_player_guid_ = 0;
};

#endif  // GAME_SERVER_HPP_