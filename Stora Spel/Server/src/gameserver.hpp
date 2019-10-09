#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared/shared.hpp>
#include <vector>
#include "serverstate.hpp"

#include "replay machine/replay_machine.hpp"
#include "util/global_settings.hpp"

#include "message.hpp"
#include "util/event.hpp"
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init(double in_update_rate);
  void Update(float dt);
  void HandlePacketsToSend();
  void HandleStateChange();


  void ChangeState(ServerStateType state) { wanted_state_type_ = state; }

  ServerPlayState* GetPlayState() { return &play_state_; }
  NetAPI::Socket::Server& GetServer() { return server_; }
  std::unordered_map<int, NetAPI::Common::Packet>& GetPackets() { return packets_; }
  entt::registry& GetRegistry() { return registry_; }

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, int16_t block_type,
                         int client_id);
  //void HandleNewTeam();


  NetAPI::Socket::Server server_;
  std::unordered_map<int, NetAPI::Common::Packet> packets_;
  entt::registry registry_;

  ServerState* current_state_ = nullptr;
  ServerPlayState play_state_;
  ServerLobbyState lobby_state_;
  ServerStateType wanted_state_type_ = ServerStateType::LOBBY;
  ServerStateType current_state_type_ = ServerStateType::LOBBY;

  std::vector<Message> messages;

  int test_player_guid_ = 0;

  std::vector<unsigned int> scores;

};

#endif  // GAME_SERVER_HPP_