#ifndef GAME_SERVER_HPP_
#define GAME_SERVER_HPP_

#define NOMINMAX
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include <glm/glm.hpp>
#include <shared/shared.hpp>
#include <vector>
#include <unordered_map>
#include "serverstate.hpp"

#include "replay machine/replay_machine.hpp"
#include "util/global_settings.hpp"

#include "message.hpp"
#include "util/event.hpp"
#include "util/timer.hpp"

class GameServer {
 public:
  ~GameServer();
  void Init(double in_update_rate, std::unordered_map<std::string, std::string> &args);
  void Update(float dt);
  void HandlePacketsToSend();
  void HandleStateChange();
  void ReceiveEvent(const EventInfo& e);
  void DoOncePerSecond();
  void ChangeState(ServerStateType state) { wanted_state_type_ = state; }

  ServerPlayState* GetPlayState() { return &play_state_; }
  void ReceiveGameEvent(const GameEvent& event);

  NetAPI::Socket::Server& GetServer() { return server_; }
  std::unordered_map<int, NetAPI::Common::Packet>& GetPackets() {
    return packets_;
  }
  entt::registry& GetRegistry() { return registry_; }

  std::unordered_map<AbilityID, float> GetAbilityCooldowns() {
    return ability_cooldowns_;
  }

  std::unordered_map<long, std::string> GetClientNames() {
    return client_names_;
  }

 private:
  void UpdateSystems(float dt);

  void HandlePacketBlock(NetAPI::Common::Packet& packet, int16_t block_type,
                         int client_id);
  // void HandleNewTeam();

  std::vector<unsigned> pings_;
  NetAPI::Socket::Server server_;
  std::unordered_map<int, NetAPI::Common::Packet> packets_;
  entt::registry registry_;

  ServerState* current_state_ = nullptr;
  ServerPlayState play_state_;
  ServerLobbyState lobby_state_;
  ServerStateType wanted_state_type_ = ServerStateType::LOBBY;
  ServerStateType current_state_type_ = ServerStateType::LOBBY;

  std::vector<Message> messages;
  std::vector<GameEvent> game_events_;

  int test_player_guid_ = 0;

  std::vector<unsigned int> scores;
  std::unordered_map<long, std::string> client_names_;
  bool client_sent_name_ = false;

  bool NameAlreadyExists(std::string name);
  std::unordered_map<AbilityID, float> ability_cooldowns_;
};

#endif  // GAME_SERVER_HPP_