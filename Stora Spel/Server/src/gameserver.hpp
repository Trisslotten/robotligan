#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared/shared.hpp>
#include "serverstate.hpp"
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init();
  void Update(float dt);

  void ChangeState(ServerStateType state) { wanted_state_type_ = state; }

  NetAPI::Socket::Server& GetServer() { return server_; }
  entt::registry& GetRegistry() { return registry_; }

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, int16_t block_type,
                         int client_id);


  NetAPI::Socket::Server server_;
  entt::registry registry_;

  ServerState* current_state_ = nullptr;
  ServerPlayState play_state_;
  ServerLobbyState lobby_state_;
  ServerStateType wanted_state_type_ = ServerStateType::LOBBY;
  ServerStateType current_state_type_ = ServerStateType::LOBBY;

  int test_player_guid_ = 0;
};

#endif  // GAME_SERVER_HPP_