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
  void ReceiveEvent(const EventInfo& e);

  void ChangeState(ServerStateType state) { wanted_state_type_ = state; }

  NetAPI::Socket::Server& GetServer() { return server_; }
  entt::registry& GetRegistry() { return registry_; }

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, int16_t block_type,
                         int client_id);
  //void HandleNewTeam();


  // Replay stuff---
  bool StartRecording(unsigned int in_replay_length_seconds);
  void Record(std::bitset<10>& in_bitset, float& in_x_value, float& in_y_value,
              const float& in_dt);
  void Replay(std::bitset<10>& in_bitset, float& in_x_value, float& in_y_value);
  //---

  NetAPI::Socket::Server server_;
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