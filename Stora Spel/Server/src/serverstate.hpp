#ifndef SERVER_STATE_HPP_
#define SERVER_STATE_HPP_
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"
#include "shared/shared.hpp"
#include "util/event.hpp"
#include "util/timer.hpp"
#include "replay machine/replay_machine.hpp"
#include "util/event.hpp"


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

  void SetTeamsUpdated(bool val) { teams_updated_ = val; }

  void SetClientIsReady(int client_id, bool is_ready) {
    clients_ready_[client_id] = is_ready;
    teams_updated_ = true;
  }

  void HandleNewClientTeam(int client_id) {
    if (last_team_ == TEAM_RED) {
      client_teams_[client_id] = TEAM_BLUE;
      last_team_ = TEAM_BLUE;
    } else {
      client_teams_[client_id] = TEAM_RED;
      last_team_ = TEAM_RED;
    }
    teams_updated_ = true;
  }

  void SetClientTeam(int client_id, unsigned int team) {
    client_teams_[client_id] = team;
    teams_updated_ = true;
  }

  void SetClientAbility(int client_id, AbilityID id) {
    client_abilities_[client_id] = id;
  }

  std::unordered_map<int, AbilityID> client_abilities_;
  std::unordered_map<int, unsigned int> client_teams_;

 private:
  std::unordered_map<int, bool> clients_ready_;
  Timer start_game_timer;
  bool starting_ = false;
  unsigned int last_team_ = TEAM_BLUE;

  bool teams_updated_ = false;
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
  void StartResetTimer();
  bool IsResetting() { return reset_; }
  bool StartRecording(unsigned int in_replay_length_seconds);

  void SetClientReceiveUpdates(long client_id, bool initialized) {
    clients_receive_updates_[client_id] = initialized;
  }

  std::unordered_map<int, AbilityID> client_abilities_;
  std::unordered_map<int, unsigned int> client_teams_;

  void ReceiveEvent(const EventInfo& e);
  // EntityID GetNextEntityGuid() { return entity_guid_++; }
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
  void EndGame();

  std::unordered_map<long, bool> clients_receive_updates_;
  std::unordered_map<int, EntityID> clients_player_ids_;
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;

  std::vector<entt::entity> created_pick_ups_;

  EntityID entity_guid_ = 0;

  int last_spawned_team_ = 1;
  int red_players_ = 0;
  int blue_players_ = 0;

  int match_time_ = 300;
  int count_down_time_ = 5;
  Timer match_timer_;
  Timer countdown_timer_;
  Timer reset_timer_;
  bool reset_ = false;

  std::vector<std::pair<PlayerID, unsigned int>> new_teams_;
  std::vector<Projectile> created_projectiles_;
  std::vector<int> destroy_entities_;

  // Replay stuff ---
  ReplayMachine* replay_machine_ = nullptr;
  bool record_ = false;
  bool replay_ = false;
  //---
};

#endif  // !STATE_HPP_
