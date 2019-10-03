#ifndef SERVER_STATE_HPP_
#define SERVER_STATE_HPP_
#include <NetAPI/socket/server.hpp>
#include <entity/registry.hpp>
#include <entt.hpp>
#include "ecs/components.hpp"
#include "shared/shared.hpp"
#include "util/timer.hpp"

class GameServer;

enum class ServerStateType {
  LOBBY = 0,
  PLAY,
};

class ServerState {
 public:
  virtual void Init() = 0;
  virtual void Update() = 0;
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
  void Update() override;
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
  void Update() override;
  void Cleanup() override;
  ServerPlayState() = default;
  ~ServerPlayState() {}

  void SetPlayerInput(int client_id, uint16_t actions, float pitch, float yaw) {
    players_inputs_[client_id] = std::make_pair(actions, glm::vec2(pitch, yaw));
  }
  // void CreatePlayer(long client_id);

 private:
  entt::entity CreateIDEntity();

  void CreateInitialEntities(int num_players);
  void CreateArenaEntity();
  void CreateBallEntity();
  void CreatePlayerEntity();
  EntityID GetNextEntityGuid() { return entity_guid_++; }

  std::unordered_map<int, EntityID> clients_player_ids_;
  std::unordered_map<int, std::pair<uint16_t, glm::vec2>> players_inputs_;

  EntityID entity_guid_ = 0;
};

#endif  // !STATE_HPP_
