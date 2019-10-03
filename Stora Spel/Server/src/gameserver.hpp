#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared/shared.hpp>
#include "serverstate.hpp"
#include <vector>

#include "replay machine/replay_machine.hpp"
#include "util/global_settings.hpp"

#include "util/event.hpp"
#include "message.hpp"
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
  void HandleNewTeam();

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

  std::vector<std::pair<PlayerID, unsigned int>> new_teams_;
  std::vector<entt::entity> pick_ups_;
  std::vector<Message> messages;

  int test_player_guid_ = 0;

  int last_spawned_team_ = 1;

  int red_players_ = 0;
  int blue_players_ = 0;
  std::vector<unsigned int> scores;

  // Replay stuff ---
  ReplayMachine* replay_machine_ = nullptr;
  double update_rate_ = 0.0f;
  bool record_ = false;
  bool replay_ = false;
  //---
};

#endif  // GAME_SERVER_HPP_