#ifndef SERVER_STATE_HPP_
#define SERVER_STATE_HPP_
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"
#include "shared/shared.hpp"
#include "util/timer.hpp"
#include "replay machine/replay_machine.hpp"

class GameServer;

enum class ServerStateType {
  LOBBY = 0,
  PLAY,
};

class ServerState {
 public:
  virtual void Init() = 0;
  virtual void Update(float dt) = 0;
  virtual void Cleanup() = 0;
  ServerState() = default;
  ~ServerState() {}

  void SetGameServer(GameServer* game_server) { game_server_ = game_server; }

 protected:
  GameServer* game_server_ = nullptr;

 private:
};

class ServerLobbyState : public ServerState {
 public:
  void Init() override;
  void Update(float dt) override;
  void Cleanup() override;
  ServerLobbyState() = default;
  ~ServerLobbyState() {}

  void SetClientIsReady(int client_id, bool is_ready) {
    clients_ready_[client_id] = is_ready;
  }

 private:
  std::unordered_map<int, bool> clients_ready_;
  Timer start_game_timer;
  bool starting = false;
};

class ServerPlayState : public ServerState {
 public:
  void Init() override;
  void Update(float dt) override;
  void Cleanup() override;
  ServerPlayState() = default;
  ~ServerPlayState() {}

  void SetPlayerInput(int client_id, uint16_t actions, float pitch, float yaw) {
    players_inputs_[client_id] = std::make_pair(actions, glm::vec2(pitch, yaw));
  }
  // void CreatePlayer(long client_id);

  void ResetEntities();

  bool StartRecording(unsigned int in_replay_length_seconds);
 private:
  entt::entity CreateIDEntity();

  void CreateInitialEntities(int num_players);
  void CreateArenaEntity();
  void CreateBallEntity();
  void CreatePlayerEntity();
  void CreateGoals();
  void Record(std::bitset<10>& in_bitset, float& in_x_value, float& in_y_value,
              const float& in_dt);
  void Replay(std::bitset<10>& in_bitset, float& in_x_value, float& in_y_value);
  void CreatePickUpComponents();
  EntityID GetNextEntityGuid() { return entity_guid_++; }


  std::unordered_map<int, EntityID> clients_player_ids_;
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;

  std::vector<entt::entity> pick_ups_;

  EntityID entity_guid_ = 0;

  int last_spawned_team_ = 1;
  int red_players_ = 0;
  int blue_players_ = 0;

  std::vector<std::pair<PlayerID, unsigned int>> new_teams_;

  // Replay stuff ---
  ReplayMachine* replay_machine_ = nullptr;
  bool record_ = false;
  bool replay_ = false;
  //---
};

#endif  // !STATE_HPP_
